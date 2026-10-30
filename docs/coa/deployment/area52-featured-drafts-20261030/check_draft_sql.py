from pathlib import Path
import subprocess,json,re,os,time,uuid,concurrent.futures
W=Path(os.environ['AREA52_BOT_REVIEW']).parent;T=W/'draft-save-20261030';name='ascension-freepick-mystic-isolated-db';db='draft_save_'+uuid.uuid4().hex[:10]
info=json.loads(subprocess.check_output(['docker','inspect',name],text=True))[0]
assert info['HostConfig']['NetworkMode']=='none'
assert all('/private/mystic-isolated-db/' in m['Source'] for m in info['Mounts'])
name='area52-draft-sql-'+uuid.uuid4().hex[:10]
args=['docker','run','--rm','-d','--name',name,'--network','none','--memory','1g','--cpus','1','--tmpfs','/var/lib/mysql:rw,size=512m','-e','MYSQL_ROOT_PASSWORD_FILE=/run/secrets/password']
for m in info['Mounts']:args+=['-v',m['Source']+':'+m['Destination']+':ro']
args+=[info['Image']]
subprocess.run(args,check=True,stdout=subprocess.DEVNULL)
was_running=False
def sql(text,database=db,fail=False):
 r=subprocess.run(['docker','exec','-i',name,'mysql','--defaults-extra-file=/run/secrets/mysql-root.cnf','-BNr',database],input=text,text=True,capture_output=True)
 if fail:assert r.returncode!=0,'Expected rejection'
 elif r.returncode:raise RuntimeError('Isolated SQL failure: '+r.stderr[:500])
 return r.stdout.strip()
src=(T/'source/src/server/database/Database/Implementation/CharacterDatabase.cpp').read_text()
def statement(key):
 part=src.split('PrepareStatement('+key+',',1)[1].split('CONNECTION_ASYNC);',1)[0]
 return ''.join(json.loads('"'+s+'"') for s in re.findall(r'"((?:[^"\\]|\\.)*)"',part))
def bind(key,*params):
 text=statement(key)
 for value in params:
  literal=str(value) if isinstance(value,int) else "'"+value.replace("'","''")+"'"
  text=text.replace('?',literal,1)
 assert '?' not in text
 return text+';'
def save(build,arch,revision=1,previous=0,owner=7,create=True):
 return 'START TRANSACTION;'+(bind('CHAR_INS_A52_BUILD',build,owner) if create else '')+bind('CHAR_INS_A52_BUILD_REVISION',revision,8,'Naturalist','abcd',build,owner,previous)+bind('CHAR_UPS_A52_DRAFT_ASSIGNMENT',arch,build,revision)+bind('CHAR_UPS_A52_FEATURED_DRAFT',build,revision,owner)+'COMMIT;'
try:
 for _ in range(60):
  if subprocess.run(['docker','exec',name,'mysql','--defaults-extra-file=/run/secrets/mysql-root.cnf','-NBe','SELECT 1'],capture_output=True).returncode==0:break
  time.sleep(1)
 else:raise RuntimeError('Isolated fixture startup timeout')
 sql('CREATE DATABASE '+db+' CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;', 'mysql')
 sql((T/'production-schema.sql').read_text())
 migration=(T/'source/data/sql/updates/pending_db_characters/2026_10_30_00_area52_draft_assignment.sql').read_text();sql(migration);sql(migration)
 sql(save(101,1));assert sql('SELECT archetype_id,build_id,revision FROM area52_build_archetype')=='1\t101\t1'
 sql(save(101,1,2,1,create=False));assert sql('SELECT revision FROM area52_build_featured WHERE build_id=101')=='2'
 sql(save(102,1),fail=True);assert sql('SELECT COUNT(*) FROM area52_build WHERE id=102')=='0'
 sql(save(101,2,3,2,create=False),fail=True);assert sql('SELECT MAX(revision) FROM area52_build_revision WHERE build_id=101')=='2'
 sql(save(101,1,3,2,owner=8,create=False),fail=True)
 sql(save(101,1,3,0,create=False),fail=True)
 assert sql('SELECT revision FROM area52_build_archetype WHERE build_id=101')=='2'
 assert sql('SELECT COUNT(*) FROM area52_build_revision WHERE build_id=101')=='2'
 def racing(build):
  try:sql(save(build,3));return True
  except RuntimeError:return False
 with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:results=list(pool.map(racing,[201,202]))
 assert sum(results)==1
 assert sql('SELECT COUNT(*) FROM area52_build WHERE id IN (201,202)')=='1'
 assert sql('SELECT COUNT(*) FROM area52_build_archetype WHERE archetype_id=3')=='1'
 sql('START TRANSACTION;'+bind('CHAR_INS_A52_BUILD',301,7)+bind('CHAR_INS_A52_BUILD_REVISION',1,1,'Ordinary','ab',301,7,0)+'COMMIT;')
 assert sql('SELECT COUNT(*) FROM area52_build_archetype WHERE build_id=301')=='0'
 assert len(sql(bind('CHAR_SEL_A52_BUILD_LIST',7,8,8,7)).splitlines())==2
 assert len(sql(bind('CHAR_SEL_A52_BUILD_LIST',7,1,1,7)).splitlines())==1
 import importlib.util
 spec=importlib.util.spec_from_file_location('draft_export',T/'export_assignments.py');export=importlib.util.module_from_spec(spec);spec.loader.exec_module(export)
 exported=sql(export.QUERY).splitlines();assert len(exported)==2 and exported[0]=='1\t101\t2'
 row=sql(bind('CHAR_SEL_A52_BUILD',101));assert row.endswith('\t1\t1')
 assert sql(bind('CHAR_SEL_A52_DELETE_STATE',101)).endswith('\t1')
 assert sql(bind('CHAR_SEL_A52_CREATION_DRAFT',1))=='7\t2\tabcd\t1'
 assert not sql(bind('CHAR_SEL_A52_CREATION_DRAFT',99999))
 sql((T/'collection-schema.sql').read_text())
 sql('INSERT INTO account_freepick_mystic_collection VALUES(7,100);')
 def reward(guid,build=101,revision=2,account=7,spells=(100,200,300),fault=False):
  text='START TRANSACTION;'+bind('CHAR_INS_A52_DRAFT_REWARD',guid,account,build,revision)
  text+=''.join(bind('CHAR_INS_A52_DRAFT_REWARD_ENCHANT',account,spell,guid,account,build,revision) for spell in spells)
  return text+('INSERT INTO deliberately_missing_table VALUES(1);' if fault else '')+'COMMIT;'
 sql(reward(41,fault=True),fail=True)
 assert sql('SELECT COUNT(*) FROM area52_draft_reward')=='0'
 assert sql('SELECT spell FROM account_freepick_mystic_collection WHERE account=7')=='100'
 sql(reward(41));sql(reward(41))
 assert sql('SELECT COUNT(*) FROM account_freepick_mystic_collection WHERE account=7')=='3'
 assert sql(bind('CHAR_SEL_A52_DRAFT_REWARD',41,7))=='101\t2'
 sql(reward(41,build=999,spells=(400,)));sql(reward(41,account=8,spells=(500,)))
 assert sql('SELECT COUNT(*) FROM account_freepick_mystic_collection')=='3'
 with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
  list(pool.map(lambda _:sql(reward(42,spells=(600,))),range(2)))
 assert sql('SELECT COUNT(*) FROM area52_draft_reward WHERE guid=42')=='1'
 assert sql('SELECT COUNT(*) FROM account_freepick_mystic_collection WHERE spell=600')=='1'
 (T/'reward-sql-checks.json').write_text(json.dumps({'passed':True,'checks':['production collection collation','atomic claim and grants rollback on partial failure','existing ownership preserved','same-character retry idempotent','different draft and account cannot reuse claim','concurrent reward single claim','exact mapped creation lookup','unassigned archetype fails closed'],'no_live_sql':True},indent=2))
 tables=('area52_build_archetype','area52_draft_reward')
 expected={table:sql('SHOW CREATE TABLE '+table) for table in tables}
 (T/'expected-schema.json').write_text(json.dumps(expected,indent=2))
 import tempfile,shutil
 spec=importlib.util.spec_from_file_location('draft_hook',T/'hooks/draft_schema.py');hook=importlib.util.module_from_spec(spec);spec.loader.exec_module(hook)
 with tempfile.TemporaryDirectory(prefix='draft-hook-') as temporary:
  hook.ROOT=Path(temporary);hook.TX=hook.ROOT;hook.query=sql
  (hook.ROOT/'sql').mkdir();(hook.ROOT/'sql/draft.sql').write_text(migration)
  (hook.ROOT/'sql/expected-schema.json').write_text(json.dumps(expected))
  for table in reversed(tables):sql('DROP TABLE '+table)
  hook.preflight();hook.backup();hook.apply();hook.verify();hook.apply();hook.verify()
  sql('ALTER TABLE area52_draft_reward ADD unexpected INT')
  try:hook.verify();raise AssertionError('Schema drift accepted')
  except RuntimeError:pass
  try:hook.rollback();raise AssertionError('Schema drift deleted by rollback')
  except RuntimeError:pass
  sql('ALTER TABLE area52_draft_reward DROP unexpected')
  sql('INSERT INTO area52_draft_reward VALUES(999,7,101,2)')
  try:hook.rollback();raise AssertionError('Post-apply data silently deleted')
  except RuntimeError:pass
  sql('DELETE FROM area52_draft_reward WHERE guid=999');hook.rollback()
  hook.preflight()
  sql(migration.split(';',1)[0]+';');hook.rollback();hook.preflight()
 (T/'migration-hook-checks.json').write_text(json.dumps({'passed':True,'checks':['exact hook backup/apply/readback/rollback','idempotent apply','schema drift rejected','partial apply rollback','post-apply data retained for recovery'],'no_live_sql':True},indent=2))
 (T/'sql-checks.json').write_text(json.dumps({'passed':True,'checks':['production DDL/collation','repeat migration','create/edit exact prepared statements','featured/mapping revision readback','conflicting archetype atomic rollback','same build conflicting archetype rollback','wrong owner rollback','stale revision rollback','concurrent assignment one winner/no orphan','ordinary save unchanged','async statements used by async commit'],'no_live_sql':True},indent=2))
 print('Draft save SQL qualification PASSED')
finally:
 try:sql('DROP DATABASE IF EXISTS '+db,'mysql')
 finally:
  if not was_running:subprocess.run(['docker','stop',name],check=True,stdout=subprocess.DEVNULL)
