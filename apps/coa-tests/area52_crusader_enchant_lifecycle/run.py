from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[3]
s=(root/'src/server/coa/AscensionMysticEnchant.cpp').read_text()
a=s.index('void Synchronize(Player* player)\n{');b=s.index('    if (Applies(player) && !state->Applied.contains(982743))',a)
v=s.index('bool ValidateOwnedSlots(')
code=Path(__file__).with_name('lifecycle.cpp.in').read_text().replace('@@SYNC@@',s[a:b]+'}\n').replace('@@VALIDATE@@',s[v:a])
import sys
sys.path.insert(0,str(root/'apps/coa-tests'))
from coa_talent_catalog import STUBS
with tempfile.TemporaryDirectory() as directory:
 p=Path(directory);(p/'test.cpp').write_text(code)
 for name,text in STUBS.items():(p/name).write_text(text)
 subprocess.run(['g++','-std=c++20','-I'+str(p),'-I'+str(root/'src/server/coa'),'-I'+str(root/'src/server/shared/DataStores'),'-I'+str(root/'src/common'),str(p/'test.cpp'),str(root/'src/server/coa/AscensionMysticEnchantCatalog.cpp'),str(root/'src/server/shared/DataStores/ClientDBC.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Crusader assigned enchants: production catalog validation, ownership and aura synchronization passed copy caps, 3/2/1 stacks, both login readiness orders, missing/stale auras, unequip, abandonment, death and realm isolation; player/storage adapters used.')
