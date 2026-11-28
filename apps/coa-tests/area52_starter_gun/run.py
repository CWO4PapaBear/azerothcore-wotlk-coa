from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[3]
source=(r/'src/server/coa/AscensionCompat.cpp').read_text()
start=source.index('bool OnPlayerCreateInitialItems(')
hook=source[start:source.index('static void PlaceStartingActionButtons',start)]
assert 'if (!handled && AscensionFreePick::Applies(player))' in hook
assert 'Area52StarterEquipment::Items(player->getRace(true))' in hook
assert 'item == 2092 ? 2 : 1' in hook
code = r"""
#include "Area52StarterEquipment.h"
#include <algorithm>
#include <cassert>
int main() {
 for (unsigned race : {1,2,3,4,5,6,7,8,10,11}) {
  auto kit=Area52StarterEquipment::Items(race);
  assert(kit.size()==13);
  assert(std::count(kit.begin(),kit.end(),2508)==1);
  std::array<unsigned,9> previous={519243,25,2362,2504,49778,35,2092,39202,6948};
  assert(std::equal(previous.begin(),previous.end(),kit.begin()+3));
  if(race==6||race==8) assert(kit[2]==0);
  if(race==10) assert(kit[1]==20902&&kit[2]==20903);
 }
}
"""
with tempfile.TemporaryDirectory() as temp:
 p=Path(temp);(p/'kit.cpp').write_text(code)
 subprocess.run(['c++','-std=c++20','-I'+str(r/'src/server/coa'),str(p/'kit.cpp'),'-o',str(p/'kit')],check=True)
 subprocess.run([str(p/'kit')],check=True)
print('PASS all10 races: one gun, existing weapon order and race exclusions preserved; creation hook remains Area52-only. Offline kit and source routing check, not gameplay.')
