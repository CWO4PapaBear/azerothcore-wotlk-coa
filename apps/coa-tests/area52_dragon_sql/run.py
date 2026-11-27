from pathlib import Path
import subprocess,time,uuid,json,re,os,hashlib
ROOT=Path(__file__).resolve().parents[3];HERE=Path(__file__).parent
name='area52-dragon-sql-'+uuid.uuid4().hex[:10]
def sql(text):
 p=subprocess.run(['docker','exec','-i',name,'mysql','-uroot','-N','-B','acore_world'],input=text,text=True,capture_output=True)
 if p.returncode:
  e=re.search(r'ERROR (\d+) \(([^)]+)\)',p.stderr);raise RuntimeError('Offline fixture failed: '+(e.group(0) if e else 'unclassified'))
 return p.stdout.strip()
def main():
 subprocess.run(['docker','run','-d','--name',name,'--network','none','--tmpfs','/var/lib/mysql:rw','-e','MYSQL_ALLOW_EMPTY_PASSWORD=yes','-e','MYSQL_DATABASE=acore_world','mysql:8.4'],check=True,capture_output=True)
 try:
  for _ in range(60):
   p=subprocess.run(['docker','exec',name,'mysql','-uroot','acore_world','-e','SELECT 1'],capture_output=True)
   logs=subprocess.run(['docker','logs',name],capture_output=True,text=True)
   if p.returncode==0 and 'MySQL init process done. Ready for start up.' in logs.stdout+logs.stderr:break
   time.sleep(1)
  else:raise RuntimeError('Offline fixture startup timeout')
  sql(json.loads((HERE/'schema.json').read_text())['sql'])
  sql("INSERT INTO spell_script_names VALUES (760094,'unrelated_preserved'),(954806,'another_existing'),(999999,'unrelated_spell');")
  query='SELECT spell_id,ScriptName FROM spell_script_names ORDER BY spell_id,ScriptName;'
  baseline=sql(query)
  migration=(ROOT/'data/sql/updates/pending_db_world/rev_20261126_30_dragon_warrior_review.sql').read_text()
  sql(migration);applied=sql(query);sql(migration);assert sql(query)==applied
  assert sql("SELECT COUNT(*) FROM spell_script_names WHERE (spell_id=954806 AND ScriptName='aura_area52_burning_rush') OR (spell_id=760094 AND ScriptName='spell_area52_grappling_followup');")=='2'
  sql("DELETE FROM spell_script_names WHERE (spell_id=954806 AND ScriptName='aura_area52_burning_rush') OR (spell_id=760094 AND ScriptName='spell_area52_grappling_followup');")
  assert sql(query)==baseline
  sql('TRUNCATE TABLE spell_script_names;')
  selected='SELECT JSON_ARRAY(`spell_id`,`ScriptName`) FROM spell_script_names WHERE spell_id IN (760094,954806) ORDER BY `spell_id`,`ScriptName`;'
  assert sql(selected)==''
  sql(migration);after=sql(selected)+'\n'
  sql("DELETE FROM spell_script_names WHERE (spell_id=760094 AND ScriptName='spell_area52_grappling_followup') OR (spell_id=954806 AND ScriptName='aura_area52_burning_rush');")
  assert sql(selected)==''
  if os.environ.get('AREA52_DRAGON_SQL_RECEIPT'):
   Path(os.environ['AREA52_DRAGON_SQL_RECEIPT']).write_text(json.dumps({'before_rows_sha256':hashlib.sha256(b'').hexdigest(),'after_rows_sha256':hashlib.sha256(after.encode()).hexdigest(),'after_rows':after},indent=2))
  print('PASS MySQL8.4 production InnoDB/utf8mb4_unicode_ci schema: apply twice, exact2 additions, unrelated same-spell bindings preserved, baseline restoration. Deployment rollback must capture preexisting selected rows.')
 finally:subprocess.run(['docker','rm','-f',name],check=True,capture_output=True)
if __name__=='__main__':main()
