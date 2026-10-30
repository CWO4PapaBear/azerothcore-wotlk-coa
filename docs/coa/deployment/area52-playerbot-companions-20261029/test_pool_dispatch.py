from pathlib import Path
import subprocess,tempfile
root=Path.cwd();module=root/'modules/mod-playerbots/src/Ai/Coa'
production=(module/'CoaDungeonPool.cpp').read_text()
production=production[production.index('namespace CoaDungeonPool'):]
fixture=r'''
#include "Area52DungeonPool.h"
#include "Area52FixedPilot.h"
#include "Area52FixedEquipment.h"
#include <cassert>
#include <string>
using uint32=unsigned;using uint8=unsigned char;using uint16=unsigned short;
constexpr unsigned CONFIG_MAX_PLAYER_LEVEL=0,EQUIPMENT_SLOT_START=0,EQUIPMENT_SLOT_END=2,INVENTORY_SLOT_BAG_0=0,EQUIPMENT_SLOT_MAINHAND=0,EQUIP_ERR_OK=0;
struct ObjectGuid {std::uint64_t id=0;explicit ObjectGuid(std::uint64_t v=0):id(v){} auto operator<=>(ObjectGuid const&)const=default;bool IsEmpty()const{return !id;}auto GetRawValue()const{return id;}};
struct Map {bool instance=false;bool Instanceable(){return instance;}};
struct Item {int data=0;unsigned GetBagSlot(){return 0;}unsigned GetSlot(){return 0;}unsigned GetEntry(){return 1;}int* GetTemplate(){return &data;}}; struct ObjectMgr {bool GetItemTemplate(unsigned){return true;}} objectMgr;auto* sObjectMgr=&objectMgr;
struct Bag{unsigned GetBagSize(){return 0;}Item* GetItemByPos(unsigned){return nullptr;}};
constexpr unsigned INVENTORY_SLOT_ITEM_START=23,INVENTORY_SLOT_ITEM_END=39,INVENTORY_SLOT_BAG_START=19,INVENTORY_SLOT_BAG_END=23;
struct Player;struct Group;
struct PlayerbotAI {Player* master=nullptr;Player* GetMaster(){return master;}};
struct GroupReference {Player* player=nullptr;GroupReference* following=nullptr;Player* GetSource(){return player;}auto* next(){return following;}};
struct Group {ObjectGuid id{1000},leader{1};bool raid=false,full=false;GroupReference* first=nullptr;ObjectGuid GetGUID(){return id;}ObjectGuid GetLeaderGUID(){return leader;}bool isRaidGroup(){return raid;}bool IsFull(){return full;}auto* GetFirstMember(){return first;}};
struct Setting{unsigned value;};
struct Player {
ObjectGuid id;unsigned level=70,faction=0;bool random=false,world=true,alive=true,combat=false,teleport=false,bg=false,bgqueue=false,mounted=false,usable=true;
unsigned changes=0,gear=0,saves=0;Group* group=nullptr;Group* invite=nullptr;bool duel=false,flight=false,hero=true,offered=false;PlayerbotAI ai;Map map;Item item;std::vector<Setting> settings{{1}};
Bag* GetBagByPos(unsigned){return nullptr;}unsigned CanEquipItem(unsigned s,uint16& d,Item*,bool){d=s;return 0;}void RemoveItem(unsigned,unsigned,bool){}Item* EquipItem(unsigned,Item* i,bool){return i;}unsigned CanEquipNewItem(unsigned,unsigned short& d,unsigned,bool){d=0;return 0;}void DestroyItem(unsigned,unsigned,bool){}Item* EquipNewItem(unsigned,unsigned,bool){return &item;}auto* GetGroupInvite(){return invite;}bool IsInFlight(){return flight;}
auto const* FindPlayerSettings(char const*)const{return &settings;}
bool IsInWorld(){return world;}bool IsAlive(){return alive;}bool IsInCombat(){return combat;}auto* GetGroup(){return group;}
bool IsBeingTeleported(){return teleport;}bool InBattleground(){return bg;}bool InBattlegroundQueue(){return bgqueue;}
auto* FindMap(){return &map;}ObjectGuid GetGUID(){return id;}unsigned GetTeamId(){return faction;}unsigned GetLevel(){return level;}
bool IsMounted(){return mounted;}void Dismount(){mounted=false;}void GiveLevel(unsigned l){level=l;++changes;}
void InitStatsForLevel(bool){}void SaveToDB(bool,bool){++saves;}unsigned CanUseItem(int*){return usable?0:1;}Item* GetItemByPos(unsigned,unsigned){return &item;}
};
namespace ObjectAccessor {std::map<ObjectGuid,Player*> players;Player* FindConnectedPlayer(ObjectGuid id){auto i=players.find(id);return i==players.end()?nullptr:i->second;}}
PlayerbotAI* GET_PLAYERBOT_AI(Player* p){return p&&p->random?&p->ai:nullptr;}
bool IsCoaHero(Player const* p){return p&&p->hero;}
bool CoaLfgOfferedToOther(Player* p,Player*){return p->offered;}
bool ApplyArea52PilotVanity(Player*){return true;}
namespace CoaDungeonPool{bool Busy(ObjectGuid);bool IsPilot(Player*);bool EnsureEquipment(Player*);}
struct Config{bool enabled=true,area52DungeonPool=true,area52FixedPilot=true;}sPlayerbotAIConfig;
struct RandomMgr{std::vector<Player*> humans;std::map<ObjectGuid,Player*> bots;bool IsRandomBot(Player* p){return p&&p->random;}auto const& GetPlayers(){return humans;}auto GetPlayerBotsBegin(){return bots.begin();}auto GetPlayerBotsEnd(){return bots.end();}}sRandomPlayerbotMgr;
namespace lfg {constexpr unsigned LFG_STATE_NONE=0,LFG_STATE_QUEUED=1,LFG_STATE_PROPOSAL=2;}
struct Lfg {
std::map<ObjectGuid,unsigned> states,roles;std::map<ObjectGuid,std::set<unsigned>> dungeons;unsigned joins=0,leaves=0;
unsigned GetState(ObjectGuid g){return states[g];}unsigned GetRoles(ObjectGuid g){return roles[g];}auto GetSelectedDungeons(ObjectGuid g){return dungeons[g];}
void JoinLfg(Player* p,unsigned role,std::set<unsigned>& ds,char const*){assert(p->random&&role==(1u<<p->settings[0].value)&&!ds.empty());states[p->id]=1;++joins;}
void LeaveLfg(ObjectGuid id){states[id]=0;++leaves;}
}lfgFixture;auto* sLFGMgr=&lfgFixture;
struct World {unsigned getIntConfig(unsigned){return 80;}}world;auto* sWorld=&world;
int build;bool apply=true,ready=true;
int const* FindCoaLegendaryClass(char const* spec){return *spec?&build:nullptr;}
bool ApplyCoaLegendaryClass(Player*,int,std::string&){return apply;}void EnsureCoaLegendaryClass(Player*){}bool CoaLegendaryClassReady(Player*){return ready;}
struct PlayerbotFactory{Player* p;PlayerbotFactory(Player* b,unsigned):p(b){}void InitSkills(){}void InitAvailableSpells(){}static void DestroyEquippedGear(Player* p){++p->gear;}void SetDeterministic(bool){}void InitEquipment(bool,bool){}void InitAmmo(){}void InitPet(){}};
'''
checks=r'''
int main(){
Player leader;leader.id=ObjectGuid(1);leader.level=20;
sRandomPlayerbotMgr.humans={&leader};ObjectAccessor::players[leader.id]=&leader;
lfgFixture.states[leader.id]=1;lfgFixture.roles[leader.id]=8;lfgFixture.dungeons[leader.id]={5};
std::array<Player,4> bots;
for(unsigned i=0;i<4;++i){auto& b=bots[i];b.id=ObjectGuid(10+i);b.random=true;b.settings[0].value=i<2?i+1:3;sRandomPlayerbotMgr.bots[b.id]=&b;ObjectAccessor::players[b.id]=&b;}
CoaDungeonPool::Update(999);assert(lfgFixture.joins==0);
for(unsigned n=0;n<9;++n)CoaDungeonPool::Update(1000);
assert(lfgFixture.joins==4&&leader.level==20&&leader.changes==0);
for(auto& b:bots){assert(b.level==20&&b.changes==1&&CoaDungeonPool::Busy(b.id));}
assert(CoaDungeonPool::CanMatch({leader.id,bots[0].id,bots[1].id,bots[2].id,bots[3].id}));
assert(!CoaDungeonPool::CanMatch({bots[0].id,bots[1].id,bots[2].id,bots[3].id}));
leader.level=21;assert(!CoaDungeonPool::CanMatch({leader.id,bots[0].id}));
CoaDungeonPool::Update(1000);assert(lfgFixture.leaves==4);
for(unsigned n=0;n<9;++n)CoaDungeonPool::Update(1000);
for(auto& b:bots)assert(b.level==21&&b.changes==2);
lfgFixture.states[leader.id]=0;CoaDungeonPool::Update(1000);
for(auto& b:bots)assert(!CoaDungeonPool::Busy(b.id)&&lfgFixture.states[b.id]==0);
Player healer;healer.id=ObjectGuid(2);healer.level=40;ObjectAccessor::players[healer.id]=&healer;
GroupReference second{&leader,nullptr},first{&healer,&second};Group group;group.leader=healer.id;group.first=&first;
leader.group=&group;healer.group=&group;sRandomPlayerbotMgr.humans={&leader,&healer};
lfgFixture.states[group.id]=1;lfgFixture.roles[healer.id]=4;lfgFixture.dungeons[healer.id]={6};
for(unsigned n=0;n<9;++n)CoaDungeonPool::Update(1000);
assert(bots[0].level==40&&bots[2].level==40&&bots[3].level==40&&bots[1].level==21);
assert(CoaDungeonPool::CanMatch({group.id,bots[0].id,bots[2].id,bots[3].id}));
for(unsigned i:{0,2,3})bots[i].group=&group;
lfgFixture.states[group.id]=3;unsigned leaves=lfgFixture.leaves;
CoaDungeonPool::Update(1000);assert(lfgFixture.leaves==leaves);
for(auto& b:bots)assert(!CoaDungeonPool::Busy(b.id));
healer.level=50;for(unsigned n=0;n<9;++n)CoaDungeonPool::Update(1000);assert(bots[0].level==40);
leader.group=nullptr;healer.group=nullptr;leader.level=35;lfgFixture.states[leader.id]=0;
for(auto& b:bots){b.group=nullptr;lfgFixture.states[b.id]=0;}
auto& recruit=bots[0];
assert(CoaDungeonPool::RecruitAvailable(&recruit,&leader,1));
assert(!CoaDungeonPool::PrepareRecruit(nullptr,&leader));
assert(!CoaDungeonPool::PrepareRecruit(&recruit,&leader));
assert(recruit.level==35&&CoaDungeonPool::PrepareRecruit(&recruit,&leader));
assert(!CoaDungeonPool::RecruitAvailable(&recruit,&leader,2));
recruit.duel=true;assert(!CoaDungeonPool::RecruitAvailable(&recruit,&leader,1));recruit.duel=false;
recruit.invite=&group;assert(!CoaDungeonPool::RecruitAvailable(&recruit,&leader,1));recruit.invite=nullptr;
recruit.ai.master=&leader;assert(!CoaDungeonPool::RecruitAvailable(&recruit,&leader,1));recruit.ai.master=nullptr;
recruit.map.instance=true;assert(!CoaDungeonPool::RecruitAvailable(&recruit,&leader,1));recruit.map.instance=false;
recruit.hero=false;assert(!CoaDungeonPool::IsPilot(&recruit));recruit.hero=true;
recruit.faction=1;assert(!CoaDungeonPool::RecruitAvailable(&recruit,&leader,1));recruit.faction=0;
for(unsigned state:{1,2,3}){lfgFixture.states[recruit.id]=state;assert(!CoaDungeonPool::RecruitAvailable(&recruit,&leader,1));}
lfgFixture.states[recruit.id]=0;leader.group=&group;group.leader=healer.id;
assert(!CoaDungeonPool::RecruiterAvailable(&leader));leader.group=nullptr;
leader.level=14;assert(!CoaDungeonPool::RecruiterAvailable(&leader));leader.level=80;
assert(CoaDungeonPool::RecruiterAvailable(&leader));leader.level=81;assert(!CoaDungeonPool::RecruiterAvailable(&leader));leader.level=35;
sRandomPlayerbotMgr.humans={&leader};lfgFixture.states[leader.id]=1;
recruit.offered=true;CoaDungeonPool::Update(1000);assert(!CoaDungeonPool::Busy(recruit.id));
recruit.offered=false;CoaDungeonPool::Update(1000);assert(CoaDungeonPool::Busy(recruit.id));
assert(!CoaDungeonPool::RecruitAvailable(&recruit,&healer,1));
}
'''
with tempfile.TemporaryDirectory() as temp:
    out=Path(temp);(out/'dispatch.cpp').write_text(fixture+production+checks)
    subprocess.run(['clang++','-std=c++20','-Wall','-Wextra','-Werror','-I'+str(module),str(out/'dispatch.cpp'),'-o',str(out/'dispatch')],check=True)
    subprocess.run([str(out/'dispatch')],check=True)
print('PASS: entire production pool adapter dispatch, actual leader selection, bounded preparation, reservation-specific match, level-change invalidation, cancel/reuse and grouped-bot preservation. Native external APIs are bounded fixtures; no gameplay or real sessions.')
