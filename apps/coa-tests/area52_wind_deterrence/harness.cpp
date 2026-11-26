#include <cassert>
#include <cstdint>
using AuraEffectHandleModes=int;struct AuraEffect{};
constexpr int MECHANIC_STUN=12,AURA_REMOVE_BY_ENEMY_SPELL=4;
struct Unit{bool area=true;std::uint64_t auras=(1ULL<<12)|(1ULL<<7);int removes=0;Unit*ToPlayer(){return this;}void RemoveAurasWithMechanic(std::uint64_t mask,int reason){assert(mask==(1ULL<<12)&&reason==4);auras&=~mask;++removes;}};
namespace AscensionFreePick{bool Applies(Unit*u){return u&&u->area;}}
struct Probe{Unit*unit;Unit*GetTarget(){return unit;}@APPLY@};
int main(){Unit unit;Probe probe{&unit};probe.HandleApply(nullptr,0);assert(unit.auras==(1ULL<<7)&&unit.removes==1);unit.area=false;unit.auras|=1ULL<<12;probe.HandleApply(nullptr,0);assert(unit.auras==((1ULL<<12)|(1ULL<<7))&&unit.removes==1);probe.unit=nullptr;probe.HandleApply(nullptr,0);}
