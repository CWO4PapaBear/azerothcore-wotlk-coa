#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <map>
using uint32=uint32_t;
constexpr int CHAT_MSG_WHISPER=1,LANG_ADDON=2,EFFECT_0=0;
struct Area52RangeView{uint32 Cooldown=0;};
struct SpellModifier{};
struct AuraEffect{SpellModifier mod;SpellModifier*GetSpellModifier()const{return const_cast<SpellModifier*>(&mod);}};
struct Player;
struct SpellInfo{bool eligible=true;mutable int calls=0;float GetMaxRange(bool,Player*)const{++calls;return 36;}float GetMinRange(bool)const{return 0;}};
struct Store{SpellInfo info;SpellInfo const*GetSpellInfo(uint32 id){return id==2912?&info:nullptr;}}store;Store*sSpellMgr=&store;
struct WorldPacket{std::string text;};
struct Session{bool bot=false;bool IsHeadlessBot(){return bot;}};
struct Custom{Area52RangeView view;template<class T>T*GetDefault(char const*){return &view;}};
struct Player{bool area=true,world=true;Custom CustomData;Session session;std::map<uint32,AuraEffect>auras;std::string sent;bool IsInWorld(){return world;}Session*GetSession(){return &session;}AuraEffect const*GetAuraEffect(uint32 id,int){auto it=auras.find(id);return it==auras.end()?nullptr:&it->second;}bool IsAffectedBySpellmod(SpellInfo const*i,SpellModifier*,void*){return i->eligible;}void SendDirectMessage(WorldPacket*p){sent=p->text;}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
namespace Acore{std::string StringFormat(char const*,uint32 n,uint32 id,float min,float max){std::ostringstream s;s<<n<<' '<<id<<' '<<min<<' '<<max;return s.str();}}
struct ChatHandler{static void BuildChatPacket(WorldPacket&p,int,int,Player*,Player*,std::string text){p.text=text;}};
struct Handler{@UPDATE@
@REQUEST@
};
int main(){Handler h;Player player,other;player.auras[16819]={};std::string query="A52RANGE\t1 2912";assert(!h.OnPlayerCanUseChat(&player,1,2,query,&player));assert(player.sent=="A52RANGE\t1 2912 0 36"&&store.info.calls==1);player.sent.clear();assert(!h.OnPlayerCanUseChat(&player,1,2,query,&player));assert(player.sent.empty());h.OnPlayerUpdate(&player,100);assert(h.OnPlayerCanUseChat(&player,1,2,query,&other));assert(h.OnPlayerCanUseChat(&player,1,0,query,&player));assert(player.sent.empty());player.area=false;assert(!h.OnPlayerCanUseChat(&player,1,2,query,&player));assert(player.sent.empty());player.area=true;player.auras.clear();h.OnPlayerUpdate(&player,100);h.OnPlayerCanUseChat(&player,1,2,query,&player);assert(player.sent=="A52RANGE\t1 2912 0 0"&&store.info.calls==1);player.auras[816819]={};store.info.eligible=false;h.OnPlayerUpdate(&player,100);h.OnPlayerCanUseChat(&player,1,2,query,&player);assert(player.sent=="A52RANGE\t1 2912 0 0"&&store.info.calls==1);for(auto bad:{"A52RANGE\t1 2912 trailing","A52RANGE\tx 2912","A52RANGE\t1 999999"}){h.OnPlayerUpdate(&player,100);player.sent.clear();std::string q=bad;h.OnPlayerCanUseChat(&player,1,2,q,&player);assert(player.sent.empty());}}
