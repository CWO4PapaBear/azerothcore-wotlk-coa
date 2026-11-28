from pathlib import Path
import hashlib,json,re,subprocess,time,uuid,os,tempfile,importlib.util
ROOT=Path(__file__).resolve().parents[3]
HERE=Path(__file__).parent
EXTERNAL=Path(os.environ['CRUSADER_FIXTURE_ROOT']) if os.environ.get('CRUSADER_FIXTURE_ROOT') else None
SAVED=EXTERNAL/'saved-order' if EXTERNAL else HERE/'fixtures'
PUBLIC_FIXTURES=json.loads((HERE/'fixtures/fixtures.json').read_text()) if not EXTERNAL else {}
def fixture_text(name):
    return (SAVED/name).read_text() if EXTERNAL else PUBLIC_FIXTURES[name]
BUILD_ID=int(re.search(r'VALUES\((\d+),',fixture_text('fixture-seed.sql')).group(1))
spec=importlib.util.spec_from_file_location('saved_fixture',ROOT/'apps/coa-tests/area52_crusader_saved_order/run.py')
saved_fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(saved_fixture)
BEFORE_WIRE,AFTER_WIRE=saved_fixture.fixture(ROOT)
def saved_text(name):
    return fixture_text(name).replace('@BEFORE_WIRE@',BEFORE_WIRE).replace('@AFTER_WIRE@',AFTER_WIRE)
def world_fixture(realm):
    if EXTERNAL:return json.loads((EXTERNAL/(realm+'-world-before.json')).read_text(encoding='utf-8-sig'))['rows'].splitlines()
    lines=[]
    for table in SCOPES:
        text=(ROOT/'data/sql/base/db_world'/(table+'.sql')).read_text()
        match=re.search(r'CREATE TABLE `'+table+r'` \(.*?;(?=\n)',text,re.S)
        assert match,table
        lines.append(table+'\t'+match.group(0).rstrip(';').replace('\n','\\n'))
    return lines
MIGRATIONS=['rev_20261126_41_area52_crusader_talents.sql','rev_20261126_35_crusader_blinding_ownership.sql','rev_20261127_45_area52_consecrated_ground.sql']
SCOPES={'spell_script_names':'spell_id IN(853512,9452,26016,53379,53484,53648,954500)','spell_group':'id=2000182','creature_template':'entry=300055','creature_template_model':'CreatureID=300055'}
name='area52-crusader-package-'+uuid.uuid4().hex[:10]
def sql(text,db='acore_world',reject=False,raw=False):
    p=subprocess.run(['docker','exec','-i',name,'mysql','-uroot','-N','-B','--raw',db],input=text,text=True,capture_output=True)
    if reject:
        assert p.returncode!=0,'Expected guarded rejection'
        return ''
    if p.returncode:
        e=re.search(r'ERROR (\d+) \(([^)]+)\)',p.stderr)
        raise RuntimeError('Offline fixture: '+(e.group(0) if e else 'unclassified'))
    return p.stdout if raw else p.stdout.strip()
def quote(value):
    if value is None:return 'NULL'
    if isinstance(value,(int,float)):return str(value)
    if value=='':return "''"
    return "CONVERT(0x"+str(value).encode().hex()+" USING utf8mb4)"
def capture(table,scope):
    columns=[line.split('\t')[0] for line in sql('SHOW COLUMNS FROM `'+table+'`;').splitlines()]
    cols=','.join('`'+c+'`' for c in columns)
    expression='JSON_ARRAY('+cols+')'
    query='SELECT '+expression+' FROM '+table+' WHERE '+scope+' ORDER BY '+cols+';'
    rows=sql(query).splitlines()
    payload='\n'.join(rows)
    digest=hashlib.sha256(payload.encode()).hexdigest()
    guard="SHA2(COALESCE((SELECT GROUP_CONCAT(j ORDER BY BINARY j SEPARATOR '\\n') FROM (SELECT "+expression+' j FROM `'+table+'` WHERE '+scope+") q),''),256)"
    guard_hash=hashlib.sha256('\n'.join(sorted(rows)).encode()).hexdigest()
    inserts=''
    for row in rows:inserts+='INSERT INTO `'+table+'`('+cols+') VALUES('+','.join(quote(x) for x in json.loads(row))+');\n'
    return {'query':query,'hash':digest,'guard':guard,'guard_hash':guard_hash,'restore':'DELETE FROM `'+table+'` WHERE '+scope+';\n'+inserts,'rows':rows}
def guard_sql(states):
    checks=' AND '.join(x['guard']+"='"+x['guard_hash']+"'" for x in states.values())
    return "SET SESSION group_concat_max_len=1048576;\nCREATE TEMPORARY TABLE crusader_world_guard(ok TINYINT NOT NULL CHECK(ok=1));\nINSERT INTO crusader_world_guard VALUES ("+checks+");\nDROP TEMPORARY TABLE crusader_world_guard;\n"
def strip_transaction(text):
    return text.replace('START TRANSACTION;','').replace('COMMIT;','')
def main():
    subprocess.run(['docker','run','-d','--name',name,'--network','none','--tmpfs','/var/lib/mysql:rw','-e','MYSQL_ALLOW_EMPTY_PASSWORD=yes','-e','MYSQL_DATABASE=acore_world','mysql:8.4'],check=True,capture_output=True)
    try:
        for _ in range(60):
            ping=subprocess.run(['docker','exec',name,'mysql','-uroot','acore_world','-e','SELECT 1'],capture_output=True)
            logs=subprocess.run(['docker','logs',name],capture_output=True,text=True)
            if ping.returncode==0 and 'MySQL init process done. Ready for start up.' in logs.stdout+logs.stderr:break
            time.sleep(1)
        else:raise RuntimeError('Offline fixture startup timeout')
        output=Path(__import__('os').environ.get('CRUSADER_SQL_OUTPUT',str(HERE/'qualified-output')));output.mkdir(parents=True,exist_ok=True)
        migration='\n'.join((ROOT/'data/sql/updates/pending_db_world'/f).read_text() for f in MIGRATIONS)
        for realm in ['development','cloud']:
            sql('DROP DATABASE IF EXISTS acore_characters; CREATE DATABASE acore_characters CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci; DROP DATABASE acore_world; CREATE DATABASE acore_world CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;','mysql')
            lines=world_fixture(realm)
            for line in lines:
                if '\tCREATE TABLE ' in line:sql(line.split('\t',1)[1].replace('\\n','\n')+';')
                elif line:
                    fields=line.split('\t')
                    assert len(fields)==2 and fields[0]=='2000182', 'New scoped baseline needs explicit decoder'
                    sql('INSERT INTO spell_group VALUES('+','.join(fields)+');')
            sql("INSERT INTO spell_script_names VALUES(999999,'outside_scope'); INSERT INTO spell_group VALUES(77,853379); INSERT INTO creature_template(entry,name) VALUES(999999,'outside_scope'); INSERT INTO creature_template_model(CreatureID,Idx,CreatureDisplayID) VALUES(999999,0,1);")
            sql(saved_text('fixture-schema.sql'),'acore_characters');sql(saved_text('fixture-seed.sql'),'acore_characters')
            character_query="SELECT build_id,revision,format_version,category,name,HEX(document) FROM area52_build_revision ORDER BY revision; SELECT * FROM area52_build_featured; SELECT * FROM area52_build_archetype;"
            verification_query='\n'.join(capture(t,scope)['query'] for t,scope in SCOPES.items())+'\n'
            verification_query+='SELECT JSON_ARRAY(`build_id`,`revision`,`format_version`,`category`,`name`,`document`) FROM acore_characters.area52_build_revision WHERE build_id=@BUILD_ID@ AND revision IN(4,5,6) ORDER BY `build_id`,`revision`;\n'
            verification_query+='SELECT JSON_ARRAY(`build_id`,`revision`,`featured_by_account`,`featured_at`) FROM acore_characters.area52_build_featured WHERE build_id=@BUILD_ID@ ORDER BY `build_id`;\n'
            verification_query+='SELECT JSON_ARRAY(`archetype_id`,`build_id`,`revision`) FROM acore_characters.area52_build_archetype WHERE archetype_id=132 ORDER BY `archetype_id`;\n'
            verification_query+='SELECT JSON_ARRAY(`id`,`owner_account`,`created_at`,`archived`) FROM acore_characters.area52_build WHERE id=@BUILD_ID@ ORDER BY `id`;\n'
            verification_query=verification_query.replace('@BUILD_ID@',str(BUILD_ID))
            before_rows=sql(verification_query,raw=True)
            characters_before=hashlib.sha256(sql(character_query,'acore_characters',raw=True).encode()).hexdigest()
            controls="SELECT * FROM spell_script_names WHERE spell_id=999999; SELECT * FROM spell_group WHERE id=77; SELECT * FROM creature_template WHERE entry=999999; SELECT * FROM creature_template_model WHERE CreatureID=999999;"
            control_before=sql(controls)
            before={t:capture(t,s) for t,s in SCOPES.items()}
            sql(migration);after={t:capture(t,s) for t,s in SCOPES.items()};sql(migration)
            assert {t:capture(t,s)['hash'] for t,s in SCOPES.items()}=={t:x['hash'] for t,x in after.items()}
            assert sql(controls)==control_before
            sql(''.join(x['restore'] for x in before.values()))
            apply='START TRANSACTION;\nUSE acore_world;\n'+guard_sql(before)+migration+'\nUSE acore_characters;\n'+strip_transaction(saved_text('apply.sql'))+'\nCOMMIT;\n'
            rollback='START TRANSACTION;\nUSE acore_world;\n'+guard_sql(after)+''.join(x['restore'] for x in before.values())+'\nUSE acore_characters;\n'+strip_transaction(saved_text('rollback.sql'))+'\nCOMMIT;\n'
            sql(apply)
            after_rows=sql(verification_query,raw=True)
            characters_after=hashlib.sha256(sql(character_query,'acore_characters',raw=True).encode()).hexdigest()
            assert sql('SELECT MAX(revision) FROM area52_build_revision;','acore_characters')=='5'
            sql(apply,reject=True)
            assert sql('SELECT MAX(revision) FROM area52_build_revision;','acore_characters')=='5'
            sql("UPDATE creature_template SET name='concurrent_edit' WHERE entry=300055;")
            sql(rollback,reject=True)
            assert sql("SELECT name FROM creature_template WHERE entry=300055;")=='concurrent_edit'
            assert sql('SELECT MAX(revision) FROM area52_build_revision;','acore_characters')=='5'
            sql(after['creature_template']['restore'])
            sql("UPDATE area52_build_revision SET document=JSON_SET(document,'$.wire','changed') WHERE revision=5;",'acore_characters')
            sql(rollback,reject=True)
            assert {t:capture(t,s)['hash'] for t,s in SCOPES.items()}=={t:x['hash'] for t,x in after.items()}
            afterwire=AFTER_WIRE
            sql("UPDATE area52_build_revision SET document=JSON_SET(document,'$.wire','"+afterwire+"') WHERE revision=5;",'acore_characters')
            sql(rollback)
            assert {t:capture(t,s)['hash'] for t,s in SCOPES.items()}=={t:x['hash'] for t,x in before.items()}
            assert sql(controls)==control_before
            assert sql('SELECT revision FROM area52_build_revision ORDER BY revision;','acore_characters').splitlines()==['4','5','6']
            assert sql('SELECT revision FROM area52_build_featured; SELECT revision FROM area52_build_archetype;','acore_characters').splitlines()==['6','6']
            assert sql("SELECT HEX(JSON_UNQUOTE(JSON_EXTRACT(document,'$.wire'))) FROM area52_build_revision WHERE revision=4;",'acore_characters')==sql("SELECT HEX(JSON_UNQUOTE(JSON_EXTRACT(document,'$.wire'))) FROM area52_build_revision WHERE revision=6;",'acore_characters')
            dest=output/realm;dest.mkdir(exist_ok=True)
            (dest/'apply.sql').write_text(apply);(dest/'rollback.sql').write_text(rollback)
            recovery_rows=sql(verification_query,raw=True)
            (dest/'query.sql').write_text(verification_query)
            for phase,rows in [('before',before_rows),('after',after_rows),('rollback',recovery_rows)]: (dest/(phase+'-rows.txt')).write_text(rows)
            (dest/'hashes.json').write_text(json.dumps({'before':{t:x['hash'] for t,x in before.items()},'after':{t:x['hash'] for t,x in after.items()},'before_rows_sha256':hashlib.sha256(before_rows.encode()).hexdigest(),'after_rows_sha256':hashlib.sha256(after_rows.encode()).hexdigest(),'rollback_rows_sha256':hashlib.sha256(recovery_rows.encode()).hexdigest(),'characters_before_sha256':characters_before,'characters_after_sha256':characters_after,'characters_recovery_sha256':hashlib.sha256(sql(character_query,'acore_characters',raw=True).encode()).hexdigest(),'apply_sha256':hashlib.sha256(apply.encode()).hexdigest(),'rollback_sha256':hashlib.sha256(rollback.encode()).hexdigest()},indent=2)+'\n')
        print('PASS combined MySQL8.4 exact-schema migrations, repeated world apply, guarded immutable revision apply, forward recovery rollback and concurrent-edit rejection; unrelated controls preserved.')
    finally:subprocess.run(['docker','rm','-f',name],check=True,capture_output=True)
if __name__=='__main__':main()
