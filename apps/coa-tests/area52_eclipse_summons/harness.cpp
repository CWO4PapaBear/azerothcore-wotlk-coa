#include <cassert>
#include <chrono>
#include <cstdint>
#include <vector>
using uint32=uint32_t;using namespace std::chrono_literals;
constexpr int TOTEM_PASSIVE=0,UNIT_CREATED_BY_SPELL=0,SENTRY_TOTEM_ENTRY=3968,REACT_AGGRESSIVE=2,SUMMON_TYPE_LIGHTWELL=5;
enum class TotemSpellIds{FireTotemSpell=3599};enum class VisibilityDistanceType{Infinite};
struct Motion{void MoveFall(){}};
struct Unit{bool alive=true;bool IsAlive(){return alive;}template<class T>void CastSpell(T*,int,bool){}};
struct Creature:Unit{int updates=0;void Update(uint32){++updates;}};
struct Minion:Creature{int inits=0;void InitSummon(){++inits;}};
struct Events{int queued=0;template<class F,class D>void AddEventAtOffset(F,D){++queued;}};
struct Props{int Type=4;};
struct Totem:Minion{int m_type=0,created=30706,entry=17539,despawns=0;uint32 m_duration=300000;uint32 spells[2]={57658,30708};Unit*owner=nullptr;Props props;Props*m_Properties=&props;Events m_Events;Motion motion;std::vector<uint32> casts;Unit*GetOwner(){return owner;}uint32 GetSpell(int i=0){return spells[i];}uint32 GetUInt32Value(int){return created;}uint32 GetEntry(){return entry;}void SetReactState(int){}void setActive(bool){}void SetVisibilityDistanceOverride(VisibilityDistanceType){}bool IsInWater(){return false;}Motion*GetMotionMaster(){return &motion;}void CastSpell(Totem*,uint32 id,bool){casts.push_back(id);}void UnSummon(){++despawns;}void InitSummon();void Update(uint32);};
@INIT@
@UPDATE@
int main(){Unit owner;for(uint32 buff:{57658u,57660u,57662u,57663u}){Totem t;t.owner=&owner;t.spells[0]=buff;t.InitSummon();assert(t.inits==1&&t.casts==std::vector<uint32>({buff,30708})&&t.m_Events.queued==0);t.Update(299999);assert(t.despawns==0&&t.m_duration==1);t.Update(1);assert(t.despawns==1);}Totem absent;absent.Update(1);assert(absent.despawns==1);Totem dead;dead.owner=&owner;owner.alive=false;dead.Update(1);assert(dead.despawns==1);owner.alive=true;Totem killed;killed.owner=&owner;killed.alive=false;killed.Update(1);assert(killed.despawns==1);}
