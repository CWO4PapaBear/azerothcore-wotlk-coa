from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[3]
def method(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
s=(r/'src/server/coa/Area52DragonWarrior.inc').read_text()
charge=s[s.index('class aura_area52_dragon_charge'):s.index('class spell_area52_dragon_charge_impact')]
roar=s[s.index('class spell_area52_dragon_roar_stun'):s.index('class area52_unstoppable_force')]
impact=s[s.index('class spell_area52_dragon_charge_impact'):s.index('class spell_area52_dragon_roar_stun')]
code=r"""
#include "Area52DragonWarriorRules.h"
#include <cassert>
#include <array>
#include <map>
#include <list>
#include <vector>
#include <cmath>
using uint32=unsigned;using int32=int;using SpellEffIndex=int;using AuraEffectHandleModes=int;struct AuraEffect{};constexpr int EFFECT_1=1,EFFECT_2=2;
struct Aura{int duration=0;unsigned stacks=0;void SetDuration(int n){duration=n;}unsigned GetStackAmount()const{return stacks;}};
struct Unit{bool alive=true,hostile=true,arc=true,near=true,los=true,immune=false;std::map<unsigned,Aura>auras;std::vector<unsigned>casts;
 void CastSpell(Unit*,unsigned spell,bool){casts.push_back(spell);auras[spell]={};}Aura*GetAura(unsigned id){return auras.contains(id)?&auras[id]:nullptr;}
 void RemoveAurasDueToSpell(unsigned id){auras.erase(id);}float GetCombatReach(){return 1.5f;}bool IsAlive(){return alive;}bool IsValidAttackTarget(Unit*t){return t->hostile;}bool HasInArc(float,Unit*t){return t->arc;}bool IsWithinDistInMap(Unit*t,float){return t->near;}bool IsWithinLOSInMap(Unit*t){return t->los;}bool IsImmunedToSpellEffect(void*,int,Unit*){return immune;}};
struct Effect{int CalcValue(Unit*){return 5000;}};struct Info{Effect Effects[3];};std::list<Unit*>targets;
namespace Acore{struct AnyUnfriendlyUnitInObjectRangeCheck{AnyUnfriendlyUnitInObjectRangeCheck(Unit*,Unit*,float){}};template<class T>struct UnitListSearcher{std::list<Unit*>&out;UnitListSearcher(Unit*,std::list<Unit*>&o,T&):out(o){}};}
namespace Cell{template<class T>void VisitObjects(Unit*,T&s,float){s.out=targets;}}
struct Charge{Unit*unit;Info info;bool removed=false;Unit*GetTarget(){return unit;}Info*GetSpellInfo(){return &info;}int GetDuration(){return 3000;}void Remove(){removed=true;}
@@CHARGE@@};
struct Impact{Unit*unit,*target;int damage=100;void*GetSpellInfo(){return nullptr;}Unit*GetCaster(){return unit;}Unit*GetHitUnit(){return target;}int GetHitDamage(){return damage;}void SetHitDamage(int n){damage=n;}@@IMPACT@@};
struct Roar{Unit*unit;bool prevented=false;Unit*GetCaster(){return unit;}void PreventHitDefaultEffect(int){prevented=true;}@@ROAR@@};
int main(){Unit caster;Charge c{&caster};bool periodic=false;int amplitude=0;c.TickEvery(nullptr,periodic,amplitude);assert(periodic&&amplitude==100);c.Start(nullptr,0);assert(caster.GetAura(954286)->duration==3000&&caster.GetAura(954289)->duration==5000);
Unit dead,friendUnit,behind,far,wall,good,second;dead.alive=false;friendUnit.hostile=false;behind.arc=false;far.near=false;wall.los=false;targets={&dead,&friendUnit,&behind,&far,&wall};auto n=caster.casts.size();c.Impact(nullptr);assert(caster.casts.size()==n&&!c.removed);targets.push_back(&good);targets.push_back(&second);c.Impact(nullptr);assert(c.removed&&caster.casts.size()==n+1&&caster.casts.back()==954288);c.Stop(nullptr,0);assert(!caster.GetAura(954286)&&caster.GetAura(954289));
for(bool immune:{false,true}){good.immune=immune;Impact i{&caster,&good};i.Damage(0);assert(i.damage==(immune?300:100));}
for(unsigned stacks=0;stacks<=5;++stacks){caster.auras[954253].stacks=stacks;Roar r{&caster};r.Stun(1);assert(r.prevented==(stacks<3));}Roar empty{nullptr};empty.Stun(1);assert(empty.prevented);}
""".replace('@@CHARGE@@','\n'.join(method(charge,x) for x in ['void TickEvery(','void Start(','void Impact(','void Stop('])).replace('@@IMPACT@@',method(impact,'void Damage(')).replace('@@ROAR@@',method(roar,'void Stun('))
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'t.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-I'+str(r/'src/server/coa'),str(p/'t.cpp'),'-o',str(p/'t')],check=True);subprocess.run([str(p/'t')],check=True)
print('PASS production Charge periodic/start/impact/stop: speed lifetime, Might5s, dead/friendly/rear/range/LOS exclusions, first eligible impact only, slow-immune triple damage; Roar stun at3+ Fury stacks. Spatial predicates supplied by adapter; not gameplay pathfinding.')
