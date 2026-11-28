#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <map>
#include <vector>
using uint32=uint32_t;using int32=int32_t;using uint8=uint8_t;using AuraType=int;using SpellGroup=int;using Stats=int;using UnitMods=int;
constexpr int SPELL_AURA_MOD_DAMAGE_PERCENT_DONE=79,SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE=137;
constexpr int AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK=1,AURA_EFFECT_HANDLE_STAT=2,STAT_STRENGTH=0,STAT_STAMINA=2,MAX_STATS=5,UNIT_MOD_STAT_START=0,TOTAL_PCT=0,SPELL_ATTR0_IS_ABILITY=1;
#define LOG_ERROR(...)
struct Unit;struct AuraApplication{Unit*target;Unit*GetTarget()const{return target;}};
struct SpellInfo{bool HasAttribute(int)const{return false;}};
struct AuraEffect{uint32 id=920264;int amount=1,misc=0,type=137;SpellInfo info;SpellInfo const*m_spellInfo=&info;
uint32 GetId()const{return id;}int GetAmount()const{return amount;}int GetMiscValue()const{return misc;}int GetEffIndex()const{return 0;}SpellInfo const*GetSpellInfo()const{return &info;}
void HandleModTotalPercentStat(AuraApplication const*,uint8,bool)const;};
using AuraEffectList=std::vector<AuraEffect const*>;
struct Unit{std::map<int,AuraEffectList>auras;float pct[5]={1,1,1,1,1};int updates=0;bool player=true;
AuraEffectList const&GetAuraEffectsByType(int type)const{static AuraEffectList empty;auto i=auras.find(type);return i==auras.end()?empty:i->second;}
bool HasAura(uint32)const{return false;}Unit const*ToPlayer()const{return this;}Unit const*GetCharmerOrOwner()const{return nullptr;}
float GetHealthPct()const{return 100;}bool IsAlive()const{return true;}bool IsPlayer()const{return player;}bool IsPet()const{return false;}
float GetPctModifierValue(int stat,int)const{return pct[stat];}void SetStatPctModifier(int stat,int,float value){pct[stat]=value;}void UpdateStatBuffMod(int){++updates;}
uint32 GetMaxHealth()const{return 100;}void SetHealth(uint32){}
float GetTotalAuraMultiplier(AuraType,std::function<bool(AuraEffect const*)>const&)const;};
namespace AscensionFreePick{bool Applies(Unit const*){return true;}}
struct Manager{bool AddSameEffectStackRuleSpellGroups(SpellInfo const*,int,uint32,int,std::map<int,int>&){return false;}}manager,*sSpellMgr=&manager;
void AddPct(float&v,int pct){v*=1+float(pct)/100;}
bool close(float a,float b){return std::abs(a-b)<.0002;}
@MULTIPLIER@
@HANDLER@
int main(){Unit unit;AuraApplication application{&unit};AuraEffect strength;AuraEffect intellect;intellect.id=1459;intellect.misc=3;intellect.amount=5;unit.auras[137]={&strength,&intellect};
for(int copies:{1,2,3,2,1,0,3,0}){strength.amount=copies;strength.HandleModTotalPercentStat(&application,1,true);assert(close(unit.pct[0],1+copies*.01));assert(close(200*unit.pct[0],200+copies*2));assert(close(400*unit.pct[0],400+copies*4));assert(unit.pct[3]==1);}
unit.auras[137]={&intellect};strength.HandleModTotalPercentStat(&application,1,false);assert(unit.pct[0]==1);int before=unit.updates;strength.HandleModTotalPercentStat(&application,1,false);assert(unit.updates==before);
AuraEffect crusade;crusade.id=931866;crusade.type=79;crusade.misc=2;crusade.amount=2;unit.auras[79]={&crusade};
for(uint32 school:{1u,2u,3u,4u,8u,16u,32u,64u}){float result=unit.GetTotalAuraMultiplier(79,[school](AuraEffect const*e){return e->GetMiscValue()&school;});assert(close(result,school&2?1.02:1.0));}
unit.auras[79].clear();assert(unit.GetTotalAuraMultiplier(79,[](AuraEffect const*){return true;})==1);
}
