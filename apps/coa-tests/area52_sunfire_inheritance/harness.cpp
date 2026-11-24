#include <algorithm>
#include <cassert>
#include <cstdint>
#include <climits>
#include <map>
#include <set>
#include <vector>
#include <iostream>
using uint32=uint32_t; using int32=int32_t;
constexpr int PROC_FLAG_DONE_PERIODIC=0x40000, SPELL_AURA_PERIODIC_DAMAGE=3, SPELLVALUE_MAX_TARGETS=1;
constexpr int BASE_ATTACK=0;
template<class T>void AddPct(T& value,int pct){value+=value*pct/100;}
struct Player; struct Aura;
namespace AscensionFreePick { bool Applies(Player const*); }
struct flag96 {uint32 a=0,b=0,c=0;operator bool()const{return a||b||c;}flag96 operator&(flag96 const& x)const{return {a&x.a,b&x.b,c&x.c};}};
struct SpellInfo { uint32 Id; uint32 root; uint32 SpellFamilyName=7;flag96 SpellFamilyFlags{2,0,0};SpellInfo const* GetFirstRankSpell()const;bool IsAffected(uint32,flag96 const&)const;uint32 GetSchoolMask()const{return 68;} };
IS_AFFECTED
struct Manager {
 std::map<uint32,SpellInfo> records;
 uint32 GetFirstSpellInChain(uint32 id)const {auto it=records.find(id);return it==records.end()?id:it->second.root;}
 uint32 GetNextSpellInChain(uint32)const{return 0;}
} manager;
auto* sSpellMgr=&manager;
SpellInfo const* SpellInfo::GetFirstRankSpell()const {return &manager.records.at(root);}
namespace AscensionMysticEnchant {
struct Grant {uint32 Enchant,Spell,Requires;};
std::vector<Grant> SpellGrants={GRANTS};
}
struct Aura {
 int duration=12000,maximum=12000,stacks=1,charges=1; bool removed=false;
 int GetDuration()const{return duration;} int GetMaxDuration()const{return maximum;}
 void SetDuration(int x){duration=x;} void SetMaxDuration(int x){maximum=x;}
 int GetStackAmount()const{return stacks;}
 void Remove(){removed=true;}
};
struct AuraEffect {
 uint32 id,owner; Aura* aura;
 uint32 GetCasterGUID()const{return owner;} uint32 GetId()const{return id;}
 Aura* GetBase()const{return aura;}
};
struct Unit {
 std::vector<AuraEffect const*> dots;
 virtual Player* ToPlayer(){return nullptr;}
 virtual bool IsPlayer()const{return false;}
 float SpellTakenCritChance(Unit*,SpellInfo const*,uint32,float done,int,bool){return done;}
 auto const& GetAuraEffectsByType(int)const{return dots;}
};
struct Spell {
 struct Targets {Unit* unit;Unit* GetUnitTarget()const{return unit;}} m_targets;
 int time=2000; int GetCastTime()const{return time;}
};
struct Damage {uint32 amount=100; uint32 GetDamage()const{return amount;}};
struct Event {
 Unit* actor;Unit* target;SpellInfo* info;Damage* damage;Spell* cast;uint32 flags=0;
 Unit* GetActor(){return actor;} Unit* GetActionTarget(){return target;}
 SpellInfo* GetSpellInfo(){return info;} Damage* GetDamageInfo(){return damage;}
 Spell* GetProcSpell(){return cast;} uint32 GetTypeMask(){return flags;}
};
using ProcEventInfo=Event;
struct Player:Unit {
 bool area=true; uint32 guid=1; std::set<uint32> known,active,auras;
 std::map<uint32,uint32> m_temporarySpellReplacements;
 std::map<uint32,Aura> buffs;
 int explosions=0,maxTargets=0;Unit* explosionTarget=nullptr;
 Player* ToPlayer()override{return this;}
 bool IsPlayer()const override{return true;}
 float SpellDoneCritChance(Unit*,SpellInfo const*,uint32,int,bool){return 30;}
 uint32 GetGUID()const{return guid;}
 bool HasAura(uint32 id)const{return auras.count(id);}
 bool HasSpell(uint32 id)const{return known.count(id);}
 bool HasActiveSpell(uint32 id)const{return active.count(id);}
 std::vector<uint32> GetSpellInteractionOrigins(uint32)const;
 bool MatchesSpellInteraction(uint32,uint32)const;
 bool WaningMatches(uint32 id)const{
  constexpr int SPELLMOD_CASTING_TIME=10;
  struct Mod{int spellId=760198,op=10;} value;
  auto* mod=&value;auto* spellInfo=&manager.records.at(id);
  WANING_MATCH
  return false;
 }
 Aura* GetAura(uint32 id,uint32){auto it=buffs.find(id);return it==buffs.end()||it->second.removed?nullptr:&it->second;}
 void RemoveAurasDueToSpell(uint32 id,uint32){buffs.erase(id);}
 void CastSpell(Unit* target,uint32 id,bool){assert(target==this);auto [it,newAura]=buffs.try_emplace(id);if(!newAura)it->second.stacks=std::min(5,it->second.stacks+1);}
 void CastCustomSpell(uint32 id,int,int count,Unit* target,bool){assert(id==983848);++explosions;maxTargets=count;explosionTarget=target;}
};
bool AscensionFreePick::Applies(Player const* p){return p&&p->area;}
ORIGINS
MATCHES
void Lunar(Player* caster,Unit* target,SpellInfo const* m_spellInfo,uint32& damage,float& tickCritChance){LUNAR}
struct NewMoon {
 Player* player; bool prevented=false;
 Unit* GetTarget(){return player;} void PreventDefaultAction(){prevented=true;}
 PROC
};
int main(){
 for(auto g:AscensionMysticEnchant::SpellGrants){manager.records[g.Requires]={g.Requires,8921};manager.records[g.Spell]={g.Spell,977832};}
 manager.records[8921]={8921,8921};manager.records[2912]={2912,2912};manager.records[30451]={30451,30451};manager.records[983848]={983848,983848};
 for(auto g:AscensionMysticEnchant::SpellGrants){
  Player p;p.known={g.Requires,g.Spell};p.active=p.known;p.auras={977831};
  assert(p.WaningMatches(2912)&&p.WaningMatches(30451)&&!p.WaningMatches(g.Spell));
  assert(p.MatchesSpellInteraction(g.Spell,8921));assert(p.GetSpellInteractionOrigins(g.Spell)==std::vector<uint32>{g.Requires});
  p.auras.insert(977782);p.auras.insert(48518);Unit creature;Player enemy;
  uint32 tick=100;float crit=0;Lunar(&p,&creature,&manager.records[g.Spell],tick,crit);assert(tick==150&&crit==30);
  tick=100;Lunar(&p,&enemy,&manager.records[g.Spell],tick,crit);assert(tick==110);
  p.auras.erase(48518);tick=100;Lunar(&p,&creature,&manager.records[g.Spell],tick,crit);assert(tick==100);
  auto& sun=manager.records[g.Spell];auto& moon=manager.records[g.Requires];
  MASK_CASES
  assert(!sun.IsAffected(3,{2,0,0})&&!sun.IsAffected(7,{4,0,0}));
  p.area=false;assert(p.GetSpellInteractionOrigins(g.Spell).empty());p.area=true;
  p.auras.clear();assert(!p.MatchesSpellInteraction(g.Spell,8921));p.auras={977831};
  p.known.erase(g.Requires);assert(!p.MatchesSpellInteraction(g.Spell,8921));p.known.insert(g.Requires);
  p.active.erase(g.Spell);assert(!p.MatchesSpellInteraction(g.Spell,8921));p.active.insert(g.Spell);
  p.known.insert(999);p.active.insert(999);p.m_temporarySpellReplacements[g.Spell]=999;
  assert(p.MatchesSpellInteraction(999,8921));p.m_temporarySpellReplacements[999]=g.Spell;
  assert(p.GetSpellInteractionOrigins(999).size()==2);
  Unit target,splash;Aura dot,foreign;AuraEffect own{g.Spell,1,&dot},other{g.Requires,2,&foreign};target.dots={&own,&other};
  Spell cast{{&target},2000};Damage damage;Event hit{&p,&target,&manager.records[2912],&damage,&cast};NewMoon n{&p};
  n.Proc(hit);assert(n.prevented&&dot.duration==15000&&foreign.duration==12000&&p.buffs[983846].stacks==1);
  for(int i=0;i<4;++i)n.Proc(hit);assert(p.buffs[983846].stacks==5&&dot.duration==27000);
  hit.info=&manager.records[g.Spell];cast.time=0;hit.target=&splash;n.Proc(hit);assert(p.explosions==0);
  hit.target=&target;n.Proc(hit);assert(p.explosions==1&&p.maxTargets==6&&p.explosionTarget==&target&&!p.GetAura(983846,1));
  n.Proc(hit);assert(p.explosions==1);
  hit.flags=PROC_FLAG_DONE_PERIODIC;for(int i=0;i<8;++i)n.Proc(hit);assert(p.buffs[760198].stacks==5);
  p.buffs.clear();damage.amount=0;n.Proc(hit);assert(p.buffs.empty());damage.amount=100;
  hit.actor=&splash;n.Proc(hit);assert(p.buffs.empty());hit.actor=&p;
  hit.info=&manager.records[983848];n.Proc(hit);assert(p.buffs.empty());
  hit.info=&manager.records[2912];hit.flags=0;cast.time=2000;
  dot.duration=INT32_MAX-2000;n.Proc(hit);assert(dot.duration==INT32_MAX-2000&&p.buffs.empty());
  dot.duration=-1;n.Proc(hit);assert(dot.duration==-1&&p.buffs.empty());
  dot.duration=12000;Aura moonDot;AuraEffect both{g.Requires,1,&moonDot};target.dots.push_back(&both);
  n.Proc(hit);assert(dot.duration==15000&&moonDot.duration==15000&&p.buffs[983846].stacks==1);
  p.area=false;n.Proc(hit);assert(dot.duration==15000);
 }
 std::cout<<"PASS all 14 Sunfire ranks: owned recursive origins, unequip/unlearn/mode gates, New Moon direct/periodic paths, stack caps, six-target explosion primary selection, foreign dots, coexisting dots, zero damage and duration bounds\n";
}
