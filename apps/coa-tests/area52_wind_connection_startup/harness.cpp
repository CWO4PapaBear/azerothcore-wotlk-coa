#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <vector>
using uint32=unsigned;
constexpr int EFFECT_1=1,EFFECT_2=2,SPELL_AURA_DUMMY=4,PROC_SPELL_PHASE_CAST=1,PROC_ATTR_REQ_SPELLMOD=8,PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG=65536;
bool Area52WindMetadataEnabled=false,Area52DesolateRegistered=false;bool(*Area52WindfuryReady)()=nullptr;
struct Player;
struct Unit{bool alive=true;bool IsAlive(){return alive;}};
struct Player:Unit{bool area=true;std::set<unsigned>auras{997082};std::vector<unsigned>casts;bool HasAura(unsigned id)const{return auras.contains(id);}void CastSpell(Unit*,unsigned id,bool b){assert(b);casts.push_back(id);}};
namespace AscensionFreePick{bool Applies(Player const*p){return p&&p->area;}}
struct SpellInfo{unsigned Id=0;struct Effect{unsigned ApplyAuraName=226,TriggerSpell=99;bool IsAura(int a)const{return ApplyAuraName==a;}}Effects[3];};
struct SpellProcEntry{int Charges=1,SpellPhaseMask=1,AttributesMask=8,ProcFlags=65536;};
struct Manager{std::map<unsigned,SpellInfo>spells;SpellProcEntry entry;bool proc=false;SpellInfo const*GetSpellInfo(unsigned id){auto i=spells.find(id);return i==spells.end()?nullptr:&i->second;}SpellProcEntry const*GetSpellProcEntry(unsigned){return proc?&entry:nullptr;}}manager;Manager*sSpellMgr=&manager;
struct SpellScriptLoader{};template<class T>struct ScriptRegistry{static inline bool loaded=false;static T*GetScriptById(unsigned){static T t;return loaded?&t:nullptr;}};
struct ObjectMgr{unsigned id=0;std::multimap<unsigned,unsigned>bindings;unsigned GetScriptId(char const*){return id;}auto GetSpellScriptsBounds(unsigned id){return bindings.equal_range(id);}}objects;ObjectMgr*sObjectMgr=&objects;
@FUNCTIONS@
@CONNECTIONS@
int main(){OnStartup();assert(Connected.empty());assert(!HasArea52DesolateHandlers());for(unsigned id:{997082u,997084u,300065u})manager.spells[id].Id=id;
Area52WindMetadataEnabled=true;ConfigureArea52Desolate(&manager.spells[997082]);OnStartup();assert(Connected.empty());assert(!HasArea52DesolateHandlers());Area52DesolateRegistered=true;RegisterArea52WindfuryReadiness(HasArea52WindfuryHandler);OnStartup();assert(Connected.empty());assert(!HasArea52DesolateHandlers());manager.proc=true;objects.id=7;ScriptRegistry<SpellScriptLoader>::loaded=true;OnStartup();assert(Connected.empty());assert(!HasArea52DesolateHandlers());objects.bindings.emplace(33757,7);OnStartup();assert(Connected.count(997082)==1);assert(HasArea52DesolateHandlers());manager.entry.SpellPhaseMask=2;OnStartup();assert(Connected.empty());assert(!HasArea52DesolateHandlers());manager.entry.SpellPhaseMask=1;OnStartup();assert(Connected.count(997082)==1);assert(HasArea52DesolateHandlers());Area52WindMetadataEnabled=false;OnStartup();assert(Connected.empty());assert(!HasArea52DesolateHandlers());Area52WindMetadataEnabled=true;
Player player;Unit target;ApplyArea52DesolateWindfury(&player,&target);assert((player.casts==std::vector<unsigned>{997084,300065}));player.casts.clear();player.auras.insert(978231);ApplyArea52DesolateWindfury(&player,&target);assert(player.casts.empty());player.auras.erase(978231);target.alive=false;ApplyArea52DesolateWindfury(&player,&target);assert(player.casts.empty());target.alive=true;player.area=false;ApplyArea52DesolateWindfury(&player,&target);assert(player.casts.empty());
}
