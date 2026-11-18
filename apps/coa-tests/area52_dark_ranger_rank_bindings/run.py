from pathlib import Path
import json, os, subprocess
root=Path(__file__).resolve().parents[3]
box='area52-offline-world-reconciliation-20261116'
meta=json.loads(subprocess.check_output(['docker','inspect',box]))[0]
assert meta['HostConfig']['NetworkMode']=='none'
schema='coa_test_dark_ranger_ranks_'+str(os.getpid())
def sql(q):
 p=subprocess.run(['docker','exec','-i',box,'mysql','-uroot','-BN'],input=q.encode(),capture_output=True)
 assert p.returncode==0,p.stderr.decode()
 return p.stdout.decode().splitlines()
migration=(root/'data/sql/updates/pending_db_world/zzzzzzzz_area52_shadow_arrow_nightfall_ranks.sql').read_text()
try:
 sql('CREATE DATABASE '+schema+'; CREATE TABLE '+schema+'.spell_script_names LIKE acore_world.spell_script_names; INSERT INTO '+schema+'.spell_script_names SELECT * FROM acore_world.spell_script_names WHERE spell_id BETWEEN 997517 AND 997535;')
 before=sql('SELECT * FROM '+schema+'.spell_script_names ORDER BY spell_id,ScriptName;')
 assert sql('SELECT COUNT(*) FROM '+schema+".spell_script_names WHERE ScriptName='spell_area52_dark_ranger_shadow_arrow';")==['1']
 for i in range(2):
  sql('USE '+schema+';'+migration)
  assert sql('SELECT spell_id,COUNT(*) FROM '+schema+".spell_script_names WHERE ScriptName='spell_area52_dark_ranger_shadow_arrow' GROUP BY spell_id ORDER BY spell_id;")==[str(n)+'\t1' for n in range(997526,997536)]
 sql('DELETE FROM '+schema+".spell_script_names WHERE spell_id BETWEEN 997527 AND 997535 AND ScriptName='spell_area52_dark_ranger_shadow_arrow';")
 assert sql('SELECT * FROM '+schema+'.spell_script_names ORDER BY spell_id,ScriptName;')==before
 auto=(root/'data/sql/updates/pending_db_world/zzzzzzzz_area52_dark_ranger_auto_shot_ranks.sql').read_text()
 assert sql('SELECT COUNT(*) FROM '+schema+".spell_script_names WHERE ScriptName='spell_area52_dark_ranger_auto_shot';")==['2']
 for i in range(2):
  sql('USE '+schema+';'+auto)
  assert sql('SELECT spell_id,COUNT(*) FROM '+schema+".spell_script_names WHERE ScriptName='spell_area52_dark_ranger_auto_shot' GROUP BY spell_id ORDER BY spell_id;")==[str(n)+'\t1' for n in range(997517,997536)]
 sql('DELETE FROM '+schema+".spell_script_names WHERE spell_id BETWEEN 997518 AND 997535 AND spell_id<>997526 AND ScriptName='spell_area52_dark_ranger_auto_shot';")
 assert sql('SELECT * FROM '+schema+'.spell_script_names ORDER BY spell_id,ScriptName;')==before
 print('Ten Nightfall and nineteen Auto Shot rank bindings, repeated apply, and exact rollback passed')
finally:
 sql('DROP DATABASE '+schema+';')

import tempfile

def method(text, signature):
 start=text.index(signature);brace=text.index('{',start);depth=1;end=brace+1
 while depth:
  depth+=(text[end]=='{')-(text[end]=='}');end+=1
 return text[start:end]

source=(root/'src/server/coa/AscensionMysticEnchant.cpp').read_text()
source=source[source.index('class spell_area52_dark_ranger_auto_shot'):]
arrow=method(source,'void StartAutoShot()')
update=method((root/'src/server/game/Entities/Unit/Unit.cpp').read_text(),'void Unit::_UpdateAutoRepeatSpell()')
fixture=(Path(__file__).parent/'cadence.cpp.in').read_text().replace('ARROW_IMPL',arrow).replace('UPDATE_IMPL',update)
with tempfile.TemporaryDirectory(prefix='area52-arrow-cadence-') as temp:
 p=Path(temp);(p/'test.cpp').write_text(fixture)
 subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('All nineteen arrow ranks: native handler idempotency, cast pause/resume, preserved timer, one shot per expiry, explicit stop passed')
