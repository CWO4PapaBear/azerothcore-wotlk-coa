from pathlib import Path
import os,subprocess,json
W=Path(os.environ['AREA52_BOT_REVIEW']).parent;T=W/'draft-save-20261030'
S=Path('/home/dml/builds/area52-playerbots-20261026/source')
source=(T/'source/src/server/coa/AscensionMysticEnchant.cpp').read_text()
def function(text,signature):
 start=text.index(signature);opening=text.index('{',start);depth=1;end=opening+1
 while depth:
  depth+=(text[end]=='{')-(text[end]=='}');end+=1
 return text[start:end]
validator=function((S/'src/server/coa/AscensionMysticEnchantCatalog.cpp').read_text(),'bool ValidateSlots(')
sync=source[source.index('void Synchronize(Player* player)'):source.index('    for (auto const& grant : SpellGrants)')]+'}\n'
code=r'''
#include "AscensionMysticEnchantCatalog.h"
#include "Area52BuildProtocol.h"
#include <algorithm>
#include <cassert>
struct Aura { unsigned stacks=0; void SetStackAmount(std::uint8_t value){stacks=value;} };
struct DataAdapter { void* state=nullptr;template<class T>T* Get(char const*)const{return static_cast<T*>(state);} };
class Player {
public:
 DataAdapter CustomData;unsigned level=1,investment=0;bool online=true,alive=true,casting=false;
 std::map<unsigned,Aura> auras;
 unsigned GetLevel()const{return level;}bool IsInWorld()const{return online;}bool IsAlive()const{return alive;}
 unsigned GetGUID()const{return 1;}
 bool IsNonMeleeSpellCast(bool)const{return casting;}
 Aura* GetAura(unsigned id,unsigned){auto it=auras.find(id);return it==auras.end()?nullptr:&it->second;}
 Aura* AddAura(unsigned id,Player*){return &auras[id];}
 void RemoveAurasDueToSpell(unsigned id,unsigned){auras.erase(id);}
};
namespace AscensionFreePick { unsigned Investment(Player const* player,unsigned,unsigned,bool){return player->investment;} }
namespace AscensionMysticEnchant {
struct State{bool Loaded=false,Busy=false;std::set<unsigned> Known,DraftEntitlement,Applied;};
constexpr char StateKey[]="test";Catalog Data;std::map<unsigned,unsigned> Connected;Slots equipped{};
constexpr unsigned SPELL_GROUP_STACK_RULE_DEFAULT=0;
struct SpellManager {bool conflicts=false;unsigned GetSpellInfo(unsigned id){return id;}unsigned CheckSpellGroupStackRules(unsigned,unsigned){return conflicts?1:0;}} manager;
auto* sSpellMgr=&manager;
bool failEquip=false;
bool Applies(Player*){return true;}
bool CanChange(Player* p,State const* s){return p->online&&s->Loaded&&!s->Busy;}
Slots ReadSlots(Player const*){return equipped;}
bool Equip(Player*,Slots const& wanted,bool admin){assert(admin);if(failEquip)return false;equipped=wanted;return true;}
'''
code+=validator+'\n'
code+=function(source,'bool DraftEquipmentReady(')+'\n'+function(source,'bool CanApplyDraftEnchants(')+'\n'
code+=function(source,'bool InvestmentMet(')+'\n'+function(source,'bool ValidateOwnedSlots(')+'\n'+sync
code+=function(source,'bool ApplyDraftEnchants(')+'\n'+function(source,'bool ClearDraftEnchants(')+'\n}\n'
code+=r'''
int main(){
 using namespace AscensionMysticEnchant;
 Player player;State state;player.CustomData.state=&state;state.Known.insert(100);
 for(unsigned id=100;id<120;++id){Entry e;e.Spell=id;e.Quality=2;e.Level=1;Data.BySpell[id]=e;Connected[id]=3;}
 Data.BySpell[101].TalentInvestment[0]=5;
 Area52Build::Enchant enchant{};enchant.Id=101;enchant.Stacks=2;
 std::vector<Area52Build::Enchant> plan{enchant};
 assert(!ApplyDraftEnchants(&player,plan));assert(equipped[0]==0);
 state.Loaded=true;assert(CanApplyDraftEnchants(&player,plan));assert(ApplyDraftEnchants(&player,plan));
 assert(equipped[0]==101&&equipped[1]==101&&state.Known==std::set<unsigned>{100});
 assert(player.auras.empty());player.investment=5;Synchronize(&player);
 assert(player.auras.at(101).stacks==2);player.investment=0;Synchronize(&player);
 assert(player.auras.empty()&&equipped[0]==101);
 assert(ApplyDraftEnchants(&player,plan)&&state.Known==std::set<unsigned>{100});
 auto saved=equipped;auto entitlement=state.DraftEntitlement;
 plan[0].Stacks=18;assert(!ApplyDraftEnchants(&player,plan)&&equipped==saved);
 plan[0].Stacks=4;assert(!ApplyDraftEnchants(&player,plan)&&equipped==saved);
 plan[0].Id=999;plan[0].Stacks=1;assert(!ApplyDraftEnchants(&player,plan)&&equipped==saved);
 plan[0].Id=102;failEquip=true;assert(!ApplyDraftEnchants(&player,plan)&&state.DraftEntitlement==entitlement);
 failEquip=false;equipped[2]=100;state.Busy=true;assert(!ClearDraftEnchants(&player)&&equipped[2]==100);
 state.Busy=false;assert(ClearDraftEnchants(&player)&&equipped==Slots{}&&state.DraftEntitlement.empty());
 assert(state.Known==std::set<unsigned>{100});
 plan.clear();for(unsigned id=100;id<117;++id){enchant.Id=id;enchant.Stacks=1;plan.push_back(enchant);}
 assert(ApplyDraftEnchants(&player,plan));assert(equipped[16]==116);
 assert(CanApplyDraftEnchants(&player,plan));manager.conflicts=true;assert(!CanApplyDraftEnchants(&player,plan));manager.conflicts=false;
 player.casting=true;assert(!CanApplyDraftEnchants(&player,plan));player.casting=false;
 for(unsigned id=100;id<104;++id){Data.BySpell[id].Worldforged=true;Data.BySpell[id].Quality=3;}
 assert(!CanApplyDraftEnchants(&player,plan));
 assert(ClearDraftEnchants(&player));
}
'''
(T/'runtime-adapter.cpp').write_text(code)
args=['docker','run','--rm','--network','none','--cpus','1','-v',str(T)+':/results','-v',str(S)+':/workspace:ro','local/ascension-freepick-builder:20261001','bash','-lc','clang++ -std=c++20 -I/results/source/src/server/coa -I/workspace/src/server/coa /results/runtime-adapter.cpp -o /results/runtime-adapter && /results/runtime-adapter']
subprocess.run(args,check=True)
active=(T/'source/src/server/coa/Area52ActiveBuild.cpp').read_text()
lifecycle=r'''
#include "Area52BuildProtocol.h"
#include <array>
#include <map>
#include <cassert>
#include <charconv>
struct Value{unsigned value=0;};
struct DataAdapter{void* state=nullptr;template<class T>T* Get(char const*)const{return static_cast<T*>(state);}};
class Player{public:DataAdapter CustomData;std::map<std::pair<std::string,unsigned>,Value> settings;bool applies=true,safe=true,clear=true;unsigned saves=0,clears=0,updates=0;
Value GetPlayerSetting(std::string const& key,unsigned index){return settings[{key,index}];}
void UpdatePlayerSetting(std::string const& key,unsigned index,unsigned value){settings[{key,index}]={value};}
void SaveToDB(bool,bool){++saves;}};
namespace AscensionFreePick{constexpr std::array<unsigned,3> SpecializationSpells={1,2,3};}
namespace AscensionMysticEnchant{bool ClearDraftEnchants(Player* p){if(!p->clear)return false;++p->clears;return true;}}
namespace Area52Build{
constexpr char StateKey[]="state",Setting[]="core.area52.active_build";
struct State{Entry Plan;unsigned Revision=0;bool Draft=false,InternalSelection=false,Busy=false,Automatic=false,Pending=false;};
bool Applies(Player* p){return p&&p->applies;}bool Safe(Player* p){return p->safe;}
void SendState(Player* p,State const&){++p->updates;}
'''
for signature in ('bool ParseId(', 'void Store(', 'bool IsFeaturedDraft(', 'bool BeforeManualDraftChange('):
 lifecycle+=function(active,signature)+'\n'
lifecycle+='}\n'+r'''
int main(){using namespace Area52Build;Player p;State s;p.CustomData.state=&s;
 s.Draft=true;s.Plan.Id="123";s.Revision=2;s.Automatic=true;s.Pending=true;
 p.UpdatePlayerSetting("core.area52.spec.0.active_build",0,123);p.UpdatePlayerSetting("core.area52.spec.0.active_build",4,1);
 p.UpdatePlayerSetting("core.area52.spec.1.active_build",0,456);p.UpdatePlayerSetting("core.area52.spec.1.active_build",4,0);
 p.UpdatePlayerSetting("core.area52.draft_progress",1,1);
 assert(IsFeaturedDraft(&p));s.Busy=true;assert(!BeforeManualDraftChange(&p)&&s.Draft);s.Busy=false;
 p.safe=false;assert(!BeforeManualDraftChange(&p)&&s.Draft);p.safe=true;
 p.clear=false;assert(!BeforeManualDraftChange(&p)&&s.Draft);p.clear=true;
 s.InternalSelection=true;assert(BeforeManualDraftChange(&p)&&s.Draft&&p.clears==0);s.InternalSelection=false;
 assert(BeforeManualDraftChange(&p));assert(!s.Draft&&!s.Automatic&&!s.Pending&&s.Plan.Id.empty()&&s.Revision==0);
 assert(p.GetPlayerSetting("core.area52.draft_progress",0).value==1);
 assert(p.GetPlayerSetting("core.area52.draft_progress",1).value==1);
 assert(p.GetPlayerSetting("core.area52.draft_progress",2).value==1);
 assert(p.GetPlayerSetting("core.area52.spec.0.active_build",0).value==0);
 assert(p.GetPlayerSetting("core.area52.spec.1.active_build",0).value==456);
 assert(p.GetPlayerSetting(Setting,0).value==0&&p.GetPlayerSetting(Setting,4).value==0);
 assert(p.saves==1&&p.clears==1&&p.updates==1);assert(BeforeManualDraftChange(&p)&&p.saves==1);
 s.Draft=true;p.applies=false;assert(BeforeManualDraftChange(&p)&&s.Draft&&p.clears==1);
}
'''
(T/'lifecycle-adapter.cpp').write_text(lifecycle)
args[-1]='clang++ -std=c++20 -I/results/source/src/server/coa -I/workspace/src/server/coa /results/lifecycle-adapter.cpp -o /results/lifecycle-adapter && /results/lifecycle-adapter'
subprocess.run(args,check=True)
for signature in ('bool SwitchSpecialization(',):
 path=function((T/'source/src/server/coa/AscensionFreePick.cpp').read_text(),signature)
 assert path.index('BeforeManualDraftChange')<path.index('StoreSlot(player, previous)')
assert 'BeforeManualDraftChange' in (T/'source/src/server/coa/Area52PrestigeReset.inc').read_text()
(T/'runtime-checks.json').write_text(json.dumps({'passed':True,'checks':['collection not ready','temporary ownership never permanently learns','unmet investment equips dormant','investment learned and removed toggles aura without re-equipping','all 17 slots at level1','invalid capacity/catalog/stack rejection','equipment failure restores entitlement','busy cleanup rejected','exit removes all equipment and preserves known collection','actual manual-exit persistence sets both forfeiture flags','clears stale Draft specializations and preserves ordinary ones','internal auto-learning bypasses manual forfeiture','busy/unsafe/equipment failure leaves Draft unchanged','other-realm isolation'],'scope':'Extracted production entitlement, slot-validation, aura synchronization portion and manual-exit functions; bounded Player/equipment adapters. Spell-grant loop and full session dispatch are source-traced, not executed here.'},indent=2))
print('Draft entitlement and effect lifecycle offline checks passed')
