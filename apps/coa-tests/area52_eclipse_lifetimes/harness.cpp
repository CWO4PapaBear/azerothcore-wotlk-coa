#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <vector>
using uint32=uint32_t;using int32=int32_t;using Milliseconds=std::chrono::milliseconds;using TimePoint=std::chrono::steady_clock::time_point;using AuraEffectHandleModes=int;using namespace std::chrono_literals;
constexpr int UNIT_AURA_TYPE=1,CHEAT_COOLDOWN=1,SPELLMOD_DURATION=1,AURA_REMOVE_BY_EXPIRE=1,POWER_MANA=0,VengefulSoulRestore=816914,UNIT_STATE_CASTING=1,MECHANIC_POLYMORPH=17,SPELL_MAGE_MIRROR_IMAGE_FROSTBOLT=59638,SPELL_MAGE_MIRROR_IMAGE_FIRE_BLAST=59637;
struct Unit{bool a52=true,cheat=false,alive=true,combat=true,visible=true,casting=false,poly=false,cc=false,frost=false;int max=1000,mana=100,healed=0,casts=0,interrupts=0,last=0;Unit*victim=nullptr;Unit*ToPlayer(){return this;}bool GetCommandStatus(int){return cheat;}void ApplySpellMod(uint32,int,int32&){}int GetMaxPower(int){return max;}int GetPower(int){return mana;}void EnergizeBySpell(Unit*,int,int n,int){healed+=n;mana=std::min(max,mana+n);}template<class T>void CastSpell(Unit*,T,bool){++casts;}bool IsInCombat(){return combat;}Unit*GetVictim(){return victim;}bool IsAlive(){return alive;}bool CanSeeOrDetect(Unit*){return visible;}void InterruptNonMeleeSpells(bool,int spell=0){++interrupts;last=spell;}bool HasUnitState(int){return casting;}bool HasAuraWithMechanic(uint64_t){return poly;}bool FindCurrentSpellBySpellId(int){return frost;}bool HasBreakableByDamageCrowdControlAura(Unit*){return cc;}};using Player=Unit;
namespace AscensionFreePick{bool Applies(Unit*p){return p&&p->a52;}}
struct SpellProcEntry{Milliseconds Cooldown{6000};};
struct Aura{Unit*owner;uint32 id=816819;TimePoint m_procCooldown{};int GetType()const{return 1;}Unit*GetUnitOwner()const{return owner;}uint32 GetId()const{return id;}bool IsProcOnCooldown(TimePoint)const;void AddProcCooldown(TimePoint end){m_procCooldown=end;}void AddProcCooldown(SpellProcEntry const*,TimePoint);};
struct Application{int mode=1;int GetRemoveMode(){return mode;}};
struct AuraEffect{int GetEffIndex()const{return 2;}};
template<class T,class V>auto CalculatePct(T a,V b){return a*b/100;}
struct Soul{Unit*target;Application app;Application*GetTargetApplication(){return &app;}Unit*GetTarget(){return target;}@REMOVE@};
struct Info{struct E{uint32 TriggerSpell=58836;}Effects[3];};
struct Mirror{Unit*target;Info info;Unit*GetTarget(){return target;}Info*GetSpellInfo(){return &info;}@APPLY@ @PERIODIC@};
struct Events{int ready=0,updates=0;Milliseconds delay{0};void Update(uint32){++updates;}uint32 ExecuteEvent(){int n=ready;ready=0;return n;}void RescheduleEvent(uint32,Milliseconds d){delay=d;}};
struct AI{bool _delayAttack=true;Unit*me;uint32 checktarget=0;int selections=0;Events events;void MySelectNextTarget(){++selections;}@AI@};
@CHECK@
@ADD@
int main(){Unit p;Aura a{&p};SpellProcEntry entry;TimePoint t{};assert(!a.IsProcOnCooldown(t));a.AddProcCooldown(&entry,t);assert(a.IsProcOnCooldown(t+5999ms));assert(!a.IsProcOnCooldown(t+6000ms));p.cheat=true;assert(!a.IsProcOnCooldown(t));p.cheat=false;
Soul soul{&p};soul.AfterRemove(nullptr,0);assert(p.healed==90&&p.mana==190);p.mana=p.max;soul.AfterRemove(nullptr,0);assert(p.healed==90);for(int mode:{0,2,3,4}){p.mana=100;soul.app.mode=mode;soul.AfterRemove(nullptr,0);assert(p.healed==90);}p.a52=false;soul.app.mode=1;p.healed=0;soul.AfterRemove(nullptr,0);assert(p.healed==45);
Mirror mirror{&p};AuraEffect effect;bool periodic=true;int32 amplitude=1000;mirror.CalcPeriodic(&effect,periodic,amplitude);assert(!periodic);mirror.HandleEffectApply(&effect,0);assert(p.casts==1);
Unit image,target;image.victim=&target;AI ai;ai.me=&image;ai.events.ready=59638;ai.UpdateAI(100);assert(image.casts==0&&ai.events.updates==0);ai._delayAttack=false;ai.UpdateAI(100);assert(image.casts==1&&ai.events.delay==2500ms);ai.events.ready=59637;ai.UpdateAI(100);assert(image.casts==2&&ai.events.delay==6500ms);target.cc=true;ai.events.ready=59638;ai.UpdateAI(100);assert(image.casts==2);target.cc=false;image.casting=true;image.frost=true;target.poly=true;ai.UpdateAI(100);assert(image.interrupts==1&&image.last==59638);image.frost=false;ai.UpdateAI(100);assert(image.interrupts==1);image.casting=false;target.poly=false;target.alive=false;ai.UpdateAI(100);assert(ai.selections==1&&image.interrupts==2);target.alive=true;image.visible=false;ai.UpdateAI(1000);assert(ai.selections==2);image.victim=nullptr;ai.UpdateAI(100);assert(ai.selections==3);
}
