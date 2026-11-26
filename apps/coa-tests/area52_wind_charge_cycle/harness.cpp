#include <cassert>
#include <chrono>
#include <cstdint>
#include <algorithm>
using uint8=uint8_t;using uint32=uint32_t;using int32=int32_t;using Milliseconds=std::chrono::milliseconds;using TimePoint=std::chrono::steady_clock::time_point;
constexpr int UNIT_AURA_TYPE=1,CHEAT_COOLDOWN=2,SPELLMOD_DURATION=3,SPELL_ATTR6_DO_NOT_CONSUME_RESOURCES=4,PROC_ATTR_USE_STACKS_FOR_CHARGES=8;
#define ASSERT assert
struct Player{bool GetCommandStatus(int){return false;}void ApplySpellMod(uint32,int,int32&){}Player*ToPlayer(){return this;}};
namespace AscensionFreePick{bool Applies(Player*){return false;}}
struct SpellInfo{bool resourceFree=false;bool HasAttribute(int)const{return resourceFree;}};
struct SpellProcEntry{Milliseconds Cooldown{500};uint32 AttributesMask=0;};
struct Manager{SpellProcEntry entry;SpellProcEntry const*GetSpellProcEntry(uint32){return &entry;}}manager;Manager*sSpellMgr=&manager;
struct AuraApplication{};struct ProcEventInfo{SpellInfo*spell=nullptr;SpellInfo const*GetSpellInfo(){return spell;}};
struct Aura{uint32 id=16257;uint8 m_procCharges=3;bool m_isUsingCharges=true,removed=false,prepare=true;TimePoint m_procCooldown{};Player owner;unsigned notifications=0;uint32 GetId()const{return id;}int GetType()const{return UNIT_AURA_TYPE;}Player*GetUnitOwner()const{return const_cast<Player*>(&owner);}bool CallScriptPrepareProcHandlers(AuraApplication*,ProcEventInfo&){return prepare;}bool IsUsingCharges()const{return m_isUsingCharges;}uint8 GetCharges(){return m_procCharges;}void SetNeedClientUpdateForTargets(){++notifications;}void Remove(){removed=true;}void ModStackAmount(int){assert(false);}bool IsProcOnCooldown(TimePoint)const;void AddProcCooldown(TimePoint);void AddProcCooldown(SpellProcEntry const*,TimePoint);void PrepareProcToTrigger(AuraApplication*,ProcEventInfo&,TimePoint);void ConsumeProcCharges(SpellProcEntry const*);void SetCharges(uint8);};
@METHODS@
int main(){AuraApplication app;ProcEventInfo event;TimePoint t{};
manager.entry.Cooldown=Milliseconds(0);Aura charge;charge.id=997084;charge.m_procCharges=1;charge.PrepareProcToTrigger(&app,event,t);assert(charge.GetCharges()==0);charge.ConsumeProcCharges(&manager.entry);assert(charge.removed);
manager.entry.Cooldown=Milliseconds(6000);Aura calm;calm.id=978232;calm.m_isUsingCharges=false;calm.PrepareProcToTrigger(&app,event,t);assert(calm.GetCharges()==3);assert(calm.IsProcOnCooldown(t));assert(calm.IsProcOnCooldown(t+Milliseconds(5999)));assert(!calm.IsProcOnCooldown(t+Milliseconds(6000)));calm.ConsumeProcCharges(&manager.entry);assert(!calm.removed);
}
