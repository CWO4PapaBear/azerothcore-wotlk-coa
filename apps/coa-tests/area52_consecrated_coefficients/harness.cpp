#include <cassert>
#include <cmath>
#include <cstdint>
#include <initializer_list>
using uint32=uint32_t;using uint8=uint8_t;using int32=int32_t;using WeaponAttackType=int;
constexpr int DOT=1,EFFECT_0=0,RANGED_ATTACK=2,BASE_ATTACK=0,SPELL_DAMAGE_CLASS_MELEE=2,SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS=1,SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS=2;
struct AuraEffect{int GetAmount()const{return 0;}};struct SpellBonusEntry{float direct_damage=0,dot_damage=0,ap_bonus=0,ap_dot_bonus=0;};struct SpellInfo{uint32 Id=977847;int DmgClass=1;bool UseRangedAttackPowerForDamage=false;bool IsRangedWeaponSpell()const{return false;}};
struct Unit;namespace AscensionFreePick{bool Applies(Unit const*);float AgilityAttackPower(Unit const*,bool){return 0;}}
struct Manager{uint32 GetFirstSpellInChain(uint32 id){return id;}}manager,*sSpellMgr=&manager;
void AddPct(float&value,int pct){value*=1+pct/100.f;}
struct Unit{bool area=true,player=true;float ap=2000,sp=1000;Unit*ToPlayer(){return player?this:nullptr;}int GetTotalAuraModifier(int){return 0;}float GetTotalAttackPowerValue(int){return ap;}bool HasAura(uint32){return false;}AuraEffect const*GetAuraEffect(uint32,int){return nullptr;}
int Damage(SpellInfo const*spellProto,int damagetype,uint8 effIndex,SpellBonusEntry const*original=nullptr){SpellBonusEntry const*bonus=original;float coeff=0;int DoneTotal=0;float ApCoeffMod=1;uint32 stack=1;float*agilityDamage=nullptr;Unit*victim=this;
@OVERRIDE@
if(bonus){@DOT@}
float DoneAdvertisedBenefit=sp,factorMod=1;@SP@
return 3+DoneTotal;}};
namespace AscensionFreePick{bool Applies(Unit const*p){return p&&p->area;}}
int main(){Unit caster;SpellInfo info;for(float ap:{0.f,1000.f,2000.f})for(float sp:{0.f,1000.f,2000.f}){caster.ap=ap;caster.sp=sp;assert(caster.Damage(&info,DOT,0)==3+int(.04f*ap)+int(.04f*sp));}caster.ap=2000;caster.sp=1000;assert(caster.Damage(&info,DOT,0)==123);caster.area=false;assert(caster.Damage(&info,DOT,0)==3);caster.area=true;caster.player=false;assert(caster.Damage(&info,DOT,0)==3);caster.player=true;assert(caster.Damage(&info,0,0)==3);assert(caster.Damage(&info,DOT,1)==3);info.Id=977846;assert(caster.Damage(&info,DOT,0)==3);SpellBonusEntry existing;existing.dot_damage=.1f;existing.ap_dot_bonus=.2f;assert(caster.Damage(&info,DOT,0,&existing)==503);info.Id=977847;assert(caster.Damage(&info,DOT,0,&existing)==123);caster.area=false;assert(caster.Damage(&info,DOT,0,&existing)==503);}
