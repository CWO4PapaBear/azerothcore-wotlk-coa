from pathlib import Path
import subprocess,time,uuid,json,re
ROOT=Path(__file__).resolve().parents[3];HERE=Path(__file__).parent
name='area52-crusader-sql-'+uuid.uuid4().hex[:10]
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
  sql('\n'.join(json.loads((HERE/'schema.json').read_text()).values()))
  sql("INSERT INTO spell_script_names VALUES (853512,'unrelated_preserved'),(999999,'unrelated_spell'); INSERT INTO spell_group VALUES(2000182,12345),(77,853379);")
  query='SELECT spell_id,ScriptName FROM spell_script_names ORDER BY spell_id,ScriptName; SELECT id,spell_id FROM spell_group ORDER BY id,spell_id;'
  baseline=sql(query)
  migration=(ROOT/'data/sql/updates/pending_db_world/rev_20261126_41_area52_crusader_talents.sql').read_text()
  migration += (ROOT/'data/sql/updates/pending_db_world/rev_20261126_35_crusader_blinding_ownership.sql').read_text()
  sql(migration);applied=sql(query);sql(migration);assert sql(query)==applied
  assert sql("SELECT COUNT(*) FROM spell_script_names WHERE ScriptName IN ('aura_area52_power_of_light','aura_area52_vindication','aura_area52_swift_retribution_party');")=='6'
  assert sql("SELECT COUNT(*) FROM spell_group WHERE id=2000182 AND spell_id IN (853379,853484,853648);")=='3'
  rollback="DELETE FROM spell_script_names WHERE (spell_id=853512 AND ScriptName='aura_area52_power_of_light') OR (spell_id IN(9452,26016) AND ScriptName='aura_area52_vindication') OR (spell_id IN(53379,53484,53648) AND ScriptName='aura_area52_swift_retribution_party'); DELETE FROM spell_group WHERE id=2000182 AND spell_id IN(853379,853484,853648);"
  assert sql("SELECT COUNT(*) FROM spell_script_names WHERE spell_id=954500 AND ScriptName='spell_area52_crusader_blinding_light';")=='1'
  rollback += "DELETE FROM spell_script_names WHERE spell_id=954500 AND ScriptName='spell_area52_crusader_blinding_light';"
  sql(rollback);assert sql(query)==baseline
  sql(migration.split('INSERT IGNORE')[0]);sql(rollback);assert sql(query)==baseline
  print('PASS MySQL8.4 exact schema: seven bindings/three group rows, repeated apply, exact scoped rollback after full and partial apply; unrelated controls preserved. Package rollback must capture preexisting selected rows.')

 finally:subprocess.run(['docker','rm','-f',name],check=True,capture_output=True)
if __name__=='__main__':main()
