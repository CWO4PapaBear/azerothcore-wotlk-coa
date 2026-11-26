bool HasArea52WindRagerHandlers() { return false; }
bool HasArea52DesolateHandlers() { return false; }
#include <cassert>
#include <map>
#include <functional>
#include <string>
#include <type_traits>
#include <cstdint>
#include <initializer_list>
using int32=int;using uint32=unsigned;using uint8=uint8_t;using AuraRemoveMode=int;
constexpr int EFFECT_0=0,EFFECT_1=1,EFFECT_2=2,SPELL_AURA_DUMMY=4,SPELL_EFFECT_NONE=0,SPELL_MISS_NONE=0,SPELLVALUE_BASE_POINT0=0;
bool Area52WindMetadataEnabled=false,Area52ZephyrRegistered=false;
struct StackInfo{int StackAmount=10;int CalcMaxAuraStacks(void*)const{return StackAmount;}bool HasAttribute(int)const{return false;}};
constexpr int SPELL_ATTR1_AURA_UNIQUE=1;
struct Aura{uint32 id=414003;int m_stackAmount=0;int&stacks=m_stackAmount;StackInfo info;StackInfo*m_spellInfo=&info;bool removed=false;int refreshes=0,updates=0;void*GetCaster(){return nullptr;}int GetStackAmount(){return m_stackAmount;}uint32 GetId(){return id;}void Remove(int){removed=true;}void RefreshSpellMods(){++refreshes;}void RefreshTimers(bool){}void SetCharges(int){}int CalcMaxCharges(){return 0;}void SetStackAmount(int n){m_stackAmount=n;}void SetNeedClientUpdateForTargets(){++updates;}bool ModStackAmount(int32,AuraRemoveMode,bool=false);};
@STACK@
struct AuraApplication{Aura aura;Aura*GetBase(){return &aura;}};
struct SpellInfo{uint32 Id=0,StackAmount=30;struct Effect{uint32 Effect=6,ApplyAuraName=42,TriggerSpell=1;bool IsAura(uint32 type)const{return Effect==6&&ApplyAuraName==type;}}Effects[3];};
struct Player{std::function<void(uint32)> onDamage;bool area=true;std::map<uint32,Aura>auras;int hits=0,grants=0;uint32 lastSpell=0;int lastBonus=0;Player*ToPlayer(){return this;}bool HasAura(uint32 id){return auras.contains(id);}uint32 GetGUID(){return 7;}Aura*GetAura(uint32 id,uint32){auto it=auras.find(id);return it==auras.end()?nullptr:&it->second;}void RemoveAurasDueToSpell(uint32 id,uint32 owner){assert(owner==7);auras.erase(id);}void CastSpell(Player*,uint32 id,bool){assert(id==414003);++grants;auto&a=auras[id];a.id=id;a.ModStackAmount(1,0);}void CastCustomSpell(uint32 id,int,int bonus,Player*,bool,void*,void*){++hits;lastSpell=id;lastBonus=bonus;if(onDamage)onDamage(id);}};using Unit=Player;
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct Manager{std::map<uint32,SpellInfo>spells;SpellInfo const*GetSpellInfo(uint32 id){auto it=spells.find(id);return it==spells.end()?nullptr:&it->second;}}manager;Manager*sSpellMgr=&manager;
struct Spell{Player*caster=nullptr;SpellInfo info;std::map<uint32,uint32>values;Player*GetCaster(){return caster;}SpellInfo const*GetSpellInfo(){return &info;}uint32 GetScriptValue(uint32 k){return values[k];}void SetScriptValue(uint32 k,uint32 v){values[k]=v;}};
struct Config{bool enabled=true;std::string model="hero",realm="live";template<class T>T GetOption(std::string const&key,T fallback){if constexpr(std::is_same_v<T,bool>)return enabled;else return key=="CoA.ClassModel"?model:realm;}}config;Config*sConfigMgr=&config;
bool typhoonReady=false;bool HasArea52ControlledTyphoonHandlers(){return typhoonReady;}
@CONFIG_LOAD@
@CONFIG@ @CONNECTED@
struct Probe{@HIT@ @REMOVE@};
void windfury(Player*player,Unit*target,uint32 spellId,int bonus){void*item=nullptr;void*aurEff=nullptr;@WINDFURY@}
int main(){Aura stackProbe;for(int i=0;i<30;++i)stackProbe.ModStackAmount(1,0);assert(stackProbe.GetStackAmount()==10&&stackProbe.updates==30);stackProbe.ModStackAmount(-1,0);assert(stackProbe.GetStackAmount()==9);assert(stackProbe.ModStackAmount(-9,0)&&stackProbe.removed);assert(!HasArea52WindHandlers(979769));for(uint32 id:{979769u,414003u,25504u,33750u})manager.spells[id].Id=id;OnAfterConfigLoad(false);assert(Area52WindMetadataEnabled);for(auto&[id,info]:manager.spells)ConfigureArea52Zephyr(&info);assert(!HasArea52WindHandlers(979769));Area52ZephyrRegistered=true;assert(HasArea52WindHandlers(979769)&&!HasArea52WindHandlers(1));assert(manager.spells[414003].StackAmount==10);manager.spells.erase(33750);assert(!HasArea52WindHandlers(979769));manager.spells[33750].Id=33750;assert(HasArea52WindHandlers(979769));
Player p,target;p.auras[979769].id=979769;Probe probe;for(uint32 id:{25504u,33750u})for(int i=0;i<7;++i){Spell spell;spell.caster=&p;spell.info.Id=id;int before=p.grants;probe.OnSpellHitResult(&spell,&target,0,100,0,false);probe.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.grants==before+1);}assert(p.auras[414003].stacks==10);windfury(&p,&target,33750,42);assert(p.hits==4&&p.lastSpell==33750&&p.lastBonus==42&&!p.auras.contains(414003));windfury(&p,&target,25504,42);assert(p.hits==6);p.auras[414003].stacks=9;windfury(&p,&target,25504,42);assert(p.hits==8&&p.auras[414003].stacks==9);p.auras[414003].stacks=10;p.area=false;windfury(&p,&target,25504,42);assert(p.hits==10&&p.auras[414003].stacks==10);p.area=true;
Spell bad;bad.caster=&p;bad.info.Id=403;int before=p.grants;probe.OnSpellHitResult(&bad,&target,0,100,0,false);bad.info.Id=25504;probe.OnSpellHitResult(&bad,&target,1,100,0,false);probe.OnSpellHitResult(&bad,&target,0,0,0,false);assert(p.grants==before);AuraApplication app;app.aura.id=979769;probe.OnAuraRemove(&p,&app,0);assert(!p.auras.contains(414003));probe.OnAuraRemove(nullptr,nullptr,0);probe.OnSpellHitResult(nullptr,nullptr,0,0,0,false);
 Player cycle;cycle.auras[979769].id=979769;cycle.onDamage=[&](uint32 id){Spell spell;spell.caster=&cycle;spell.info.Id=id;probe.OnSpellHitResult(&spell,&target,0,100,0,false);};
 for(int i=0;i<5;++i)windfury(&cycle,&target,25504,42);assert(cycle.hits==10&&cycle.auras.at(414003).stacks==10);
 windfury(&cycle,&target,33750,42);assert(cycle.hits==14&&cycle.auras.at(414003).stacks==4);
 config.model="coa";OnAfterConfigLoad(true);assert(!HasArea52WindHandlers(979769));SpellInfo other;other.Id=414003;ConfigureArea52Zephyr(&other);assert(other.StackAmount==30);
 config.model="hero";config.realm="seasonal";OnAfterConfigLoad(true);assert(!HasArea52WindHandlers(979769));config.realm="live";OnAfterConfigLoad(true);assert(HasArea52WindHandlers(979769));
}
