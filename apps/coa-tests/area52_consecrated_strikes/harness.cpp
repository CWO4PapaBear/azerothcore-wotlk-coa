#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <set>
#include "AscensionMysticSpellGrants.h"
using uint32=uint32_t;using uint8=uint8_t;using int32=int32_t;
constexpr int EFFECT_0=0,SPELL_AURA_DUMMY=4,ALLSPELLHOOK_ON_HIT_RESULT=1,SPELL_MISS_NONE=0,SPELLFAMILY_PALADIN=10,SPELLVALUE_MAX_TARGETS=1;
struct Effect{int ApplyAuraName=42;uint32 TriggerSpell=20424;};struct SpellInfo{uint32 Id=35395,ProcFlags=16,ProcCharges=0,StackAmount=1,SpellLevel=10,SpellFamilyName=10;std::array<uint32,3>SpellFamilyFlags{};Effect Effects[3];};
bool Enabled=false,groundReady=false,weaponReady=false;bool HasArea52ConsecratedGroundHandlers(){return groundReady;}bool ConsecratedWeaponReady(){return weaponReady;}
struct Manager{std::map<uint32,SpellInfo>records;SpellInfo const*GetSpellInfo(uint32 id){auto i=records.find(id);return i==records.end()?nullptr:&i->second;}}manager,*sSpellMgr=&manager;
struct Player;struct Unit{Player*player=nullptr;bool alive=true;Player*ToPlayer(){return player;}bool IsAlive(){return alive;}void CastCustomSpell(uint32,int,int,Unit*,bool,void*,void const*);};
struct Player:Unit{bool area=true,owned=true,aura=true;uint32 level=10;int casts=0,targets=0;uint32 payload=0;Player(){player=this;}bool HasAura(uint32){return aura;}bool HasSpell(uint32){return false;}uint32 GetLevel(){return level;}bool MatchesSpellInteraction(uint32 id,uint32 original){return id==original||(id==999999&&original==35395);}void CastSpell(Player*,uint32 id,bool){++casts;payload=id;}};
void Unit::CastCustomSpell(uint32 id,int,int count,Unit*,bool,void*,void const*){player->payload=id;player->targets=count;++player->casts;}
bool Applies(Player*p){return p&&p->area;}namespace AscensionFreePick{bool Applies(Player*p){return ::Applies(p);}}namespace AscensionMysticEnchant{bool GrantsActive(Player*p,uint32){return p&&p->owned;}}
struct Spell{Player*caster;SpellInfo info;bool triggered=false;std::map<uint32,uint32>state;Unit*GetCaster(){return caster;}SpellInfo const*GetSpellInfo(){return &info;}bool IsTriggered(){return triggered;}uint32 GetScriptValue(uint32 key){return state[key];}void SetScriptValue(uint32 key,uint32 value){state[key]=value;}};
struct AllSpellScript{AllSpellScript(char const*,std::initializer_list<int>){}};int rolls=0;bool roll=true;bool roll_chance_i(int n){assert(n==50);++rolls;return roll;}
struct AuraEffect{SpellInfo info;SpellInfo const*GetSpellInfo()const{return &info;}};struct ProcEventInfo{Player*actor;Unit*target;SpellInfo const*info;Unit*GetActor(){return actor;}Unit*GetActionTarget(){return target;}SpellInfo const*GetSpellInfo(){return info;}};
@PRODUCTION@
void Register(){@REGISTER@}
struct Seal{void PreventDefaultAction(){}@SEAL@};
bool Due(Player*player,SpellInfo const*info){AscensionMysticEnchant::SpellGrant grant{965295,278745};uint32 rank=278745;using AscensionMysticEnchant::GrantRankDue;@LEVEL@return rankDue;}
int main(){assert(!ConsecratedStrikesReady());Register();assert(!ConsecratedStrikesReady());Enabled=true;weaponReady=true;assert(!ConsecratedStrikesReady());groundReady=true;assert(!ConsecratedStrikesReady());for(uint32 id:{278745u,278746u,965321u,965903u,977847u,977848u,977849u})manager.records[id].Id=id;assert(ConsecratedStrikesReady());Register();assert(ConsecratedStrikesReady());manager.records.erase(965903);assert(!ConsecratedStrikesReady());manager.records[965903].Id=965903;assert(ConsecratedStrikesReady());Enabled=false;assert(!ConsecratedStrikesReady());Enabled=true;Player p;Unit target;ConsecratedStrikesCasts casts;SpellInfo metadata;metadata.Id=965295;ConfigureConsecratedStrikes(&metadata);assert(metadata.ProcFlags==0&&metadata.Effects[0].ApplyAuraName==4&&metadata.Effects[0].TriggerSpell==0);
assert(std::any_of(AscensionMysticEnchant::SpellGrants.begin(),AscensionMysticEnchant::SpellGrants.end(),[](auto const&e){return e.Enchant==965295&&e.Spell==278745;}));for(uint32 level:{1u,9u,10u,80u,9u}){p.level=level;assert(Due(&p,&metadata)==(level>=10));}
for(uint32 id:{35395u,53385u,53595u,999999u,278745u,42u}){Spell spell{&p};spell.info.Id=id;int before=p.casts;casts.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==before+(id==278745||id==42?0:1));int previous=rolls;casts.OnSpellHitResult(&spell,&target,0,100,0,false);assert(rolls==previous);}
for(int x=0;x<6;++x){Spell spell{&p};p.area=x!=0;p.owned=x!=1;p.aura=x!=2;spell.triggered=x==3;int before=p.casts;casts.OnSpellHitResult(&spell,&target,x==4?1:0,x==5?0:100,100,false);assert(p.casts==before);}p.area=p.owned=p.aura=true;roll=false;Spell missedRoll{&p};int before=rolls;casts.OnSpellHitResult(&missedRoll,&target,0,100,0,false);casts.OnSpellHitResult(&missedRoll,&target,0,100,0,false);assert(rolls==before+1);
Seal seal;AuraEffect aura;SpellInfo incoming;ProcEventInfo event{&p,&target,&incoming};seal.HandleProc(&aura,event);assert(p.payload==278746&&p.targets==3);incoming.SpellFamilyFlags[1]=0x20000;seal.HandleProc(&aura,event);assert(p.payload==278746&&p.targets==1);for(int x=0;x<3;++x){p.area=x!=0;p.owned=x!=1;p.aura=x!=2;seal.HandleProc(&aura,event);assert(p.payload==20424);}p.area=p.owned=p.aura=true;target.alive=false;before=p.casts;seal.HandleProc(&aura,event);assert(p.casts==before);}
