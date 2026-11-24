from pathlib import Path
import json
import shlex
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[3]
def main():
    source = (ROOT / 'src/server/game/Spells/Spell.cpp').read_text()
    start = source.index('        for (int i = 0; i < MAX_ITEM_SPELLS; ++i)', source.index('SpellCastResult Spell::CheckItems'))
    end = source.index('        // consumable cast item checks', start)
    block = source[start:end]
    cpp = r"""
#include "Item.h"
#include <cstdlib>
#undef assert
#define assert(x) do { if (!(x)) std::abort(); } while (false)
#include <iterator>
#undef LOG_INFO
#define LOG_INFO(...) ((void)0)
static_assert(MAX_ITEM_SPELLS == 5);
static_assert(MAX_ITEM_PROTO_SPELLS == 5);
struct ItemChargeProbe {
 int charges[5]{};
 int GetSpellCharges(int i) { assert(i>=0 && i<5); return charges[i]; }
};
int Check(int itemid, ItemTemplate const* proto, ItemChargeProbe* m_CastItem) {
""" + block + r"""
return SPELL_CAST_OK;
}
int main() {
 ItemTemplate t{}; ItemChargeProbe i;
 static_assert(std::size(t.Spells)==5);
 for(int id: {3648545,8210197,1903514}) {
  assert(Check(id,&t,&i)==SPELL_CAST_OK);
  for(int slot=0;slot<5;++slot) {
   t.Spells[slot].SpellCharges=-3;
   assert(Check(id,&t,&i)==SPELL_FAILED_NO_CHARGES_REMAIN);
   i.charges[slot]=-3; assert(Check(id,&t,&i)==SPELL_CAST_OK);
   t.Spells[slot].SpellCharges=0; i.charges[slot]=0;
  }
 }
}
"""
    record=next(x for x in json.loads(Path('/botbuild/compile_commands.json').read_text())
                if x['file'].endswith('/Spells/Spell.cpp'))
    args=shlex.split(record['command']); index=args.index('-o'); del args[index:index+2]
    args.remove('-c'); args.remove(record['file'])
    with tempfile.TemporaryDirectory() as tmp:
        file=Path(tmp)/'check.cpp'; file.write_text(cpp); exe=Path(tmp)/'check'
        subprocess.run(args+[str(file),'-o',str(exe)],cwd='/botbuild',check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS: production headers/PCH, native slot constants and extracted charge predicate; all five slots')
    return 0
if __name__ == '__main__':
    raise SystemExit(main())
