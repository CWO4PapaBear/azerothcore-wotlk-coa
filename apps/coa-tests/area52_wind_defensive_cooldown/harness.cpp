#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <initializer_list>
#include "Area52DefensiveCooldowns.h"
using uint32=unsigned;using int32=int;
constexpr int SPELL_CATEGORY_SHARED_POTION_COOLDOWN=4,SPELL_ATTR0_CU_FORCE_SEND_CATEGORY_COOLDOWNS=1,SPELL_COOLDOWN_FLAG_NONE=0;
struct SpellInfo{uint32 Id,SpellFamilyName,category;int RecoveryTime,CategoryRecoveryTime;SpellInfo const*GetFirstRankSpell()const{return this;}bool IsCooldownStartedOnEvent()const{return false;}bool HasAttribute(int)const{return false;}};
struct Manager{std::map<unsigned,SpellInfo>rows;SpellInfo const*GetSpellInfo(unsigned id){auto i=rows.find(id);return i==rows.end()?nullptr:&i->second;}}manager;Manager*sSpellMgr=&manager;
using SpellCategorySet=std::set<std::pair<bool,unsigned>>;using SpellCategoryStore=std::map<unsigned,SpellCategorySet>;SpellCategoryStore sSpellsByCategoryStore;using PacketCooldowns=std::map<unsigned,unsigned>;struct WorldPacket{};
struct Player;namespace AscensionFreePick{bool Applies(Player*);}
struct Player{bool area=true;std::map<unsigned,unsigned>cooldowns;void _AddSpellCooldown(unsigned id,unsigned,unsigned,int time,bool=false,bool=false){cooldowns[id]=std::max(cooldowns[id],unsigned(time));}template<class T>void BuildCooldownPacket(WorldPacket&,int,T const&){}void BuildCooldownPacket(WorldPacket&,int,unsigned,int){}void SendDirectMessage(WorldPacket*){}
void cast(SpellInfo const*spellInfo){uint32 itemId=0,cat=spellInfo->category;int rec=spellInfo->RecoveryTime,catrec=spellInfo->CategoryRecoveryTime;bool needsCooldownPacket=false;@POLICY@ int recTime=rec,catrecTime=catrec;@DISPATCH@}
};namespace AscensionFreePick{bool Applies(Player*p){return p->area;}}
int main(){for(SpellInfo row:std::initializer_list<SpellInfo>{@RECORDS@}){manager.rows.emplace(row.Id,row);sSpellsByCategoryStore[row.category].insert({false,row.Id});}
for(unsigned id:{19263u,31224u,642u,1022u,45438u}){Player p;p.cast(manager.GetSpellInfo(id));for(unsigned other:{19263u,31224u,642u,1022u,45438u})assert(p.cooldowns.at(other)==(other==id?unsigned(manager.rows.at(id).RecoveryTime):30000u));assert(!p.cooldowns.contains(645438));}
Player other;other.area=false;other.cast(manager.GetSpellInfo(19263));assert(other.cooldowns.size()==1&&other.cooldowns.at(19263)==120000);Player longCD;longCD.cooldowns[642]=100000;longCD.cast(manager.GetSpellInfo(19263));assert(longCD.cooldowns[642]==100000);}
