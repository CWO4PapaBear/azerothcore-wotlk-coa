from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[3]
s=(root/'src/server/coa/AscensionMysticEnchant.cpp').read_text()
a=s.index('void Synchronize(Player* player)\n{');b=s.index('    if (Applies(player) && !state->Applied.contains(982743))',a)
v=s.index('bool ValidateOwnedSlots(')
code=Path(__file__).with_name('lifecycle.cpp.in').read_text().replace('@@SYNC@@',s[a:b]+'}\n').replace('@@VALIDATE@@',s[v:a])
catalog=(root/'src/server/coa/AscensionMysticEnchantCatalog.cpp').read_text()
a=catalog.index('bool ResolveNative(')
catalog='#include "AscensionMysticEnchantCatalog.h"\n#include <algorithm>\nnamespace AscensionMysticEnchant{\n'+catalog[a:]
with tempfile.TemporaryDirectory() as directory:
 p=Path(directory);(p/'test.cpp').write_text(code);(p/'catalog.cpp').write_text(catalog)
 subprocess.run(['g++','-std=c++20','-I'+str(root/'src/server/coa'),str(p/'test.cpp'),str(p/'catalog.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Production catalog validation, ownership and aura synchronization passed copy caps, 3/2/1 stacks, both login readiness orders, missing/stale auras, unequip, abandonment, death and realm isolation; player/storage adapters used.')
