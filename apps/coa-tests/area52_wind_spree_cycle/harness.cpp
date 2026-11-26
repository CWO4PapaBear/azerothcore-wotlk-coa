#include <cassert>
#include <list>
#include <map>
#include <vector>
#include <cstdint>
using uint8=uint8_t;using uint32=uint32_t;using int32=int32_t;using ObjectGuid=unsigned;using AuraEffectHandleModes=int;using WorldLocation=int;using TriggerCastFlags=unsigned;
constexpr unsigned SPELL_ROGUE_KILLING_SPREE_TELEPORT=57840,SPELL_ROGUE_KILLING_SPREE_WEAPON_DMG=57841,TRIGGERED_FULL_MASK=255,TRIGGERED_DONT_REPORT_CAST_ERROR=4;
constexpr int SPELL_ATTR5_EXTRA_INITIAL_PERIOD=512;
struct Unit{unsigned id=0,teleports=0,hits=0,positions=0;bool valid=true,stealth=false,invisible=false;unsigned GetGUID(){return id;}bool HasStealthAura(){return stealth;}bool HasInvisibilityAura(){return invisible;}bool IsValidAttackTarget(Unit*u){return u->valid;}void CastSpell(Unit*,unsigned spell,unsigned){if(spell==57840)++teleports;else{assert(spell==57841);++hits;}}Unit*ToPlayer(){return this;}WorldLocation const&GetTeleportDest(){static int pos=1;return pos;}void UpdatePosition(WorldLocation const&,bool){++positions;}};
namespace ObjectAccessor{std::map<unsigned,Unit*>units;Unit*GetUnit(Unit&,unsigned id){return units.contains(id)?units[id]:nullptr;}}
namespace Acore::Containers{unsigned SelectRandomContainerElement(std::list<unsigned>const&ids){return ids.front();}}
struct Manager{unsigned GetSpellTwinFor(unsigned id,unsigned){return id;}}manager;Manager*sSpellMgr=&manager;
struct AuraEffect;struct Probe{Unit*owner;std::list<unsigned>_targets;Unit*GetTarget(){return owner;}unsigned GetId(){return 51690;}@PERIODIC@};
struct AuraApplication{bool HasEffect(int){return true;}};
struct SpellInfo{int AttributesEx3=0,AttributesEx4=0;unsigned Id=51690;bool initial=true;bool HasAttribute(int)const{return initial;}};
struct Base{int duration=2000;int GetDuration()const{return duration;}int GetMaxDuration()const{return 2000;}bool IsPermanent()const{return false;}bool IsPassive()const{return false;}};
struct AuraEffect{bool m_isPeriodic=true;int m_amplitude=500,m_periodicTimer=0;unsigned m_tickNumber=0;SpellInfo info;SpellInfo*m_spellInfo=&info;Base base;Probe*probe;AuraApplication app;Base const*GetBase()const{return &base;}Base*GetBase(){return &base;}int GetEffIndex(){return 0;}void UpdatePeriodic(Unit*){}void GetApplicationList(std::list<AuraApplication*>&out){out.push_back(&app);}void PeriodicTick(AuraApplication*,Unit*){probe->HandleEffectPeriodic(this);}int32 GetTotalTicks()const;void Update(uint32,Unit*);};
@TOTAL@ @UPDATE@
bool Area52WindMetadataEnabled=false;constexpr int SPELL_ATTR3_SUPPRESS_CASTER_PROCS=0x10000,SPELL_ATTR4_SUPPRESS_WEAPON_PROCS=0x800000;
@CONFIG@
int main(){Unit owner,target,hidden,missing;hidden.stealth=true;target.id=2;ObjectAccessor::units={{1,&hidden},{2,&target}};Probe probe{&owner,{999,1,2}};AuraEffect effect;effect.probe=&probe;
 assert(effect.GetTotalTicks()==5);effect.Update(0,&owner);assert(owner.hits==1&&probe._targets.size()==1);effect.Update(499,&owner);assert(owner.hits==1);effect.Update(1,&owner);assert(owner.hits==2);effect.Update(1500,&owner);assert(owner.hits==5&&owner.teleports==5&&owner.positions==5);effect.Update(500,&owner);assert(owner.hits==5);
 target.invisible=true;probe.HandleEffectPeriodic(nullptr);assert(probe._targets.empty()&&owner.hits==5);
 SpellInfo off;off.Id=57842;ConfigureArea52WindTalents(&off);assert(!off.AttributesEx3&&!off.AttributesEx4);Area52WindMetadataEnabled=true;ConfigureArea52WindTalents(&off);assert(off.AttributesEx3&0x10000);assert(off.AttributesEx4&0x800000);SpellInfo main;main.Id=57841;ConfigureArea52WindTalents(&main);assert(!main.AttributesEx3&&!main.AttributesEx4);
}
