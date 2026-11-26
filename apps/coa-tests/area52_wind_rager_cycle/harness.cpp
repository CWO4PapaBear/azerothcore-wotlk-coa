#include <cassert>
#include <map>
#include <set>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstdint>
#include "Area52WindScaling.h"
using uint32=unsigned;using uint8=unsigned char;using int32=int;using AuraRemoveMode=int;using Milliseconds=std::chrono::milliseconds;
constexpr int OFF_ATTACK=1;
constexpr int EFFECT_0=0,EFFECT_1=1,EFFECT_2=2,SPELL_AURA_DUMMY=4,SPELL_MISS_NONE=0,DOT=2,SPELL_SCHOOL_MASK_NORMAL=1;
constexpr unsigned PROC_FLAG_DONE_MELEE_AUTO_ATTACK=4,PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS=16,PROC_FLAG_DONE_RANGED_AUTO_ATTACK=64,PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS=256,PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG=65536;
bool Area52WindMetadataEnabled=false,Area52WindRagerRegistered=false;
struct Aura {unsigned id=0,stack=1;unsigned GetId(){return id;}unsigned GetStackAmount(){return stack;}};
struct AuraApplication {Aura aura;Aura*GetBase(){return &aura;}};
struct Player;
struct Unit {Player*player=nullptr;std::map<std::pair<unsigned,unsigned>,Aura>auras;Player*ToPlayer(){return player;}bool IsPlayer(){return player;}Aura*GetAura(unsigned id,unsigned owner){auto i=auras.find({id,owner});return i==auras.end()?nullptr:&i->second;}void RemoveAurasDueToSpell(unsigned id,unsigned owner){auras.erase({id,owner});}};
struct Player:Unit {bool area=true,offhand=true;void*GetWeaponForAttack(int,bool){return offhand?this:nullptr;}unsigned guid=7;std::set<unsigned>talents{978232};std::map<unsigned,int>cooldowns;unsigned casts=0,last=0;Player(){player=this;}bool HasAura(unsigned id)const{return talents.contains(id)||auras.contains({id,guid});}unsigned GetGUID(){return guid;}bool MatchesSpellInteraction(unsigned id,unsigned first)const{return id==first||(id==777001&&first==1680);}void ModifySpellCooldown(unsigned id,int delta){cooldowns[id]+=delta;}void CastSpell(Unit*target,unsigned id,bool){++casts;last=id;auto&a=target->auras[{id,guid}];if(id==978938)a.stack=a.id?std::min(10u,a.stack+1):1;else a.stack=1;a.id=id;}};
namespace AscensionFreePick{bool Applies(Player const*p){return p&&p->area;}}
struct SpellInfo{unsigned Id=0,ProcFlags=0,ProcChance=0,StackAmount=12;struct Effect{int ApplyAuraName=108,BasePoints=2;float RealPointsPerLevel=1;}Effects[3];};
struct SpellProcEntry{float Chance=10;Milliseconds Cooldown{6000};unsigned DisableEffectsMask=6;};
struct Manager{std::map<unsigned,SpellInfo>spells;bool procReady=false;SpellProcEntry proc;SpellInfo const*GetSpellInfo(unsigned id){auto i=spells.find(id);return i==spells.end()?nullptr:&i->second;}SpellProcEntry const*GetSpellProcEntry(unsigned){return procReady?&proc:nullptr;}unsigned GetFirstSpellInChain(unsigned id){return id>=984046&&id<=984049?983982:id;}std::vector<unsigned>GetSpellAndRelatives(unsigned){return {1680,1681};}}manager;Manager*sSpellMgr=&manager;
struct SpellScriptLoader{};template<class T>struct ScriptRegistry{static inline bool loaded=false;static T*GetScriptById(unsigned){static T object;return loaded?&object:nullptr;}};
struct ObjectMgr{unsigned id=0;std::multimap<unsigned,unsigned>bindings;unsigned GetScriptId(char const*){return id;}auto GetSpellScriptsBounds(unsigned id){return bindings.equal_range(id);}}objects;ObjectMgr*sObjectMgr=&objects;
struct Spell{Player*caster=nullptr;SpellInfo info;bool triggered=false;std::map<unsigned,unsigned>values;bool IsTriggered(){return triggered;}Unit*GetCaster(){return caster;}SpellInfo const*GetSpellInfo(){return &info;}unsigned GetScriptValue(unsigned id){return values[id];}void SetScriptValue(unsigned id,unsigned value){values[id]=value;}};
struct TargetInfo{int damage=1000,damageBeforeTakenMods=1200;};
struct DamageInfo{unsigned damage=1,type=1,school=1;unsigned GetDamage()const{return damage;}unsigned GetDamageType()const{return type;}unsigned GetSchoolMask()const{return school;}};
struct ProcEventInfo{DamageInfo damage;Player*actor=nullptr;DamageInfo const*GetDamageInfo(){return &damage;}Player*GetActor(){return actor;}};
struct AuraEffect{unsigned id=983974,index=0;unsigned GetId()const{return id;}unsigned GetEffIndex()const{return index;}};
@KIND@ @CONFIG@ @RISING@ @READY@
struct Probe{Player*target=nullptr;Player*GetTarget(){return target;}void PreventDefaultAction(){} @CHECK@ @PROC@ @CAST@ @DAMAGE@ @HIT@ @AMOUNT@ @REMOVE@};
int main(){assert(!HasArea52WindRagerHandlers());for(unsigned id:{978938u,978939u,983986u,983974u})manager.spells[id].Id=id;Area52WindMetadataEnabled=true;for(auto&[id,info]:manager.spells)ConfigureArea52WindRager(&info);Area52WindRagerRegistered=true;assert(!HasArea52WindRagerHandlers());manager.procReady=true;assert(!HasArea52WindRagerHandlers());objects.id=7;ScriptRegistry<SpellScriptLoader>::loaded=true;assert(!HasArea52WindRagerHandlers());objects.bindings.emplace(978232,7);assert(HasArea52WindRagerHandlers());manager.proc.Cooldown=Milliseconds(0);assert(!HasArea52WindRagerHandlers());manager.proc.Cooldown=Milliseconds(6000);assert(HasArea52WindRagerHandlers());
Player p,enemyPlayer;Unit enemy;Probe probe;probe.target=&p;ProcEventInfo event;event.actor=&p;assert(probe.CheckProc(event));event.damage.type=DOT;assert(!probe.CheckProc(event));event.damage.type=1;event.damage.school=8;assert(!probe.CheckProc(event));event.damage.school=1;event.damage.damage=0;assert(!probe.CheckProc(event));event.damage.damage=1;probe.HandleProc(nullptr,event);assert(p.HasAura(983986));
for(unsigned rank:{983982u,984046u,984047u,984048u,984049u}){p.auras[{983986,7}]={983986,1};Spell cast;cast.caster=&p;cast.info.Id=rank;probe.OnSpellBeforeEffects(&cast,&p,&cast.info);assert(!p.HasAura(983986));}
p.auras.clear();for(unsigned n=0;n<5;++n){Spell cast;cast.caster=&p;cast.info.Id=1680;probe.OnSpellBeforeEffects(&cast,&p,&cast.info);probe.OnSpellBeforeEffects(&cast,&p,&cast.info);if(n<4)assert(p.GetAura(978938,7)->stack==2*(n+1));}assert(!p.GetAura(978938,7)&&p.HasAura(978939));
std::vector<unsigned>ids{978946,978947,983983,983984};for(unsigned id=979927;id<=979940;++id)ids.push_back(id);for(unsigned id=984073;id<=984076;++id)ids.push_back(id);for(unsigned id=984083;id<=984086;++id)ids.push_back(id);
for(unsigned id:ids){p.auras.clear();p.cooldowns.clear();Spell cast;cast.caster=&p;cast.info.Id=id;p.auras[{978938,7}]={978938,5};TargetInfo normal;probe.OnSpellCalculatedTarget(&cast,&enemy,normal);assert(normal.damage==(Area52Wind::StrikeKind(id)==1?1200:1000));TargetInfo versus;probe.OnSpellCalculatedTarget(&cast,&enemyPlayer,versus);assert(versus.damage==(Area52Wind::StrikeKind(id)==1?1100:1000));p.auras[{978939,7}]={978939,1};TargetInfo boosted;probe.OnSpellCalculatedTarget(&cast,&enemy,boosted);assert(boosted.damage==(Area52Wind::StrikeKind(id)==1?1800:1500));probe.OnSpellHitResult(&cast,&enemy,0,1,0,true);probe.OnSpellHitResult(&cast,&enemy,0,1,0,true);assert(p.GetAura(978938,7)->stack==6);assert(p.cooldowns[1680]==(Area52Wind::StrikeKind(id)==1?-4000:0));}
for(unsigned tick=1;tick<=8;++tick)assert(Area52Wind::TornadoBaseTick(80,tick)==10);for(int total:{0,1,3,7,9,81,700}){int sum=0;for(unsigned tick=1;tick<=8;++tick)sum+=Area52Wind::TornadoBaseTick(total,tick);assert(sum==total);}
p.auras.clear();p.offhand=false;Spell noOffhand;noOffhand.caster=&p;noOffhand.info.Id=978947;probe.OnSpellHitResult(&noOffhand,&enemy,0,0,0,false);assert(!p.HasAura(978938));p.offhand=true;
AuraEffect effect;int amount=20;probe.OnAfterAuraEffectCalculateAmount(&effect,&p,amount);assert(amount==20);p.talents.insert(46917);probe.OnAfterAuraEffectCalculateAmount(&effect,&p,amount);assert(amount==10);
for(unsigned id:{978938u,978939u,983986u,983974u}){p.auras[{id,7}]={id,1};p.auras[{id,8}]={id,1};}AuraApplication app{{978232,1}};probe.OnAuraRemove(&p,&app,0);for(unsigned id:{978938u,978939u,983986u,983974u})assert(!p.GetAura(id,7)&&p.GetAura(id,8));
p.area=false;assert(!probe.CheckProc(event));Area52WindMetadataEnabled=false;assert(!HasArea52WindRagerHandlers());
}
