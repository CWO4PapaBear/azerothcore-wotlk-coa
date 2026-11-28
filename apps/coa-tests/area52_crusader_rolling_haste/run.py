from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[3]
def block(s,m):
    a=s.index(m); b=s.index('{',a); n=1; e=b+1
    while n:
        n+=(s[e]=='{')-(s[e]=='}'); e+=1
    return s[a:e]
haste=block((ROOT/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text(),'void AuraEffect::HandleModCastingSpeed(')
rolling=block((ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text(),'void Unit::CastDelayedSpellWithPeriodicAmount(')
code=r'''
#include <cassert>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <utility>
#include <cmath>
using uint32=uint32_t;using int32=int32_t;using uint8=uint8_t;using AuraType=int;using TriggerCastFlags=unsigned;
constexpr unsigned AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK=1,AURA_EFFECT_HANDLE_STAT=2,CONFIG_MUNCHING_BLIZZLIKE=1,SPELLVALUE_BASE_POINT0=0,TRIGGERED_FULL_MASK=255,TRIGGERED_NO_PERIODIC_RESET=8;
struct Unit;
struct AuraEffect{int amount=3,old=20,ticks=4,tick=0,id=61840,caster=1,index=0;int GetAmount()const{return amount;}int GetAuraType()const{return 0;}int GetCasterGUID(){return caster;}int GetId(){return id;}int GetEffIndex(){return index;}int GetTotalTicks(){return ticks;}int GetTickNumber(){return tick;}int GetOldAmount(){return old;}void HandleModCastingSpeed(struct AuraApplication const*,uint8,bool)const;};
struct AuraMunchingQueue{int amount;AuraMunchingQueue(Unit&,int,int a,unsigned,AuraEffect*,int):amount(a){}};
struct Events{int delay=0,amount=0;int CalculateQueueTime(int d){return d;}void AddEvent(AuraMunchingQueue*p,int d){delay=d;amount=p->amount;delete p;}};
struct Unit{using AuraEffectList=std::vector<AuraEffect*>;AuraEffectList m_modAuras[1];Events m_Events;int guid=1,group=0,castAmount=-1;unsigned castFlags=0;bool instant=false;std::vector<std::pair<float,bool>> mods;int GetGUID(){return guid;}int GetHighestExclusiveSameEffectSpellGroupValue(AuraEffect const*,int){return group;}void SetInstantCast(bool b){instant=b;}void ApplyCastTimePercentMod(float v,bool a){mods.emplace_back(v,a);}void CastCustomSpell(unsigned,int,int amount,Unit*,unsigned flags,void*,AuraEffect*,int){castAmount=amount;castFlags=flags;}void CastDelayedSpellWithPeriodicAmount(Unit*,uint32,AuraType,int32,uint8);};
struct AuraApplication{Unit*u;Unit*GetTarget()const{return u;}};
struct SpellMgr{static unsigned GetRunningScriptTwin(){return 0;}unsigned GetSpellTwinFor(unsigned id,unsigned){return id;}} manager;auto*sSpellMgr=&manager;
struct World{bool munch=false;bool getBoolConfig(unsigned){return munch;}} world;auto*sWorld=&world;
@@HASTE@@
@@ROLLING@@
int main(){
 Unit u;AuraEffect effect;AuraApplication app{&u};
 for(int rank=1;rank<=3;++rank){effect.amount=rank;u.group=0;u.mods.clear();effect.HandleModCastingSpeed(&app,1,true);assert(u.mods.size()==1&&u.mods[0]==std::make_pair(float(rank),true));u.mods.clear();u.group=5;effect.HandleModCastingSpeed(&app,1,true);assert(u.mods.empty());u.group=rank;effect.HandleModCastingSpeed(&app,1,false);assert(u.mods.empty());}
 effect.amount=3;u.group=1;u.mods.clear();effect.HandleModCastingSpeed(&app,1,true);assert((u.mods==std::vector<std::pair<float,bool>>{{1,false},{3,true}}));u.mods.clear();effect.HandleModCastingSpeed(&app,1,false);assert((u.mods==std::vector<std::pair<float,bool>>{{1,true},{3,false}}));
 Unit caster,target;target.guid=2;AuraEffect dot;target.m_modAuras[0]={&dot};
 for(int tick=0;tick<=5;++tick){dot.tick=tick;target.CastDelayedSpellWithPeriodicAmount(&caster,61840,0,35,0);assert(caster.castAmount==35+20*std::max(4-tick,0)/4);assert(!(caster.castFlags&TRIGGERED_NO_PERIODIC_RESET));}
 dot.tick=1;dot.caster=3;target.CastDelayedSpellWithPeriodicAmount(&caster,61840,0,35,0);assert(caster.castAmount==35);dot.caster=1;dot.id=123;target.CastDelayedSpellWithPeriodicAmount(&caster,61840,0,35,0);assert(caster.castAmount==35);dot.id=61840;dot.ticks=0;target.CastDelayedSpellWithPeriodicAmount(&caster,61840,0,35,0);assert(caster.castAmount==35);
 dot.ticks=4;world.munch=true;caster.castAmount=-1;target.CastDelayedSpellWithPeriodicAmount(&caster,61840,0,35,0);assert(caster.castAmount==-1&&caster.m_Events.delay==400&&caster.m_Events.amount==50);
}
'''.replace('@@HASTE@@',haste).replace('@@ROLLING@@',rolling)
with tempfile.TemporaryDirectory() as d:
    p=Path(d);(p/'test.cpp').write_text(code)
    subprocess.run(['c++','-std=c++20','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
print('PASS: native strongest-haste application/removal and rolling periodic carryover, ownership, tick exhaustion, reset flags and 400ms queue admission. Group discovery, periodic dispatcher and queue execution remain separate gates.')
