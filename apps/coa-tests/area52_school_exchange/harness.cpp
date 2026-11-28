#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <set>
using uint32=uint32_t;
constexpr int SPELL_SCHOOL_MASK_NORMAL=1,SPELL_SCHOOL_MASK_HOLY=2,SPELL_SCHOOL_MASK_FIRE=4,SPELL_SCHOOL_MASK_NATURE=8,SPELL_SCHOOL_MASK_SHADOW=32,SPELL_SCHOOL_MASK_ARCANE=64;
constexpr int SPELL_EFFECT_SCHOOL_DAMAGE=2,SPELL_EFFECT_HEALTH_LEECH=9,SPELL_EFFECT_WEAPON_DAMAGE=58,SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL=17,SPELL_EFFECT_NORMALIZED_WEAPON_DMG=121,SPELL_EFFECT_WEAPON_PERCENT_DAMAGE=31,SPELL_EFFECT_POWER_BURN=62,SPELL_EFFECT_TRIGGER_SPELL=64,SPELL_EFFECT_TRIGGER_MISSILE=32;
constexpr int SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE=142,SPELL_EFFECT_TRIGGER_MISSILE_SPELL_WITH_VALUE=148,SPELL_AURA_PERIODIC_TRIGGER_SPELL=23,SPELL_AURA_PERIODIC_TRIGGER_SPELL_FROM_CLIENT=48,SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE=227;
constexpr int SPELL_AURA_PERIODIC_DAMAGE=3,SPELL_AURA_PERIODIC_DAMAGE_PERCENT=89,SPELL_AURA_PERIODIC_LEECH=53,SPELL_AURA_POWER_BURN=162,ALLSPELLHOOK_ON_CAST=1;
struct EffectData{int Effect=0;uint32 TriggerSpell=0;int ApplyAuraName=0;};
struct SpellInfo{uint32 Id=1,school=1;std::array<EffectData,3>Effects;std::set<int>auras;uint32 GetSchoolMask()const{return school;}bool HasEffect(int id)const{for(auto const&e:Effects)if(e.Effect==id)return true;return false;}bool HasAura(int id)const{return auras.contains(id);}};
struct Manager{std::map<uint32,SpellInfo>records;SpellInfo const*GetSpellInfo(uint32 id){auto i=records.find(id);return i==records.end()?nullptr:&i->second;}}manager,*sSpellMgr=&manager;
struct Player;struct Unit{Player*player=nullptr;Player*ToPlayer(){return player;}};
struct Session{bool logout=false;bool PlayerLogout(){return logout;}};
struct Player:Unit{bool IsValidAttackTarget(Unit*target){return target&&target!=this;}bool MatchesSpellInteraction(uint32 id,uint32 original){return id==original||(original==20473&&id==48825);}bool area=true,loaded=true,ready=true,auraRestored=true;Session session;std::set<uint32>owned,known;std::map<uint32,int>stacks;std::set<uint32>foreign;int casts=0;Player(){player=this;}Session*GetSession(){return &session;}uint32 GetGUID(){return 7;}bool HasSpell(uint32 id){return known.contains(id);}bool HasAura(uint32 id){return stacks.contains(id)||foreign.contains(id)||(auraRestored&&owned.contains(id));}void CastSpell(Player*,uint32 id,bool triggered){assert(triggered);stacks[id]=std::min(stacks[id]+1,4);++casts;}void RemoveAurasDueToSpell(uint32 id,uint32 guid){assert(guid==7);stacks.erase(id);}};
bool Enabled=false;bool Applies(Player*p){return Enabled&&p&&p->area;}
namespace AscensionMysticEnchant{bool GrantsActive(Player*p,uint32 id){return p->owned.contains(id);}bool CollectionLoaded(Player*p){return p->loaded;}bool DraftEquipmentReady(Player*p){return p->ready;}}
struct Targets{Unit*target=nullptr;Unit*GetUnitTarget(){return target;}};
struct Spell{Targets m_targets;uint32 school=1;uint32 GetSpellSchoolMask(){return school;}bool triggered=false,automatic=false;std::map<uint32,uint32>state;bool IsTriggered(){return triggered;}bool IsAutoRepeat(){return automatic;}uint32 GetScriptValue(uint32 id){return state[id];}void SetScriptValue(uint32 id,uint32 v){state[id]=v;}};
struct AllSpellScript{AllSpellScript(char const*,std::initializer_list<int>){}};struct PlayerScript{PlayerScript(char const*){}};
namespace Area52Holy{
@DAMAGE@
}
@PRODUCTION@
int main(){
SchoolExchangeCasts casts;SchoolExchangeLifecycle lifecycle;Player p;SpellInfo info;info.Effects[0].Effect=2;
assert(!SchoolExchangeReady(84579));Enabled=true;assert(!SchoolExchangeReady(84579));SchoolExchangeRegistered=true;assert(!SchoolExchangeReady(84579));
for(uint32 id:{286100u,286101u,286102u,286103u,286105u,286106u})manager.records[id].Id=id;
for(auto const&e:SchoolExchanges){assert(SchoolExchangeReady(e.enchant));p.owned={e.enchant};for(uint32 school:{1u,e.school,uint32(1|e.school),16u}){p.stacks.clear();Spell spell;spell.school=school;info.school=school;casts.OnSpellCast(&spell,&p,&info,false);assert(p.stacks.contains(e.helper)==bool(school&1));assert(p.stacks.contains(286100)==bool(school&e.school));int before=p.casts;casts.OnSpellCast(&spell,&p,&info,false);assert(p.casts==before);}}
p.owned={84579,84581};p.stacks.clear();info.school=7;Spell multi;multi.school=7;casts.OnSpellCast(&multi,&p,&info,false);assert(p.stacks[286101]==1&&p.stacks[286102]==1&&p.stacks[286100]==2);int before=p.casts;casts.OnSpellCast(&multi,&p,&info,false);assert(p.casts==before);Spell second;second.school=7;casts.OnSpellCast(&second,&p,&info,false);assert(p.stacks[286100]==4);
for(int excluded=0;excluded<5;++excluded){Spell spell;SpellInfo no=info;if(excluded==0)spell.triggered=true;if(excluded==1)spell.automatic=true;if(excluded==2)no.Id=75;if(excluded==3)no.Id=6603;if(excluded==4)no.Effects[0].Effect=10;before=p.casts;casts.OnSpellCast(&spell,&p,&no,false);assert(before==p.casts);}
p.auraRestored=false;Spell absent;before=p.casts;casts.OnSpellCast(&absent,&p,&info,false);assert(p.casts==before);p.auraRestored=true;
Spell dot;SpellInfo damageOverTime;damageOverTime.school=1;damageOverTime.auras={3};before=p.casts;casts.OnSpellCast(&dot,&p,&damageOverTime,false);assert(p.casts==before+2);
Spell wrapper;wrapper.school=2;SpellInfo wrapped;wrapped.Id=100;wrapped.school=2;wrapped.Effects[0]={64,101};manager.records[101]=info;manager.records[101].Id=101;before=p.casts;casts.OnSpellCast(&wrapper,&p,&wrapped,false);assert(p.casts==before+1);manager.records[101]=wrapped;manager.records[101].Id=101;Spell cycle;before=p.casts;casts.OnSpellCast(&cycle,&p,&wrapped,false);assert(p.casts==before);
p.stacks={{286100,4},{286101,2},{286102,3}};p.owned={84581};CleanupSchoolExchange(&p);assert(p.stacks[286100]==4&&!p.stacks.contains(286101)&&p.stacks[286102]==3);p.owned.clear();p.loaded=false;CleanupSchoolExchange(&p);assert(p.stacks.size()==2);p.loaded=true;p.ready=false;lifecycle.OnPlayerLogin(&p);assert(p.stacks.size()==2);p.ready=true;lifecycle.OnPlayerUpdate(&p,1);assert(p.stacks.empty());
p.stacks={{286100,2},{286101,1}};p.known={286100};p.foreign={286101};CleanupSchoolExchange(&p);assert(p.stacks[286100]==2&&!p.stacks.contains(286101)&&p.foreign.contains(286101));p.known.clear();p.session.logout=true;CleanupSchoolExchange(&p);assert(p.stacks.contains(286100));p.session.logout=false;p.area=false;CleanupSchoolExchange(&p);assert(p.stacks.contains(286100));p.area=true;CleanupSchoolExchange(&p);assert(p.stacks.empty());
p.owned={84579};Spell converted;converted.school=3;info.school=4;p.stacks.clear();casts.OnSpellCast(&converted,&p,&info,false);assert(p.stacks[286100]==1&&p.stacks[286101]==1);
for(uint32 id:{20271u,53407u,53408u,20473u,48825u,954817u,47540u}){Spell scripted;scripted.school=2;Unit enemy;scripted.m_targets.target=&enemy;SpellInfo wrapper;wrapper.Id=id;before=p.casts;casts.OnSpellCast(&scripted,&p,&wrapper,false);assert(p.casts==before+1);Spell healing;healing.school=2;healing.m_targets.target=&p;before=p.casts;casts.OnSpellCast(&healing,&p,&wrapper,false);assert(p.casts==before);}
for(int aura:{23,48,227}){Spell channel;channel.school=3;SpellInfo info;info.Id=200;info.Effects[0]={6,201,aura};manager.records[201].Id=201;manager.records[201].Effects[0].Effect=2;before=p.casts;casts.OnSpellCast(&channel,&p,&info,false);assert(p.casts==before+2);Spell tick;tick.triggered=true;before=p.casts;casts.OnSpellCast(&tick,&p,&manager.records[201],false);assert(p.casts==before);manager.records[201].Effects[0].Effect=10;Spell healing;before=p.casts;casts.OnSpellCast(&healing,&p,&info,false);assert(p.casts==before);}
for(int effect:{142,148}){Spell value;value.school=3;SpellInfo info;info.Id=200;info.Effects[0]={effect,201,0};manager.records[201].Effects[0].Effect=2;before=p.casts;casts.OnSpellCast(&value,&p,&info,false);assert(p.casts==before+2);}
@CHANNEL_RECORDS@
manager.records.erase(286101);assert(!SchoolExchangeReady(84579));manager.records[286101].Id=286101;assert(SchoolExchangeReady(84579));Enabled=false;assert(!SchoolExchangeReady(84579));
}
