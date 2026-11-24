#include <cassert>
#include <map>
#include <vector>
#include <chrono>
#include <cstdint>
#include <string>
#include <type_traits>
using uint32=uint32_t;
constexpr int EFFECT_0=0,EFFECT_1=1,EFFECT_2=2,TARGET_UNIT_CASTER=1,SPELL_ATTR0_CU_NEGATIVE_EFF0=1,SPELL_SCHOOL_MASK_ALL=127,SPELL_EFFECT_APPLY_AURA=6,SPELL_AURA_PROC_TRIGGER_SPELL=42,SPELL_EFFECT_APPLY_AREA_AURA_RAID=119,SPELLFAMILY_WARLOCK=5,SPELL_AURA_MOD_IGNORE_SHAPESHIFT=275;
constexpr int PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS=16,PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS=256,PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG=4096,PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG=65536,SPELL_EFFECT_ASCENSION_RESTORE_BASE_HEALTH_PCT=181,SPELL_AURA_NONE=0,SPELL_ATTR2_CANT_CRIT=0x20000000;
struct flag96{uint32 bits[3];flag96(uint32 a=0,uint32 b=0,uint32 c=0):bits{a,b,c}{}uint32&operator[](int i){return bits[i];}explicit operator bool()const{return bits[0]||bits[1]||bits[2];}flag96&operator|=(flag96 const&o){for(int i=0;i<3;++i)bits[i]|=o.bits[i];return *this;}};
struct SpellImplicitTargetInfo{int value;SpellImplicitTargetInfo(int v=0):value(v){}};
struct Effect{int Amplitude=0,Effect=0,ApplyAuraName=0,MiscValue=0,TriggerSpell=0,BasePoints=0;float RealPointsPerLevel=0;SpellImplicitTargetInfo TargetA,TargetB;void*RadiusEntry=nullptr;flag96 SpellClassMask{0,0,0};bool IsEffect()const{return Effect!=0;}bool IsAura()const{return Effect==6;}bool IsAura(int aura)const{return IsAura()&&ApplyAuraName==aura;}};
struct SpellInfo{uint32 Id;Effect Effects[3];uint32 AttributesEx2=0,ProcFlags=0,ProcChance=0,AttributesCu=0,ProcCharges=0,StackAmount=0,SpellFamilyName=0,SpellFamilyFlags[3]={};};
struct Store{void*LookupEntry(int){return nullptr;}}sSpellRadiusStore;
struct Config{bool enable=true;std::string model="hero",realm="live";uint32 mask=0;template<class T>T GetOption(std::string key,T fallback){if constexpr(std::is_same_v<T,bool>)return enable;else if constexpr(std::is_same_v<T,std::string>)return key=="CoA.ClassModel"?model:realm;else return mask;}}config;Config*sConfigMgr=&config;
struct Hook{@HOOK@};
struct WorldObject{bool inrange=true;template<class T>bool IsWithinDistInMap(T*,float)const{return inrange;}};
struct Unit:WorldObject{bool vehicle=false,party=false,raid=false,hostile=false,alive=true;bool IsVehicle(){return vehicle;}bool IsInRaidWith(Unit*u)const{return u->raid;}bool IsInPartyWith(Unit*u)const{return u->party;}bool IsHostileTo(Unit*u)const{return u->hostile;}bool IsAlive(){return alive;}};
@GRID@
using uint8=uint8_t;using uint16=uint16_t;using Milliseconds=std::chrono::milliseconds;
constexpr int MAX_SPELL_EFFECTS=3,TOTAL_AURAS=512,TAKEN_HIT_PROC_FLAG_MASK=0;
@PROC_CONSTANTS@
bool isTriggerAura[TOTAL_AURAS],isAlwaysTriggeredAura[TOTAL_AURAS];uint32 spellTypeMask[TOTAL_AURAS];
@INIT_PROCS@
struct SpellProcEntry{int SchoolMask=0,SpellFamilyName=0,ProcFlags=0,SpellTypeMask=0,SpellPhaseMask=0,HitMask=0,AttributesMask=0,DisableEffectsMask=0,Charges=0;flag96 SpellFamilyMask;float Chance=0,ProcsPerMinute=0;Milliseconds Cooldown{0};};
struct ProcManager{std::vector<SpellInfo const*>mSpellInfoMap;std::map<uint32,SpellProcEntry>mSpellProcMap;std::map<uint32,uint32>_spellTwinSources;SpellInfo const*GetSpellInfo(uint32){return nullptr;}void Generate(){uint32 count=0;@GENERATE_PROCS@}};
int main(){Hook h;
InitTriggerAuraData();ProcManager manager;std::vector<SpellInfo> records;for(uint32 id:{28176u,28189u,47892u,47893u}){records.push_back(SpellInfo{id});auto&f=records.back();f.Effects[0].Effect=6;f.Effects[0].ApplyAuraName=220;h.OnLoadSpellCustomAttr(&f);}for(auto&f:records)manager.mSpellInfoMap.push_back(&f);for(int reload=0;reload<2;++reload){manager.mSpellProcMap.clear();manager.Generate();assert(manager.mSpellProcMap.size()==4);for(auto const&[id,entry]:manager.mSpellProcMap){assert(entry.ProcFlags==69904&&entry.Chance==10&&entry.SpellPhaseMask==PROC_SPELL_PHASE_HIT&&entry.DisableEffectsMask==1&&!entry.SpellFamilyMask&&entry.Charges==0&&entry.Cooldown.count()==0);}}manager.Generate();assert(manager.mSpellProcMap.size()==4);

for(uint32 id:{28176u,28189u,47892u,47893u}){SpellInfo f{id};f.Effects[1].ApplyAuraName=20;f.Effects[1].Amplitude=10000;h.OnLoadSpellCustomAttr(&f);h.OnLoadSpellCustomAttr(&f);assert(f.Effects[1].ApplyAuraName==42&&f.Effects[1].TriggerSpell==828176&&f.Effects[1].Amplitude==0&&f.ProcFlags==69904&&f.ProcChance==10&&f.ProcCharges==0);}
SpellInfo heal{828176};h.OnLoadSpellCustomAttr(&heal);assert(heal.Effects[0].Effect==181&&heal.Effects[0].ApplyAuraName==0&&heal.Effects[0].BasePoints==2&&(heal.AttributesEx2&SPELL_ATTR2_CANT_CRIT));
config.model="coa";SpellInfo untouched{28189};untouched.Effects[1].ApplyAuraName=20;h.OnLoadSpellCustomAttr(&untouched);assert(untouched.Effects[1].ApplyAuraName==20&&untouched.ProcFlags==0);config=Config{};
for(int i=0;i<2;++i){SpellInfo earth{48511};h.OnLoadSpellCustomAttr(&earth);assert(earth.Effects[2].Effect==6&&earth.Effects[2].ApplyAuraName==42&&earth.Effects[2].TriggerSpell==301118&&earth.Effects[2].TargetA.value==1);h.OnLoadSpellCustomAttr(&earth);assert(earth.Effects[2].TriggerSpell==301118);}for(uint32 id:{48506u,48510u}){SpellInfo s{id};h.OnLoadSpellCustomAttr(&s);assert(s.Effects[2].Effect==0);}
for(uint32 id:{57658u,57660u,57662u,57663u}){SpellInfo s{id};s.Effects[0].Effect=65;s.Effects[1].Effect=65;h.OnLoadSpellCustomAttr(&s);assert(s.Effects[0].Effect==119&&s.Effects[1].Effect==119);h.OnLoadSpellCustomAttr(&s);assert(s.Effects[0].Effect==119);}
for(int mode=0;mode<4;++mode){config=Config{};if(mode==0)config.enable=false;if(mode==1)config.model="coa";if(mode==2)config.realm="league";if(mode==3)config.mask=1;SpellInfo s{48511};h.OnLoadSpellCustomAttr(&s);assert(s.Effects[2].Effect==0);SpellInfo totem{57658};totem.Effects[0].Effect=65;h.OnLoadSpellCustomAttr(&totem);assert(totem.Effects[0].Effect==65);}config=Config{};config.mask=0x400;SpellInfo a{48511};h.OnLoadSpellCustomAttr(&a);assert(a.Effects[2].TriggerSpell==301118);
Unit source,member;member.raid=true;AnyGroupedUnitInObjectRangeCheck old(&source,&source,30,false),fixed(&source,&source,30,true);assert(!old(&member)&&fixed(&member));member.party=true;assert(old(&member)&&fixed(&member));member.hostile=true;assert(!fixed(&member));member.hostile=false;member.alive=false;assert(!fixed(&member));member.alive=true;member.vehicle=true;assert(!fixed(&member));member.vehicle=false;source.inrange=false;assert(!fixed(&member));source.inrange=true;member.raid=false;assert(!fixed(&member));
}
