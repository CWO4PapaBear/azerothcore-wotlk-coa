#include <cassert>
#include <vector>
#include <cstdint>
using uint32=unsigned;using int32=int;
constexpr int SPELL_SHAMAN_LIGHTNING_OVERLOAD_LB=45284,SPELL_SHAMAN_LIGHTNING_OVERLOAD_CL=45297,SPELLVALUE_BASE_POINT0=0,PROC_EX_CRITICAL_HIT=2;
struct AuraEffect{};
struct SpellInfo{unsigned Id=403,SpellFamilyFlags[3]={1,0,0};};
struct TargetInfo{int targetGUID;int damageBeforeTakenMods;};
struct Spell{std::vector<TargetInfo> hits;std::vector<TargetInfo>const*GetUniqueTargetInfo()const{return &hits;}};
struct Unit{bool area=true;int guid=1,calls=0,value=0,helper=0;Unit*ToPlayer(){return this;}bool MatchesSpellInteraction(unsigned id,unsigned base){return id==base||(base==403&&id>=997083&&id<=997097)||(base==421&&id==999999);}int GetGUID(){return guid;}void CastCustomSpell(unsigned id,int,int amount,Unit*,bool triggered,void*,AuraEffect const*){assert(triggered);++calls;value=amount;helper=id;}};
namespace AscensionFreePick{bool Applies(Unit*p){return p&&p->area;}}
struct DamageInfo{int damage=40;int GetDamage(){return damage;}};
struct ProcEventInfo{SpellInfo*info;Unit*target;DamageInfo*damage;Spell*original;unsigned hit=0;SpellInfo const*GetSpellInfo(){return info;}Unit*GetActionTarget(){return target;}DamageInfo*GetDamageInfo(){return damage;}Spell const*GetProcSpell(){return original;}unsigned GetHitMask(){return hit;}};
struct Probe{Unit*owner;bool prevented=false;void PreventDefaultAction(){prevented=true;}Unit*GetTarget(){return owner;}@HANDLE@};
int main(){Unit owner,target;target.guid=22;SpellInfo info;DamageInfo damage;Spell original;ProcEventInfo event{&info,&target,&damage,&original};Probe probe{&owner};AuraEffect effect;
for(unsigned id:{403u,421u,997083u,997097u,999999u})for(int base:{100,333,1000})for(unsigned hit:{0u,2u}){info.Id=id;event.hit=hit;original.hits={{11,9000},{22,base},{33,777}};owner.calls=0;probe.HandleProc(&effect,event);assert(probe.prevented&&owner.calls==1&&owner.value==base/2);assert(owner.helper==((id==421||id==999999)?45297:45284));}
original.hits={{22,0}};owner.calls=0;probe.HandleProc(&effect,event);assert(!owner.calls);original.hits={{44,500}};probe.HandleProc(&effect,event);assert(!owner.calls);event.original=nullptr;probe.HandleProc(&effect,event);assert(!owner.calls);event.original=&original;event.damage=nullptr;probe.HandleProc(&effect,event);assert(!owner.calls);event.damage=&damage;
owner.area=false;info.SpellFamilyFlags[0]=2;event.hit=2;probe.HandleProc(&effect,event);assert(owner.value==10&&owner.helper==45297);
}
