#include <cassert>
#include <map>
#include <cstdint>
using uint8=uint8_t;using uint32=uint32_t;using WeaponAttackType=int;using BaseModGroup=int;
constexpr int BASE_ATTACK=0,OFF_ATTACK=1,RANGED_ATTACK=2,CRIT_PERCENTAGE=0,OFFHAND_CRIT_PERCENTAGE=1,RANGED_CRIT_PERCENTAGE=2,MAX_ATTACK=3;
constexpr int SPELL_AURA_MOD_HIT_CHANCE=54,SPELL_AURA_ASCENSION_MOD_HIT_CHANCE_ALL_PCT=333,SPELL_AURA_MOD_SPELL_HIT_CHANCE=55,SPELL_AURA_MOD_SPELL_CRIT_CHANCE=57,SPELL_AURA_MOD_CRIT_PCT=290,SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL=71,SPELL_AURA_MOD_WEAPON_CRIT_PERCENT=52;
constexpr int CR_HIT_MELEE=1,CR_HIT_RANGED=2,CR_HIT_SPELL=3,CR_CRIT_SPELL=4,SPELL_SCHOOL_NORMAL=0,PLAYER_SPELL_CRIT_PERCENTAGE1=10;
#define ABORT() assert(false)
struct AuraEffect{int id=913715,amount=2;int GetId()const{return id;}int GetAuraType()const{return 290;}};
struct Player{int hit=0,crit=0;AuraEffect aura;float m_modMeleeHitChance=0,m_modRangedHitChance=0,m_modSpellHitChance=0;std::map<int,float>values,weapon;
 int GetTotalAuraModifier(int,int second){return second==333?hit:crit;}template<class F>int GetTotalAuraModifier(int,int,F const&filter){return filter(&aura)?crit:0;}float GetRatingBonusValue(int){return 0;}float GetSpellCritFromIntellect(){return 5;}int GetTotalAuraModifierByMiscMask(int,int){return 0;}void SetFloatValue(int i,float v){values[i]=v;}void SetBaseModFlatValue(int i,float v){weapon[i]=v;}bool CheckAttackFitToAuraRequirement(int,AuraEffect const*){return true;}
 void UpdateMeleeHitChances();void UpdateRangedHitChances();void UpdateSpellHitChances();void UpdateSpellCritChance(uint32);void UpdateWeaponDependentCritAuras(WeaponAttackType);};
namespace AscensionFreePick{bool Applies(Player*){return true;}}
@METHODS@
int main(){Player p;for(int rank=0;rank<=3;++rank){p.hit=rank;p.UpdateMeleeHitChances();p.UpdateRangedHitChances();p.UpdateSpellHitChances();assert(p.m_modMeleeHitChance==rank&&p.m_modRangedHitChance==rank&&p.m_modSpellHitChance==rank);}for(int value:{0,2}){p.crit=value;for(int school=1;school<7;++school){p.UpdateSpellCritChance(school);assert(p.values[10+school]==5+value);}for(int attack=0;attack<3;++attack){p.UpdateWeaponDependentCritAuras(attack);assert(p.weapon[attack]==value);}}}
