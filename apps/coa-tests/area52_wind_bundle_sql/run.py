from pathlib import Path
import subprocess,time,uuid,json,re,os,hashlib
ROOT=Path(__file__).resolve().parents[3]
HERE=Path(__file__).parent
TASK=Path(os.environ['AREA52_WIND_BUNDLE_TASK'])
name='area52-wind-sql-'+uuid.uuid4().hex[:10]
def sql(text):
    result=subprocess.run(['docker','exec','-i',name,'mysql','-uroot','-N','-B','acore_world'],input=text,text=True,capture_output=True)
    if result.returncode:
        error=re.search(r'ERROR (\d+) \(([^)]+)\)',result.stderr)
        raise RuntimeError('Offline SQL fixture failed: '+(error.group(0) if error else 'unclassified'))
    return result.stdout.strip()
def main():
    subprocess.run(['docker','run','-d','--name',name,'--network','none','--tmpfs','/var/lib/mysql:rw','-e','MYSQL_ALLOW_EMPTY_PASSWORD=yes','-e','MYSQL_DATABASE=acore_world','mysql:8.4'],check=True,capture_output=True)
    try:
        for _ in range(90):
            logs=subprocess.run(['docker','logs',name],capture_output=True,text=True)
            r=subprocess.run(['docker','exec',name,'mysql','-uroot','acore_world','-e','SELECT 1'],capture_output=True)
            if 'MySQL init process done. Ready for start up.' in logs.stdout+logs.stderr and r.returncode==0:break
            time.sleep(1)
        else:raise RuntimeError('Offline SQL startup timed out')
        schema=json.loads((ROOT/'apps/coa-tests/area52_wind_sql/schema.json').read_text())['sql']
        raw=json.loads((TASK/'cloud-sql-baseline.json').read_text())
        assert raw['rows']=='' and (TASK/'development-sql-before.txt').read_text()==''
        sql(schema)
        sql('CREATE DATABASE acore_playerbots;')
        for table,ddl in raw['schemas'].items():
            text=ddl.split('\t',1)[1].strip()
            sql(text.replace('CREATE TABLE `'+table+'`','CREATE TABLE acore_playerbots.`'+table+'`')+';')
        sql("INSERT INTO spell_script_names (spell_id,ScriptName) VALUES (123,'preserved'); INSERT INTO spell_group (id,spell_id) VALUES (1101,1490),(1101,51726),(1101,60431); INSERT INTO spell_group_stack_rules (group_id,stack_rule) VALUES (1101,3);")
        allrows='SELECT * FROM spell_script_names ORDER BY spell_id,ScriptName; SELECT * FROM spell_group ORDER BY id,spell_id; SELECT * FROM spell_proc ORDER BY SpellId;'
        for table in raw['schemas']:allrows+='SELECT * FROM acore_playerbots.'+table+' ORDER BY 1,2;'
        baseline=sql(allrows);result={}
        for realm in ('development','cloud'):
            query=(TASK/('world-query.txt' if realm=='development' else 'cloud-query.txt')).read_text()
            assert sql(query)==''
            sql('START TRANSACTION;'+(TASK/(realm+'-apply.sql')).read_text()+'COMMIT;')
            after=sql(query);assert after
            result[realm]={'before_rows_sha256':hashlib.sha256(b'').hexdigest(),'after_rows_sha256':hashlib.sha256((after+'\n').encode()).hexdigest()}
            (TASK/(realm+'-sql-after.txt')).write_text(after+'\n')
            sql('START TRANSACTION;'+(TASK/(realm+'-rollback.sql')).read_text()+'COMMIT;')
            assert sql(allrows)==baseline
        (TASK/'sql-qualified.json').write_text(json.dumps(result,indent=2))
        print('PASS combined per-realm migration and exact rollback with deployed schemas; Cloud adds Defias configuration only, no account/roster changes')
    finally:subprocess.run(['docker','rm','-f',name],check=True,capture_output=True)
if __name__=='__main__':main()
