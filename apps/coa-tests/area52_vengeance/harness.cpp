#include <algorithm>
#include <cassert>
#include <cstdint>
#include <set>
#include <vector>
using uint32=uint32_t;using uint64=uint64_t;using int32=int32_t;using SpellGroup=int;
constexpr int SPELL_DAMAGE_CLASS_MELEE=2,SPELL_DAMAGE_CLASS_RANGED=3,SPELL_SCHOOL_MASK_MAGIC=126,SPELL_AURA_MOD_CRIT_DAMAGE_BONUS=163,SPELL_AURA_MOD_CRIT_PERCENT_VERSUS=1,ASCENSION_CONDITIONAL_CRIT_DAMAGE=1,SPELLMOD_CRIT_DAMAGE_BONUS=1;
template<class T,class U>auto CalculatePct(T v,U p){return v*p/100;}template<class T,class U>void AddPct(T& v,U p){v+=CalculatePct(v,p);}
struct SpellInfo{uint32 Id=0;int DmgClass=1;uint32 school=64;uint32 GetSchoolMask()const{return school;}SpellInfo const* GetFirstRankSpell()const{return this;}};
struct AuraEffect{SpellInfo info;int amount=0;int GetMiscValue()const{return 126;}uint32 GetId()const{return info.Id;}SpellInfo const* GetSpellInfo()const{return &info;}};
struct Player{bool a52=true;template<class T>void ApplySpellMod(uint32,int,T&) {}};
namespace AscensionFreePick{bool Applies(Player const* p){return p&&p->a52;}}
struct Manager{bool IsSpellMemberOfSpellGroup(uint32 id,int){return id==16909||id==33221;}}manager;auto* sSpellMgr=&manager;
struct Unit{Player* player;std::set<uint32> auras;std::vector<AuraEffect> effects;Player* ToPlayer()const{return player;}bool HasAura(uint32 id)const{return auras.contains(id);}template<class F>int GetTotalAuraModifier(int,F f)const{int sum=0;for(auto const&e:effects)if(f(&e))sum+=e.amount;return sum;}int GetTotalAuraModifierByMiscMask(int,int)const{return 0;}int GetAscensionConditionalCombatModifier(Unit const*,SpellInfo const*,int)const{return 0;}int GetCreatureTypeMask()const{return 1;}Player* GetSpellModOwner()const{return player;}static uint32 SpellCriticalDamageBonus(Unit const*,SpellInfo const*,uint32,Unit const*);};
CRITICAL
int main(){Player p;Unit u{&p};SpellInfo spell;for(uint32 r=0;r<5;++r){u.auras={16909+r};assert(Unit::SpellCriticalDamageBonus(&u,&spell,10000,nullptr)==18000+500*r);for(uint32 other=0;other<5;++other){u.auras={16909+r,33221+other};assert(Unit::SpellCriticalDamageBonus(&u,&spell,10000,nullptr)==18000+500*std::max(r,other));}}
u.auras={16913};spell.DmgClass=2;assert(Unit::SpellCriticalDamageBonus(&u,&spell,10000,nullptr)==20000);spell.DmgClass=1;p.a52=false;assert(Unit::SpellCriticalDamageBonus(&u,&spell,10000,nullptr)==15000);p.a52=true;u.auras.clear();assert(Unit::SpellCriticalDamageBonus(&u,&spell,10000,nullptr)==15000);u.auras={16909};u.effects={{{33221},40}};assert(Unit::SpellCriticalDamageBonus(&u,&spell,10000,nullptr)==21000);u.effects={{{999},10}};assert(Unit::SpellCriticalDamageBonus(&u,&spell,10000,nullptr)==19500);u.effects.clear();u.auras={16909,16913};assert(Unit::SpellCriticalDamageBonus(&u,&spell,10000,nullptr)==20000);u.auras={16909};assert(Unit::SpellCriticalDamageBonus(&u,&spell,10000,nullptr)==18000);
}
