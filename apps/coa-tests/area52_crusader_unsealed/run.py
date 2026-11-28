from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[3]
s=(r/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text();a=s.index('    SpellProcEntry area52UnsealedProc;');b=s.index('    if (!sSpellMgr->CanSpellTriggerProcOnEvent',a);block=s[a:b]
code=r"""
#include <array>
#include <cassert>
using uint32=unsigned;
constexpr unsigned SPELL_SPECIFIC_SEAL=1,SPELL_DIRECT_DAMAGE=1,PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG=16;
struct flag96:std::array<unsigned,3>{flag96(unsigned a=1,unsigned b=1,unsigned c=1):std::array<unsigned,3>{{a,b,c}}{}};
struct SpellProcEntry{unsigned SpellFamilyName=10;flag96 SpellFamilyMask{};unsigned ProcFlags=4,Chance=25,Cooldown=1000,Charges=0,DisableEffectsMask=0;};
struct SpellInfo{unsigned Id=585,specific=1;unsigned GetSpellSpecific(){return specific;}};
struct Player{bool realm=true,cap=true,window=true;Player*ToPlayer(){return this;}bool HasAura(unsigned id,unsigned=0){return id==25958?cap:window;}unsigned GetGUID(){return 1;}bool MatchesSpellInteraction(unsigned id,unsigned root){return id==root||id==root+100000;}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->realm;}}
struct DamageInfo{unsigned amount=100,type=1;unsigned GetDamage()const{return amount;}unsigned GetDamageType()const{return type;}};
struct Spell{bool triggered=false;bool IsTriggered(){return triggered;}};
struct Event{Player*actor;Player*target;Spell*spell;DamageInfo*damage;Player*GetActor(){return actor;}Player*GetActionTarget(){return target;}Spell*GetProcSpell(){return spell;}DamageInfo*GetDamageInfo(){return damage;}};
struct Application{Player*owner;Player*GetTarget(){return owner;}};
struct Aura{SpellInfo info;Player*owner;SpellInfo*GetSpellInfo(){return &info;}Player*GetCaster(){return owner;}
SpellProcEntry probe(Application*aurApp,Event&eventInfo,SpellInfo*eventSpell,SpellProcEntry const*procEntry){@@BLOCK@@return *procEntry;}};
int main(){
 for(unsigned spellId:{585u,14914u,100585u,114914u})for(unsigned negative=0;negative<=12;++negative){
  Player p,other;DamageInfo d;Spell spell;Event e{&p,&other,&spell,&d};Application app{&p};Aura a{{0,1},&p};SpellInfo info{spellId,0};SpellProcEntry original;
  if(negative==1)p.realm=false;if(negative==2)p.cap=false;if(negative==3)p.window=false;if(negative==4)e.actor=&other;
  if(negative==5)d.amount=0;if(negative==6)d.type=2;if(negative==7)e.target=&p;if(negative==8)spell.triggered=true;
  if(negative==9)e.spell=nullptr;if(negative==10)info.Id=999;if(negative==11)a.info.specific=0;if(negative==12)app.owner=&other;
  auto result=a.probe(&app,e,&info,&original);assert(result.SpellFamilyName==(negative?10u:0u));assert(result.ProcFlags==(negative?4u:20u));
  assert(result.Chance==25&&result.Cooldown==1000&&result.Charges==0&&result.DisableEffectsMask==0);
  assert(original.SpellFamilyName==10&&original.ProcFlags==4);
 }
}
"""
code=code.replace('@@BLOCK@@',block)
with tempfile.TemporaryDirectory() as directory:
 p=Path(directory);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
assert s.index('CallScriptCheckProcHandlers',b)<s.index('float procChance = CalcProcChance',b)
print('PASS native Unsealed admission48positive/negativecases with rank/replacementmatching adapter, unchanged seal chance/cooldown/charge data; native script and chance checks remain downstream')
