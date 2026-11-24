#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <map>
#include <vector>
using uint32=uint32_t;using int32=int32_t;
constexpr int SPELL_SCHOOL_MASK_NATURE=8,SPELL_SCHOOL_MASK_ARCANE=64,SPELL_SCHOOL_MASK_MAGIC=126,SPELL_DAMAGE_CLASS_MAGIC=1,SPELL_AURA_PERIODIC_DAMAGE=3,EFFECT_0=0,SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE=179,SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE=197;
template<class T,class V>void AddPct(T& a,V b){a+=a*b/100;}
struct Player;struct AuraEffect;
struct SpellInfo{uint32 school=8;bool positive=false;int DmgClass=1;uint32 GetSchoolMask()const{return school;}bool IsPositive()const{return positive;}};
struct Unit{Player* player=nullptr;std::vector<AuraEffect*> dots;int positiveCrit=0,allCrit=0;Player* ToPlayer(){return player;}auto const& GetAuraEffectsByType(int)const{return dots;}int GetMaxPositiveAuraModifierByMiscMask(int,uint32)const{return positiveCrit;}int GetTotalAuraModifier(int)const{return allCrit;}float Damage(Unit* caster,SpellInfo const* spellProto);float Crit(SpellInfo const* spellProto);};
struct AuraEffect{uint32 id=0,ownerId=0;int amount=0;Unit* owner=nullptr;uint32 GetCasterGUID()const{return ownerId;}uint32 GetId()const{return id;}int GetAmount()const{return amount;}Unit* GetCaster()const{return owner;}};
struct Player:Unit{bool a52=true,sunfire=true;uint32 guid=7;std::map<uint32,AuraEffect> auras;Player(){player=this;}bool HasAura(uint32 id)const{return auras.contains(id);}uint32 GetGUID()const{return guid;}AuraEffect const* GetAuraEffect(uint32 id,int){return auras.contains(id)?&auras.at(id):nullptr;}bool MatchesSpellInteraction(uint32 id,uint32 root){return id==root||(sunfire&&id==977831&&root==8921);}};
namespace AscensionFreePick{bool Applies(Player const* p){return p&&p->a52;}}
float Unit::Damage(Unit* caster,SpellInfo const* spellProto){float TakenTotalMod=1;@DAMAGE@ return TakenTotalMod;}
float Unit::Crit(SpellInfo const* spellProto){float crit_chance=0;uint32 schoolMask=spellProto->school;@CRIT@ return crit_chance;}
int main(){Player p;Unit target;SpellInfo spell;AuraEffect swarm{5570,7,0,&p},moon{8921,7,0,&p},sun{977831,7,0,&p};
for(int rank=0;rank<3;++rank){p.auras.clear();p.auras[57849+rank]={0,0,2+2*rank};target.dots={&swarm};spell.school=8;assert(std::abs(target.Damage(&p,&spell)-(1.02f+.02f*rank))<.0001f);swarm.ownerId=8;assert(target.Damage(&p,&spell)==1);swarm.ownerId=7;spell.school=64;assert(target.Damage(&p,&spell)==1);target.dots={&sun};assert(std::abs(target.Damage(&p,&spell)-(1.02f+.02f*rank))<.0001f);p.sunfire=false;assert(target.Damage(&p,&spell)==1);p.sunfire=true;spell.school=4;assert(target.Damage(&p,&spell)==1);}
for(int copies=0;copies<=3;++copies){for(int rank=0;rank<=3;++rank){p.auras.clear();if(copies)p.auras[957849]={0,0,2*copies};if(rank)p.auras[57848+rank]={0,0,2*rank};target.dots={&swarm};spell.school=8;assert(std::abs(target.Damage(&p,&spell)-(1.f+.02f*copies)*(1.f+.02f*rank))<.0001f);target.dots={&sun};spell.school=64;assert(std::abs(target.Damage(&p,&spell)-(1.f+.02f*copies)*(1.f+.02f*rank))<.0001f);target.dots.clear();assert(target.Damage(&p,&spell)==1);}}
p.auras={{57851,{0,0,6}}};target.dots={&swarm};spell.school=8;assert(target.Crit(&spell)==5);for(int existing:{0,3,5,8}){target.positiveCrit=existing;assert(target.Crit(&spell)==std::max(0,5-existing));}target.positiveCrit=0;target.allCrit=3;assert(target.Crit(&spell)==2);target.allCrit=0;spell.positive=true;assert(target.Crit(&spell)==0);spell.positive=false;spell.DmgClass=2;assert(target.Crit(&spell)==0);spell.DmgClass=1;p.a52=false;assert(target.Crit(&spell)==0);assert(target.Damage(&p,&spell)==1);p.a52=true;p.auras.clear();assert(target.Crit(&spell)==0);target.dots.clear();p.auras[57851]={0,0,6};assert(target.Crit(&spell)==0);
}
