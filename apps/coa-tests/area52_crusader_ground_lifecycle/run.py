from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def block(s,m):
 a=s.index(m);b=s.index('{',a);e=b+1;n=1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
s=(ROOT/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text();update=block(s,'void Aura::UpdateTargetMap(');remove=block(s,'void Aura::_Remove(')
code=r"""
#include <cassert>
#include <map>
#include <vector>
#include <set>
#include <list>
#include <cstdlib>
#define ASSERT(x) assert(x)
#define ABORT() abort()
#define LOG_FATAL(...) do{}while(0)
using uint8=unsigned char;using uint32=unsigned;using AuraRemoveMode=int;
constexpr int UPDATE_TARGET_MAP_INTERVAL=500,MAX_SPELL_EFFECTS=3,DYNOBJ_AURA_TYPE=1,SPELL_ATTR0_CU_ONLY_ONE_AREA_AURA=1,AURA_REMOVE_BY_DEFAULT=0;
struct Aura;struct AuraApplication;struct SpellInfo{int Id=965903;bool HasAttribute(int)const{return false;}};
struct Unit{using AuraApplicationMap=std::map<unsigned,AuraApplication*>;unsigned guid;AuraApplicationMap applied;unsigned GetGUID(){return guid;}bool IsImmunedToSpellEffect(SpellInfo const*,int,Unit*){return false;}bool IsImmunedToSpell(SpellInfo const*,Unit*){return false;}bool IsHighestExclusiveAura(Aura*,bool){return true;}bool IsInFlight(){return false;}bool HasAura(int){return false;}AuraApplicationMap&GetAppliedAuras(){return applied;}bool IsSelfOrInSameMap(Unit*){return true;}bool IsInWorld(){return true;}bool IsInMap(Unit*){return true;}void _UnapplyAura(AuraApplication*,int);void _CreateAuraApplication(Aura*,uint8);void _ApplyAura(AuraApplication*,uint8);};
struct AuraApplication{Aura*base;Unit*target;uint8 mask;bool active=false;Unit*GetTarget(){return target;}uint8 GetEffectMask(){return mask;}Aura const*GetBase(){return base;}};
struct Aura{using ApplicationMap=std::map<unsigned,AuraApplication*>;using UnitList=std::list<Unit*>;ApplicationMap m_applications;bool m_isRemoved=false;int m_updateTargetMapInterval=0;Unit*owner;unsigned id;std::set<Unit*>near;SpellInfo info;bool IsRemoved(){return m_isRemoved;}void FillTargetMap(std::map<Unit*,uint8>&map,Unit*){for(auto*u:near)map[u]=1;}bool IsArea(){return true;}SpellInfo const*GetSpellInfo(){return &info;}Unit*GetCaster(){return owner;}bool CanBeAppliedOn(Unit*){return true;}AuraApplication*GetApplicationOfTarget(unsigned guid){auto i=m_applications.find(guid);return i==m_applications.end()?nullptr:i->second;}int GetType(){return 0;}int GetId(){return 965903;}Unit*GetOwner(){return owner;}bool CanStackWith(Aura const*other){return owner!=other->owner;}void UpdateTargetMap(Unit*,bool);void _Remove(AuraRemoveMode);};
void Unit::_CreateAuraApplication(Aura*a,uint8 mask){auto*app=new AuraApplication{a,this,mask};a->m_applications[guid]=app;applied[a->id]=app;}
void Unit::_UnapplyAura(AuraApplication*a,int){a->base->m_applications.erase(guid);applied.erase(a->base->id);delete a;}
void Unit::_ApplyAura(AuraApplication*a,uint8){a->active=true;}
@@UPDATE@@
@@REMOVE@@
int main(){Unit first{1},second{2},ally{3};Aura a;a.owner=&first;a.id=11;a.near={&first,&ally};a.UpdateTargetMap(&first,true);assert(first.applied.size()==1&&ally.applied.size()==1);assert(ally.applied[11]->active);a.UpdateTargetMap(&first,true);assert(ally.applied.size()==1);Aura b;b.owner=&second;b.id=12;b.near={&second,&ally};b.UpdateTargetMap(&second,true);assert(ally.applied.size()==2);a.near.erase(&ally);a.UpdateTargetMap(&first,true);assert(ally.applied.size()==1&&ally.applied.contains(12));a.near.insert(&ally);a.UpdateTargetMap(&first,true);assert(ally.applied.size()==2);a._Remove(0);assert(first.applied.empty()&&ally.applied.size()==1);a.UpdateTargetMap(&first,true);assert(first.applied.empty());b._Remove(0);assert(second.applied.empty()&&ally.applied.empty());}
""".replace('@@UPDATE@@',update).replace('@@REMOVE@@',remove)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS: full native Aura::UpdateTargetMap and Aura::_Remove, entry/exit/reentry/repeated update, two source owners, selective source despawn and final cleanup. Grid selection/stack compatibility are adapters, qualified separately.')
