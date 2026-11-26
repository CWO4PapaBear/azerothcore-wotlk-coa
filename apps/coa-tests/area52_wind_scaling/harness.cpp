#include <cassert>
#include <algorithm>
#include <cstdint>
#include <vector>
#include "Area52WindScaling.h"
using uint32=unsigned;using uint8=unsigned char;using int32=int;
constexpr int DOT=2,EFFECT_0=0,OFF_ATTACK=1,BASE_ATTACK=0,RANGED_ATTACK=2,SPELL_DAMAGE_CLASS_MELEE=2,SPELL_DAMAGE_CLASS_NONE=0,SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS=0,SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS=1,SPELLMOD_BONUS_MULTIPLIER=0;
struct SpellInfo{unsigned Id=0,DmgClass=1,SpellFamilyName=0;bool UseRangedAttackPowerForDamage=false;struct Effect{float BonusMultiplier=0.5f;}Effects[3];bool IsRangedWeaponSpell()const{return false;}};
struct SpellBonusEntry{float direct_damage=0,dot_damage=0,ap_bonus=0,ap_dot_bonus=0;};
struct AuraEffect{unsigned tick=1;int GetAmount()const{return 0;}unsigned GetTickNumber()const{return tick;}};
struct Unit;using Player=Unit;using WeaponAttackType=int;
namespace AscensionFreePick{bool Applies(Unit const*);float AgilityAttackPower(Unit*,bool){return 0;}}
struct Manager{SpellBonusEntry*bonus=nullptr;SpellBonusEntry const*GetSpellBonusData(unsigned){return bonus;}unsigned GetFirstSpellInChain(unsigned id){return id;}}manager;Manager*sSpellMgr=&manager;
template<class T>void AddPct(T&v,int n){v+=v*n/100;}
struct Unit{bool area=true,offhand=true,hasTornado=true;float ap=1000,sp=800,coeffMod=1;AuraEffect aura;Unit*ToPlayer()const{return const_cast<Unit*>(this);}unsigned GetGUID(){return 7;}void*GetWeaponForAttack(int,bool){return offhand?this:nullptr;}AuraEffect const*GetAuraEffect(unsigned,int,unsigned){return hasTornado?&aura:nullptr;}AuraEffect const*GetAuraEffect(unsigned,int){return nullptr;}bool HasAura(unsigned){return false;}int GetComboPoints(){return 0;}float GetTotalAuraModifier(int){return 0;}float GetTotalAttackPowerValue(int){return ap;}float GetSpellPowerCoefficientFlatBonus(SpellInfo const*){return 0;}float GetSpellAttackPowerCoefficientFlatBonus(SpellInfo const*){return 0;}float CalculateLevelPenalty(SpellInfo const*){return 1;}Unit*GetSpellModOwner(){return this;}void ApplySpellMod(unsigned,int,float&v){v*=coeffMod;}unsigned Calculate(SpellInfo const*spellProto){
 Unit*victim=this;unsigned effIndex=0,stack=1;int damagetype=0;bool grapeAreaDamage=false;float area52CoefficientScale=1,DoneAdvertisedBenefit=sp,ApCoeffMod=1,DoneTotalMod=1;int DoneTotal=0;unsigned pdamage=0;float*agilityDamage=nullptr;
 @NATIVE@
 return DoneTotal;
}};
namespace AscensionFreePick{bool Applies(Unit const*p){return p&&p->area;}}
struct Spell{Unit*m_caster;SpellInfo*m_spellInfo;int damage=0,effIndex=0;bool admitted=false;void Admit(){@ADMISSION@ admitted=true;}};
int main(){Unit p;SpellInfo info;std::vector<unsigned>wind{978946,978947},hurricane{983983,983984};for(unsigned id=979927;id<=979940;++id)wind.push_back(id);for(unsigned id=984073;id<=984076;++id)hurricane.push_back(id);for(unsigned id=984083;id<=984086;++id)hurricane.push_back(id);
for(unsigned id:wind){info.Id=id;assert(p.Calculate(&info)==540);p.sp=0;assert(p.Calculate(&info)==540);p.sp=800;}
for(unsigned id:hurricane){info.Id=id;assert(p.Calculate(&info)==1620);p.ap=0;assert(p.Calculate(&info)==720);p.ap=1000;p.sp=0;assert(p.Calculate(&info)==900);p.sp=800;}
for(unsigned rank=0;rank<14;++rank){info.Id=rank?997084+rank:997083;float expected=rank==0?0.125f:rank==1?0.317f:rank==2?0.561f:0.714f;assert(p.Calculate(&info)==unsigned(800*expected));p.coeffMod=1.2f;assert(p.Calculate(&info)==unsigned(800*(expected*100*1.2f/100)));p.coeffMod=1;}
info.Id=878939;unsigned sum=0;for(unsigned tick=1;tick<=8;++tick){p.aura.tick=tick;Spell spell{&p,&info,83};spell.Admit();assert(spell.admitted);sum+=spell.damage+p.Calculate(&info);}assert(sum==83+200+144);
p.hasTornado=false;Spell orphan{&p,&info,83};orphan.Admit();assert(!orphan.admitted);p.hasTornado=true;
for(unsigned id:wind){info.Id=id;for(bool equipped:{false,true}){p.offhand=equipped;Spell cast{&p,&info};cast.Admit();assert(cast.admitted==(equipped||!Area52Wind::OffhandStrike(id)));}}
p.area=false;info.Id=978946;assert(p.Calculate(&info)==400);info.Id=42;p.area=true;assert(p.Calculate(&info)==400);
}
