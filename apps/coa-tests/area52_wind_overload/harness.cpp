#include <algorithm>
#include <cassert>
#include <cstdint>
#include <set>
using uint32=uint32_t;using int32=int32_t;using SpellEffIndex=int;
constexpr int PROC_EX_CRITICAL_HIT=2;
constexpr int SPELL_DIRECT_DAMAGE=1,SPELL_DAMAGE_CLASS_MELEE=2,CREATURE_TYPE_DEMON=3,SPELLMOD_CHANCE_OF_SUCCESS=1,PROC_ATTR_REDUCE_PROC_60=1,SPELL_EFFECT_HANDLE_LAUNCH_TARGET=1,HEAL=1;
struct SpellInfo{uint32 SpellFamilyFlags[3]={};uint32 Id=828176;int DmgClass=0;struct Time{int CastTime=1000;}*CastTimeEntry=nullptr;bool IsRangedWeaponSpell()const{return false;}};
struct DamageInfo{int amount=100,type=1;uint32 GetDamage()const{return amount;}int GetDamageType()const{return type;}int GetAttackType()const{return 0;}};
struct Pet{bool alive=true,world=true;int type=3;bool IsAlive(){return alive;}bool IsInWorld(){return world;}int GetCreatureType(){return type;}};
struct Player{bool IsPlayer(){return true;}bool area=true,alive=true;int base=1000,maximum=9000,level=80;float taken=1;int doneCalls=0;Pet*pet=nullptr;std::set<uint32>auras;bool MatchesSpellInteraction(uint32 id,uint32 original){return id==original||(id==997083&&original==403); }Player*ToPlayer(){return this;}Player*GetSpellModOwner(){return this;}Pet*GetPet(){return pet;}bool HasAura(uint32 id){return auras.count(id);}int GetAttackTime(int){return 2000;}float GetPPMProcChance(int,float,SpellInfo const*){return 7;}void ApplySpellMod(uint32,int,float&){}uint32 GetLevel(){return level;}bool IsAlive(){return alive;}uint32 GetCreateHealth(){return base;}uint32 SpellHealingBonusDone(Player*,SpellInfo const*,uint32 h,int,int){++doneCalls;return h+1000;}uint32 SpellHealingBonusTaken(Player*,SpellInfo const*,uint32 h,int){return h*taken;}};
using Unit=Player;
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct Cast{bool WasStationaryDuringCast(){return false;}};
struct ProcEventInfo{uint32 GetHitMask(){return hit;}uint32 hit=0;Player*target=nullptr;Player*GetActionTarget(){return target;}DamageInfo*damage=nullptr;SpellInfo*info=nullptr;Player*actor=nullptr;DamageInfo*GetDamageInfo(){return damage;}void*GetHealInfo(){return nullptr;}SpellInfo const*GetSpellInfo(){return info;}Player*GetActor(){return actor;}Cast*GetProcSpell(){return nullptr;}};
struct SpellProcEntry{float Chance=10,ProcsPerMinute=0;uint32 AttributesMask=0;};
struct Aura{uint32 id=28176;Player*player=nullptr;SpellInfo info;uint32 GetId()const{return id;}Player*GetCaster()const{return player;}SpellInfo const*GetSpellInfo()const{return &info;}float CalcProcChance(SpellProcEntry const&,ProcEventInfo&)const;};
template<class T>uint32 CalculatePct(T value,int pct){return value*pct/100;}
struct Spell{int effectHandleMode=1,damage=3,m_damage=0,m_damageBeforeTakenMods=0;Player*unitTarget=nullptr,*m_originalCaster=nullptr;SpellInfo*m_spellInfo=nullptr;void EffectAscensionRestoreBaseHealthPct(SpellEffIndex);};
@CHANCE@


bool rolled=false;bool roll_chance_i(int){rolled=true;return false;}
struct ProcProbe{Player*target=nullptr;Player*GetTarget(){return target;} @CHECK@ };
int main(){Player p,enemy;SpellInfo spell;DamageInfo damage;ProcEventInfo event;event.actor=&p;event.target=&enemy;event.info=&spell;event.damage=&damage;Aura aura;aura.player=&p;SpellProcEntry entry;
for(auto id:{30675u,30678u,30679u}){aura.id=id;entry.Chance=id==30675?7:id==30678?14:20;event.hit=0;assert(aura.CalcProcChance(entry,event)==entry.Chance);event.hit=2;assert(aura.CalcProcChance(entry,event)==3*entry.Chance);p.area=false;assert(aura.CalcProcChance(entry,event)==entry.Chance);p.area=true;}
for(auto id:{30816u,30818u,30819u}){aura.id=id;entry.Chance=id==30816?3:id==30818?6:9;for(unsigned hit:{0u,2u}){event.hit=hit;assert(aura.CalcProcChance(entry,event)==entry.Chance);}}
aura.id=978232;entry.Chance=10;event.hit=0;assert(aura.CalcProcChance(entry,event)==10);event.hit=2;assert(aura.CalcProcChance(entry,event)==10);ProcProbe probe;probe.target=&p;for(auto id:{403u,421u,997083u}){spell.Id=id;assert(probe.CheckProc(event));assert(!rolled);}spell.Id=123;assert(!probe.CheckProc(event));spell.Id=403;damage.type=2;assert(!probe.CheckProc(event));damage.type=1;damage.amount=0;assert(!probe.CheckProc(event));damage.amount=100;event.actor=&enemy;assert(!probe.CheckProc(event));event.actor=&p;p.area=false;spell.SpellFamilyFlags[0]=2;assert(!probe.CheckProc(event)&&rolled);
}
