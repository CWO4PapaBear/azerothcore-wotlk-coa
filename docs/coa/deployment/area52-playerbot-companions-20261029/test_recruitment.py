from pathlib import Path
import ast
import subprocess
import tempfile

root = Path.cwd()
module = root/'modules/mod-playerbots/src'
review = Path('/results')
base = ast.parse((review/'test_pool_dispatch.py').read_text())
fixture = next(ast.literal_eval(node.value) for node in base.body if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'fixture' for t in node.targets))
fixture = fixture.replace('bool CoaLfgOfferedToOther(Player* p,Player*){return p->offered;}', 'bool CoaLfgOfferedToOther(Player*,Player*);')
fixture = fixture.replace('auto const& GetPlayers()', 'auto const& GetAllBots(){return bots;}auto const& GetPlayers()')
fixture = fixture.replace('unsigned changes=0,gear=0,saves=0;', 'unsigned whispers=0;std::string GetName(){return std::to_string(id.id);}void Whisper(std::string const&,unsigned,Player*){++whispers;}unsigned changes=0,gear=0,saves=0;')
source = (module/'Ai/Class/Coa/CoaSpecialization.cpp').read_text()

def block(text, marker):
    start = text.index(marker)
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]

extras = r'''
#include <mutex>
#include <ctime>
enum class CoaRole {Dps,Tank,Heal};
constexpr unsigned CoaHeroClass=10,IN_MILLISECONDS=1000,LANG_UNIVERSAL=0;
struct CoaLfgOffer {ObjectGuid player;time_t until=0;bool ready=true;unsigned level=0;CoaRole role=CoaRole::Dps;};
std::mutex LfgLock;
std::map<ObjectGuid,CoaLfgOffer> LfgOffers;
std::string OfferText(Player*,CoaRole){return "ready";}
unsigned sincePrepare=0;
namespace RandomBotLevelMgr{bool held=false;bool IsHeld(ObjectGuid){return held;}}
'''
pool = (module/'Ai/Coa/CoaDungeonPool.cpp').read_text().split('namespace CoaDungeonPool',1)[1]
find = source[source.index('Player* FindCoaRecruit'):source.index('    // A free random bot')]
find += 'return nullptr;\n}\n'
queue_start = source.index('bool QueuePilotRecruit(Player* master', source.index('struct CoaLfgOffer'))
queue = block(source[queue_start:], 'bool QueuePilotRecruit')
tick_start = source.index('    void OnUpdate(uint32 diff) override', source.index('class CoaLfgWorldScript'))
tick = block(source[tick_start:], 'if (sPlayerbotAIConfig.area52FixedPilot)')
functions = block(source, 'unsigned PilotRole(') + find + queue
functions += block(source, 'bool CoaLfgOfferedToOther(') + block(source, 'bool CoaLfgTakeOffer(')
functions += 'void Tick(unsigned diff){' + tick + '}\n'
checks = r'''
int main(){
Player human,other,bot;human.id=ObjectGuid(1);other.id=ObjectGuid(2);bot.id=ObjectGuid(10);human.level=30;other.level=30;bot.random=true;
ObjectAccessor::players={{human.id,&human},{other.id,&other},{bot.id,&bot}};sRandomPlayerbotMgr.bots={{bot.id,&bot}};
std::string message;
RandomBotLevelMgr::held=true;assert(!QueuePilotRecruit(&human,CoaRole::Tank,message));RandomBotLevelMgr::held=false;
assert(QueuePilotRecruit(&human,CoaRole::Tank,message));
assert(LfgOffers.size()==1&&!LfgOffers[bot.id].ready);
assert(QueuePilotRecruit(&human,CoaRole::Tank,message)&&LfgOffers.size()==1);
assert(!QueuePilotRecruit(&other,CoaRole::Tank,message));
assert(!CoaLfgTakeOffer(&bot,&human));
Tick(1000);assert(bot.level==30&&bot.whispers==0&&bot.changes==1);
Tick(1000);assert(bot.whispers==1&&LfgOffers[bot.id].ready);
Tick(1000);assert(bot.whispers==1&&bot.changes==1);
assert(!CoaLfgTakeOffer(&bot,&other));
human.level=31;assert(!CoaLfgTakeOffer(&bot,&human));human.level=30;
Group group;bot.invite=&group;assert(CoaLfgTakeOffer(&bot,&human)&&LfgOffers.empty());bot.invite=nullptr;
bot.group=&group;assert(!QueuePilotRecruit(&other,CoaRole::Tank,message));bot.group=nullptr;
bot.ai.master=&human;assert(!QueuePilotRecruit(&other,CoaRole::Tank,message));bot.ai.master=nullptr;
assert(QueuePilotRecruit(&other,CoaRole::Tank,message));LfgOffers[bot.id].until=time(nullptr)-1;Tick(1000);assert(LfgOffers.empty());
assert(QueuePilotRecruit(&other,CoaRole::Tank,message));other.world=false;Tick(1000);assert(LfgOffers.empty());other.world=true;
assert(QueuePilotRecruit(&other,CoaRole::Tank,message));other.level=40;Tick(1000);assert(LfgOffers.empty()&&bot.level==30);
lfgFixture.states[bot.id]=lfg::LFG_STATE_QUEUED;assert(!QueuePilotRecruit(&human,CoaRole::Tank,message));lfgFixture.states[bot.id]=0;
sPlayerbotAIConfig.enabled=false;assert(!QueuePilotRecruit(&human,CoaRole::Tank,message));
}
'''
with tempfile.TemporaryDirectory() as temporary:
    out = Path(temporary)
    (out/'test.cpp').write_text(fixture+extras+'namespace CoaDungeonPool'+pool+functions+checks)
    subprocess.run(['clang++','-std=c++20','-Wall','-Wextra','-Werror','-I'+str(module/'Ai/Coa'),str(out/'test.cpp'),'-o',str(out/'test')],check=True)
    subprocess.run([str(out/'test')],check=True)
print('PASS: production Hero recruitment branch, offer reservation/preparation/take/expiry and world update; bounded native external adapters, no gameplay.')

duel_source = (module/'Ai/Base/Actions/AcceptDuelAction.cpp').read_text()
duel = block(duel_source,'bool AcceptDuelAction::Execute(')
duel_fixture = r'''
#include <cassert>
#include <vector>
#include <map>
using uint8=unsigned char;
struct ObjectGuid {unsigned value=0;bool operator==(ObjectGuid const&)const=default;};
struct WorldPacket{unsigned opcode;std::vector<ObjectGuid> data;unsigned next=0;WorldPacket(unsigned op,unsigned=0):opcode(op){}WorldPacket& operator>>(ObjectGuid& id){id=data.at(next++);return *this;}WorldPacket& operator<<(ObjectGuid id){data.push_back(id);return *this;}};
struct Event{WorldPacket packet{0};WorldPacket& getPacket(){return packet;}};
constexpr unsigned CMSG_DUEL_CANCELLED=1,CMSG_DUEL_ACCEPTED=2;
struct Session{unsigned accept=0,decline=0;void HandleDuelCancelledOpcode(WorldPacket&){++decline;}void HandleDuelAcceptedOpcode(WorldPacket&){++accept;}};
struct Group{ObjectGuid id{100};ObjectGuid GetGUID(){return id;}};
struct Map{bool instance=false;bool Instanceable(){return instance;}};
struct Player{ObjectGuid id;bool world=true,pilot=false,busy=false;Group* group=nullptr;Map map;Session session;auto GetGUID(){return id;}bool IsInWorld(){return world;}auto* GetGroup(){return group;}auto* FindMap(){return &map;}auto* GetSession(){return &session;}};
struct AI{Player* master=nullptr;unsigned resets=0;bool HasGameClientMaster(){return master;}Player* GetMaster(){return master;}void ResetStrategies(){++resets;}};
namespace CoaDungeonPool{Player* selected=nullptr;bool IsPilot(Player* p){return p->pilot;}bool Busy(ObjectGuid){return selected->busy;}}
namespace lfg{constexpr unsigned LFG_STATE_NONE=0;}
struct Lfg{std::map<unsigned,unsigned> state;unsigned GetState(ObjectGuid g){return state[g.value];}}lfgFixture;auto* sLFGMgr=&lfgFixture;
unsigned health=100;
#define AI_VALUE2(t,a,b) health
struct AcceptDuelAction{Player* bot;AI* botAI;bool Execute(Event);};
'''
duel_checks = r'''
int main(){Player human,bot,other;Group group;human.id={1};other.id={2};bot.id={10};bot.pilot=true;human.group=bot.group=&group;AI ai;ai.master=&human;CoaDungeonPool::selected=&bot;AcceptDuelAction action{&bot,&ai};Event event;event.packet.data={{99},human.id};
assert(action.Execute(event)&&bot.session.accept==1);
event.packet.data[1]=other.id;assert(!action.Execute(event));event.packet.data[1]=human.id;
for(unsigned state:{1,2,3}){lfgFixture.state[100]=state;assert(!action.Execute(event));}lfgFixture.state[100]=0;
bot.busy=true;assert(!action.Execute(event));bot.busy=false;
bot.map.instance=true;assert(!action.Execute(event));bot.map.instance=false;
bot.group=nullptr;assert(!action.Execute(event));bot.group=&group;
ai.master=nullptr;assert(!action.Execute(event));ai.master=&human;
health=10;assert(action.Execute(event));
bot.pilot=false;ai.master=nullptr;assert(!action.Execute(event));health=100;assert(action.Execute(event));
assert(ai.resets==3);
}
'''
with tempfile.TemporaryDirectory() as temporary:
    out=Path(temporary)
    (out/'duel.cpp').write_text(duel_fixture+duel+duel_checks)
    subprocess.run(['clang++','-std=c++20','-Wall','-Wextra','-Werror',str(out/'duel.cpp'),'-o',str(out/'duel')],check=True)
    subprocess.run([str(out/'duel')],check=True)
print('PASS: actual duel packet handler, recruited owner admission, foreign challengers and dungeon commitment rejection, legacy health behavior. Combat AI and damage remain live tester acceptance.')

invite_source = (module/'Ai/Base/Actions/AcceptInvitationAction.cpp').read_text()
invite = block(invite_source,'bool AcceptInvitationAction::Execute(')
invite_fixture = r'''
#include <cassert>
#include <string>
using uint32=unsigned;
constexpr unsigned SMSG_GROUP_DECLINE=1,BOT_STATE_NON_COMBAT=0,PLAYERBOT_SECURITY_INVITE=1;
struct Event{};struct WorldPacket{WorldPacket(unsigned=0,unsigned=0){}template<class T>WorldPacket& operator<<(T const&){return *this;}};
struct Group{unsigned leader=1;bool joined=false;unsigned GetLeaderGUID(){return leader;}bool IsMember(unsigned id){return joined&&id==leader;}};
struct Player;
struct Session{Player* player=nullptr;void HandleGroupAcceptOpcode(WorldPacket&);};
struct Player{unsigned id=0,declines=0,teleports=0;bool pilot=false,afk=false,teleportOK=true;Group* group=nullptr;Group* invite=nullptr;Session session{this};auto* GetGroupInvite(){return invite;}auto* GetGroup(){return group;}unsigned GetGUID(){return id;}std::string GetName(){return "bot";}void SendDirectMessage(WorldPacket*){++declines;}void UninviteFromGroup(){invite=nullptr;}bool isAFK(){return afk;}void ToggleAFK(){afk=!afk;}auto* GetSession(){return &session;}unsigned GetMap(){return id;}unsigned GetMapId(){return 1;}float GetDistance(Player*){return 100;}float GetPositionX(){return 0;}float GetPositionY(){return 0;}float GetPositionZ(){return 0;}float GetOrientation(){return 0;}bool TeleportTo(unsigned,float,float,float,float,unsigned,void*,bool){++teleports;return teleportOK;}};
void Session::HandleGroupAcceptOpcode(WorldPacket&){player->group=player->invite;player->invite=nullptr;if(player->group)player->group->joined=true;}
namespace ObjectAccessor{Player* inviter=nullptr;Player* FindPlayer(unsigned){return inviter;}}
namespace CoaDungeonPool{bool IsPilot(Player* p){return p->pilot;}}
bool offered=false,foreign=false;
bool CoaLfgTakeOffer(Player*,Player*){return offered;}bool CoaLfgOfferedToOther(Player*,Player*){return foreign;}
bool IsSelfBot(Player*){return false;}bool IsRealPlayer(Player* p){return !p->pilot;}
void CoaLeaveBattlegroundQueues(Player*){}void CoaDropForeignInstanceBinds(Player*,Player*){}bool CoaInOtherInstance(Player*,Player*){return false;}
struct Security{bool CheckLevelFor(unsigned,bool,Player*){return true;}};
struct AI{Player* master=nullptr;unsigned leaves=0,resets=0;Security security;auto* GetSecurity(){return &security;}void SetMaster(Player* p){master=p;}void ResetStrategies(){++resets;}void Reset(){}void ChangeStrategy(char const*,unsigned){}void TellMasterGreeting(std::string const&){}void LeaveOrDisbandGroup(){++leaves;}};
struct RandomMgr{bool IsRandomBot(Player* p){return p->pilot;}}sRandomPlayerbotMgr;
struct Config{bool summonWhenGroup=false;float sightDistance=50;}sPlayerbotAIConfig;
struct PlayerbotTextMgr{static PlayerbotTextMgr& instance(){static PlayerbotTextMgr m;return m;}std::string GetBotTextOrDefault(char const*,char const* t,std::initializer_list<int>){return t;}};
void Teleport(Player*,Player*,bool){}
struct AcceptInvitationAction{Player* bot;AI* botAI;bool Execute(Event);};
'''
invite_checks = r'''
int main(){Player human,bot;Group group;human.id=1;bot.id=10;bot.pilot=true;bot.invite=&group;AI ai;ObjectAccessor::inviter=&human;AcceptInvitationAction action{&bot,&ai};
assert(!action.Execute({})&&!bot.invite&&!bot.group&&human.declines==1&&bot.teleports==0);
bot.invite=&group;offered=true;assert(action.Execute({})&&bot.group==&group&&bot.teleports==1&&ai.master==&human);
bot.group=nullptr;bot.invite=&group;bot.teleportOK=false;assert(!action.Execute({})&&ai.leaves==1);
}
'''
with tempfile.TemporaryDirectory() as temporary:
    out=Path(temporary)
    (out/'invite.cpp').write_text(invite_fixture+invite+invite_checks)
    subprocess.run(['clang++','-std=c++20','-Wall','-Wextra','-Werror',str(out/'invite.cpp'),'-o',str(out/'invite')],check=True)
    subprocess.run([str(out/'invite')],check=True)
print('PASS: actual invitation handler admits reserved pilot, rejects unreserved invitation, sets controller and summons, and requests group leave on failed teleport. Session/teleport services are bounded offline adapters.')

leave_source = (module/'Ai/Base/Actions/LeaveGroupAction.cpp').read_text()
leave = block(leave_source,'bool LeaveGroupAction::Execute(')+block(leave_source,'bool LeaveGroupAction::Leave(')
ai_source = (module/'Bot/PlayerbotAI.cpp').read_text()
release = ai_source[ai_source.index('void PlayerbotAI::UpdateAIGroupMaster()'):ai_source.index('    // Bot in BG, but master no longer')]+ '}\n'
release_fixture = r'''
#include <cassert>
#include <string>
constexpr unsigned PLAYERBOT_SECURITY_TALK=0;
struct Group{};struct PlayerbotAI;
struct Player{Group* group=nullptr;bool random=true;PlayerbotAI* ai=nullptr;auto* GetGroup(){return group;}};
struct Event{Player* owner;Player* getOwner(){return owner;}};
struct PlayerbotAI{Player* bot=nullptr;Player* master=nullptr;unsigned leaves=0,resets=0,strategies=0;auto* GetMaster(){return master;}void SetMaster(Player* p){master=p;}void Reset(bool){++resets;}void ResetStrategies(){++strategies;}void TellMasterGreeting(std::string,unsigned){}void LeaveOrDisbandGroup(){++leaves;}void UpdateAIGroupMaster();};
PlayerbotAI* GET_PLAYERBOT_AI(Player* p){return p->ai;}
struct RandomMgr{bool IsRandomBot(Player* p){return p->random;}}sRandomPlayerbotMgr;
struct PlayerbotTextMgr{static PlayerbotTextMgr& instance(){static PlayerbotTextMgr m;return m;}std::string GetBotTextOrDefault(char const*,char const* t,std::initializer_list<int>){return t;}};
struct LeaveGroupAction{PlayerbotAI* botAI;bool Execute(Event);bool Leave();};
'''
release_checks = r'''
int main(){Player bot,owner,stranger;Group group;PlayerbotAI ai;bot.ai=&ai;bot.group=&group;ai.bot=&bot;ai.master=&owner;LeaveGroupAction action{&ai};
assert(!action.Execute({&stranger})&&ai.leaves==0);assert(action.Execute({&owner})&&ai.leaves==1);
ai.UpdateAIGroupMaster();assert(ai.master==&owner);bot.group=nullptr;ai.UpdateAIGroupMaster();assert(!ai.master&&ai.resets==1&&ai.strategies==1);
ai.UpdateAIGroupMaster();assert(ai.resets==1);bot.random=false;ai.master=&owner;ai.UpdateAIGroupMaster();assert(ai.master==&owner);
}
'''
with tempfile.TemporaryDirectory() as temporary:
    out=Path(temporary)
    (out/'leave.cpp').write_text(release_fixture+leave+release+release_checks)
    subprocess.run(['clang++','-std=c++20','-Wall','-Wextra','-Werror',str(out/'leave.cpp'),'-o',str(out/'leave')],check=True)
    subprocess.run([str(out/'leave')],check=True)
print('PASS: existing leave-command owner authorization and actual ungrouped-random-bot controller cleanup branch; repeated updates and nonrandom isolation. Session group removal is an explicit external boundary.')
