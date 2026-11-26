#include <cassert>
#include <array>
#include <algorithm>
using uint32=unsigned;
constexpr uint32 SPELL_SHAMAN_STORMSTRIKE=17364,SPELL_SHAMAN_LAVA_LASH=60103,SPELLFAMILY_SHAMAN=11;
struct SpellInfo{uint32 Id=0,SpellFamilyName=11;uint32 SpellFamilyFlags[3]={};};
struct Player{bool area=true;Player*ToPlayer(){return this;}bool MatchesSpellInteraction(uint32 id,uint32 original){return id==original||(original==17364&&id==978945);}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct ProcEventInfo{SpellInfo*info=nullptr;SpellInfo*GetSpellInfo(){return info;}};
struct Probe{Player*player;Player*GetTarget(){return player;}@CHECK@};
int main(){Player player;Probe probe{&player};ProcEventInfo event;assert(probe.CheckProc(event));SpellInfo info;event.info=&info;for(auto id:{17364u,60103u,978945u}){info.Id=id;assert(!probe.CheckProc(event));}info.Id=25504;info.SpellFamilyFlags[0]=0x00800000;assert(!probe.CheckProc(event));info.SpellFamilyFlags[0]=0;info.Id=403;assert(probe.CheckProc(event));player.area=false;info.Id=978945;assert(probe.CheckProc(event));}
