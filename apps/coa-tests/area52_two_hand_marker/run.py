from pathlib import Path
import subprocess,tempfile,resource
ROOT=Path(__file__).resolve().parents[3]
def extract(text,signature):
 a=text.index(signature);b=text.index('{',a)+1;depth=1
 while depth:
  depth+=(text[b]=='{')-(text[b]=='}');b+=1
 return text[a:b]
def main():
 resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 s=(ROOT/'src/server/coa/AscensionWhiteWalkerSupport.cpp').read_text();p=(ROOT/'src/server/game/Entities/Player/Player.h').read_text()
 function=extract(s,'void RefreshTwoHandRequirement(')
 code=Path(__file__).with_name('harness.cpp').read_text().replace('PREDICATE',extract(p,'bool IsTwoHandUsed() const')).replace('FUNCTION',function)
 assert s.count('        RefreshTwoHandRequirement(player);')==2
 with tempfile.TemporaryDirectory() as t:
  p=Path(t)
  for baseline in (False,True):
   (p/'test.cpp').write_text(code.replace(function,'void RefreshTwoHandRequirement(Player*) {}') if baseline else code)
   subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
   r=subprocess.run([str(p/'test')],capture_output=True);assert (r.returncode!=0)==baseline
 player=(ROOT/'src/server/game/Entities/Player/Player.cpp').read_text()
 learn=extract(player,'void Player::_learnSpell(')
 add=extract(player,'bool Player::_addSpell(')
 forward=learn[learn.index('learnFromSkill || !announce'):].split(');',1)[0]
 start=add.index('if (IsInWorld()')
 predicate=add[start:add.index('\n    {',start)].removeprefix('if (').removesuffix(')')
 test='#include <cassert>\nconstexpr int SPELL_ATTR0_PASSIVE=1,SPELL_ATTR0_DO_NOT_DISPLAY=2,SPELL_EFFECT_LEARN_SPELL=36;\nint SpellAttr0(int x){return x;}\nstruct Info{bool HasAttribute(int){return true;} bool HasAnyAura(){return false;} bool HasEffect(int){return false;}} info;\nbool IsInWorld(){return true;} bool isBeingLoaded(){return false;}\nbool notified(bool announce){bool temporary=true,learnFromSkill=false;auto spellInfo=&info;learnFromSkill=FORWARD;return PREDICATE;}\nint main(){assert(!notified(false));assert(notified(true));}\n'
 test=test.replace('FORWARD',forward).replace('PREDICATE',predicate)
 with tempfile.TemporaryDirectory() as t:
  p=Path(t);(p/'packet.cpp').write_text(test)
  subprocess.run(['c++','-std=c++20',str(p/'packet.cpp'),'-o',str(p/'packet')],check=True)
  subprocess.run([str(p/'packet')],check=True)
 print('PASS: extracted native learning-forward and learned-packet predicate reproduce silent notification suppression and normal notification delivery')
 print('PASS: native equipment predicate and marker synchronizer; 2H sword/axe/mace/polearm/staff, invalid types, empty weapon, Titan Grip, repeat sync, failed grant retry, restoration adapters, other-mode isolation; baseline missing marker fails.')
if __name__=='__main__':main()
