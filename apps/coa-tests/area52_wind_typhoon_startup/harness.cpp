bool HasArea52WindRagerHandlers() { return false; }
bool HasArea52DesolateHandlers() { return false; }
#include <cassert>
#include <map>
#include <functional>
#include <algorithm>
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
struct SpellScriptLoader{};template<class T>struct ScriptRegistry{static inline bool loaded=false;static T*GetScriptById(uint32){static T instance;return loaded?&instance:nullptr;}};
struct ObjectMgr{uint32 id=0;std::multimap<uint32,uint32>bindings;uint32 GetScriptId(char const*){return id;}auto GetSpellScriptsBounds(uint32 rank){return bindings.equal_range(rank);}}objects;ObjectMgr*sObjectMgr=&objects;
@TYPHOON_READY@
@CONFIG_LOAD@
@CONFIG@ @CONNECTED@
struct Probe{@HIT@ @REMOVE@};
void windfury(Player*player,Unit*target,uint32 spellId,int bonus){void*item=nullptr;void*aurEff=nullptr;@WINDFURY@}
int main(){assert(!HasArea52WindHandlers(977725));
 for(uint32 id:{977725u,977726u,61391u,61390u,61388u,61387u,53227u})manager.spells[id].Id=id;
 OnAfterConfigLoad(false);for(auto&[id,info]:manager.spells)ConfigureArea52Zephyr(&info);
 assert(!HasArea52WindHandlers(977725));RegisterArea52ControlledTyphoonReadiness(HasArea52ControlledTyphoonHandlers);objects.id=7;assert(!HasArea52WindHandlers(977725));ScriptRegistry<SpellScriptLoader>::loaded=true;assert(!HasArea52WindHandlers(977725));
 for(uint32 id:{61391u,61390u,61388u,61387u,53227u})objects.bindings.emplace(id,7);
 assert(HasArea52WindHandlers(977725));objects.bindings.erase(53227);assert(!HasArea52WindHandlers(977725));objects.bindings.emplace(53227,7);assert(HasArea52WindHandlers(977725));manager.spells.erase(977726);assert(!HasArea52WindHandlers(977725));manager.spells[977726].Id=977726;
 config.realm="seasonal";OnAfterConfigLoad(true);assert(!HasArea52WindHandlers(977725));config.realm="live";OnAfterConfigLoad(true);assert(HasArea52WindHandlers(977725));}
