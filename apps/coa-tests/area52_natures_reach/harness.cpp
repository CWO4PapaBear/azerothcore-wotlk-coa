#include <algorithm>
#include <cassert>
#include <cstdint>
#include <list>
#include <map>
#include <vector>
using uint32 = uint32_t;
constexpr int SPELLMOD_RANGE=3, SPELL_AURA_PERIODIC_TRIGGER_SPELL=23, SPELL_EFFECT_TRIGGER_SPELL=64, SPELL_EFFECT_TRIGGER_MISSILE=32;
struct Range { float RangeMax[2]={30,30}; };
struct Effect { int ApplyAuraName=0, Effect=0; uint32 TriggerSpell=0; };
struct Info { Range* RangeEntry=nullptr; bool channel=false, damage=false; int cast=0; std::vector<Effect> Effects; bool IsChanneled() const{return channel;} int CalcCastTime() const{return cast;} };
using SpellInfo=Info;
struct Manager { std::map<uint32,Info> rows; Info const* GetSpellInfo(uint32 id){return rows.contains(id)?&rows[id]:nullptr;} } manager;
auto* sSpellMgr=&manager;
namespace Area52Holy { bool DamagingSpell(Info const* i){return i->damage;} }
struct Mod { uint32 spellId; int op=3, value=0; };
struct Player { bool a52=true; bool Match(Mod* mod,Info const* spellInfo){ ELIGIBILITY return false; } int Sum(std::vector<Mod> mods); };
namespace AscensionFreePick { bool Applies(Player const* p){return p && p->a52;} }
int Player::Sum(std::vector<Mod> mods){int shadowReach=0,similarRange=0,totalflat=0,op=3;for(auto& m:mods){auto* mod=&m;int flatValue=m.value;AGGREGATE totalflat+=flatValue;}if(shadowReach>similarRange)totalflat+=shadowReach-similarRange;return totalflat;}
struct AuraEffect { uint32 id=5570; uint32 GetId() const{return id;} };
struct Unit { Player* player=nullptr; AuraEffect* swarm=nullptr; bool hostile=true; int casts=0; uint32 last=0; Player* ToPlayer(){return player;} bool IsValidAttackTarget(Unit* e){return e->hostile;} void CastSpell(Unit* e,uint32 id,bool,void*,AuraEffect const*){e->casts++;e->last=id;} };
AuraEffect const* CastersInsectSwarm(Unit* t,Unit*){return t->swarm;}
std::list<Unit*> units;
namespace Acore { struct AnyUnfriendlyUnitInObjectRangeCheck { AnyUnfriendlyUnitInObjectRangeCheck(Unit*,Unit*,float){} }; template<class T>struct UnitListSearcher {std::list<Unit*>& result;UnitListSearcher(Unit*,std::list<Unit*>& r,T&):result(r){} }; }
namespace Cell {template<class T> void VisitObjects(Unit*,T& search,float){search.result=units;} }
struct ProcEventInfo {Unit* target;Unit* GetProcTarget(){return target;}};
constexpr float NATURES_REACH_YARDS=5;
constexpr uint32 NATURES_REACH_TARGETS=10;
struct Script {Unit* owner;Unit* GetTarget(){return owner;}void PreventDefaultAction(){} SPREAD };
int main(){Player p;Range range;Info cast{&range,false,true,2000,{}};Info instant=cast;instant.cast=0;Info heal=cast;heal.damage=false;Info channel=instant;channel.channel=true;Info trigger=channel;trigger.damage=false;trigger.Effects={{23,0,1}};manager.rows[1]=cast;
for(uint32 id:{16819u,16820u,816819u,17322u,17323u,817323u}){Mod m{id};assert(p.Match(&m,&cast));assert(p.Match(&m,&channel));assert(p.Match(&m,&trigger));assert(!p.Match(&m,&instant));assert(!p.Match(&m,&heal));m.op=0;assert(!p.Match(&m,&cast));}
for(int amount:{2,4,6}){assert(p.Sum({{16819,3,amount}})==amount);assert(p.Sum({{16819,3,amount},{17322,3,4}})==std::max(amount,4));assert(p.Sum({{17322,3,4},{16819,3,amount}})==std::max(amount,4));assert(p.Sum({{16819,3,amount},{11100,3,6}})==6);}
AuraEffect swarm;Unit owner{&p},target;target.swarm=&swarm;std::vector<Unit> enemies(15);for(auto& e:enemies)units.push_back(&e);units.push_back(&target);enemies[0].swarm=&swarm;enemies[1].hostile=false;Script script{&owner};ProcEventInfo event{&target};script.HandleProc(&swarm,event);int total=0;for(auto& e:enemies)total+=e.casts;assert(total==13);assert(target.casts==0);for(auto& e:enemies)e.casts=0;p.a52=false;script.HandleProc(&swarm,event);total=0;for(auto& e:enemies)total+=e.casts;assert(total==10);target.swarm=nullptr;for(auto& e:enemies)e.casts=0;script.HandleProc(&swarm,event);for(auto& e:enemies)assert(e.casts==0);
}
