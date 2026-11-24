#include <algorithm>
#include <cassert>
#include <cstdint>
#include <set>
using uint32=uint32_t;using int32=int32_t;using SpellEffIndex=int;
constexpr int SPELL_DIRECT_DAMAGE=1,SPELL_DAMAGE_CLASS_MELEE=2,CREATURE_TYPE_DEMON=3,SPELLMOD_CHANCE_OF_SUCCESS=1,PROC_ATTR_REDUCE_PROC_60=1,SPELL_EFFECT_HANDLE_LAUNCH_TARGET=1,HEAL=1;
struct SpellInfo{uint32 Id=828176;int DmgClass=0;struct Time{int CastTime=1000;}*CastTimeEntry=nullptr;bool IsRangedWeaponSpell()const{return false;}};
struct DamageInfo{int amount=100,type=1;uint32 GetDamage()const{return amount;}int GetDamageType()const{return type;}int GetAttackType()const{return 0;}};
struct Pet{bool alive=true,world=true;int type=3;bool IsAlive(){return alive;}bool IsInWorld(){return world;}int GetCreatureType(){return type;}};
struct Player{bool area=true,alive=true;int base=1000,maximum=9000,level=80;float taken=1;int doneCalls=0;Pet*pet=nullptr;std::set<uint32>auras;Player*ToPlayer(){return this;}Player*GetSpellModOwner(){return this;}Pet*GetPet(){return pet;}bool HasAura(uint32 id){return auras.count(id);}int GetAttackTime(int){return 2000;}float GetPPMProcChance(int,float,SpellInfo const*){return 7;}void ApplySpellMod(uint32,int,float&){}uint32 GetLevel(){return level;}bool IsAlive(){return alive;}uint32 GetCreateHealth(){return base;}uint32 SpellHealingBonusDone(Player*,SpellInfo const*,uint32 h,int,int){++doneCalls;return h+1000;}uint32 SpellHealingBonusTaken(Player*,SpellInfo const*,uint32 h,int){return h*taken;}};
using Unit=Player;
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct Cast{bool WasStationaryDuringCast(){return false;}};
struct ProcEventInfo{DamageInfo*damage=nullptr;SpellInfo*info=nullptr;Player*actor=nullptr;DamageInfo*GetDamageInfo(){return damage;}void*GetHealInfo(){return nullptr;}SpellInfo const*GetSpellInfo(){return info;}Player*GetActor(){return actor;}Cast*GetProcSpell(){return nullptr;}};
struct SpellProcEntry{float Chance=10,ProcsPerMinute=0;uint32 AttributesMask=0;};
struct Aura{uint32 id=28176;Player*player=nullptr;SpellInfo info;uint32 GetId()const{return id;}Player*GetCaster()const{return player;}SpellInfo const*GetSpellInfo()const{return &info;}float CalcProcChance(SpellProcEntry const&,ProcEventInfo&)const;};
template<class T>uint32 CalculatePct(T value,int pct){return value*pct/100;}
struct Spell{int effectHandleMode=1,damage=3,m_damage=0,m_damageBeforeTakenMods=0;Player*unitTarget=nullptr,*m_originalCaster=nullptr;SpellInfo*m_spellInfo=nullptr;void EffectAscensionRestoreBaseHealthPct(SpellEffIndex);};
@CHANCE@
@HEAL@
int main(){Player player,other;Pet pet;SpellInfo info;DamageInfo damage;ProcEventInfo event{&damage,&info,&player};SpellProcEntry entry;Aura aura;aura.player=&player;
for(auto id:{28176u,28189u,47892u,47893u}){aura.id=id;for(int rank=0;rank<=3;++rank){player.auras.clear();if(rank)player.auras.insert(30142+rank);player.pet=nullptr;assert(aura.CalcProcChance(entry,event)==10+2*rank);player.pet=&pet;assert(aura.CalcProcChance(entry,event)==15+2*rank);pet.alive=false;assert(aura.CalcProcChance(entry,event)==10+2*rank);pet.alive=true;pet.world=false;assert(aura.CalcProcChance(entry,event)==10+2*rank);pet.world=true;pet.type=1;assert(aura.CalcProcChance(entry,event)==10+2*rank);pet.type=3;}}
player.auras={30143,30144,30145};assert(aura.CalcProcChance(entry,event)==21);player.auras={30143};assert(aura.CalcProcChance(entry,event)==17);player.auras.clear();assert(aura.CalcProcChance(entry,event)==15);
damage.type=2;assert(aura.CalcProcChance(entry,event)==0);damage.type=1;damage.amount=0;assert(aura.CalcProcChance(entry,event)==0);damage.amount=100;event.info=nullptr;assert(aura.CalcProcChance(entry,event)==0);event.info=&info;event.actor=&other;assert(aura.CalcProcChance(entry,event)==0);event.actor=&player;player.area=false;entry.Chance=27;assert(aura.CalcProcChance(entry,event)==27);player.area=true;
Spell spell;spell.unitTarget=&player;spell.m_originalCaster=&player;spell.m_spellInfo=&info;spell.EffectAscensionRestoreBaseHealthPct(0);assert(spell.m_damage==-30&&player.doneCalls==0);player.maximum=20000;spell.m_damage=0;spell.EffectAscensionRestoreBaseHealthPct(0);assert(spell.m_damage==-30);player.base=2000;spell.m_damage=0;spell.EffectAscensionRestoreBaseHealthPct(0);assert(spell.m_damage==-60);player.taken=.5;spell.m_damage=0;spell.EffectAscensionRestoreBaseHealthPct(0);assert(spell.m_damage==-30);player.alive=false;spell.m_damage=0;spell.EffectAscensionRestoreBaseHealthPct(0);assert(spell.m_damage==0);player.alive=true;spell.m_originalCaster=nullptr;spell.EffectAscensionRestoreBaseHealthPct(0);assert(spell.m_damage==0);spell.m_originalCaster=&player;player.area=false;spell.EffectAscensionRestoreBaseHealthPct(0);assert(player.doneCalls==1);
}
