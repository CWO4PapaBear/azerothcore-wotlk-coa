#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
using uint32=uint32_t;using int8=int8_t;using SpellEffIndex=int;using Powers=int;
constexpr int SPELL_EFFECT_HANDLE_HIT_TARGET=1,TARGET_CHECK_ENTRY=9,CREATURE_TYPE_BEAST=1,CREATURE_TYPE_GIANT=5,UNIT_STATE_ROOT=1,UNIT_STATE_ISOLATED=2,SPELL_EFFECT_KNOCK_BACK_DEST=144,MAX_POWERS=7,SPELL_ATTR7_ONLY_IN_SPELLBOOK_UNTIL_LEARNED=1,POWER_MANA=0,SPELL_DRUID_GLYPH_OF_TYPHOON=62135,SPELL_SHAMAN_GLYPH_OF_THUNDERSTORM=62132;
struct Guid{int v=0;bool IsEmpty(){return !v;}};
struct Position{float x=2,y=3;void GetPosition(float&a,float&b){a=x;b=y;}float GetPositionX(){return x;}float GetPositionY(){return y;}};
struct Unit:Position{bool vehicle=false,riding=false,creature=false,boss=false,dungeon=false,immune=false,alive=true,player=false,eligible=true,casting=true,glyph=false;int type=1,state=0,knocks=0,interrupts=0,max=1000,power=0,entry=0,immunes=0;Unit* owner=nullptr;float sx=0,sz=0,kx=0,ky=0;bool IsVehicle(){return vehicle;}int GetCreatureType(){return type;}Unit* GetVehicle(){return riding?this:nullptr;}Unit* ToCreature(){return creature?this:nullptr;}bool isWorldBoss(){return boss;}bool IsDungeonBoss(){return dungeon;}bool IsImmuneToKnockback(){return immune;}bool HasUnitState(int v){return state&v;}bool IsNonMeleeSpellCast(bool){return casting;}void InterruptNonMeleeSpells(bool){++interrupts;}void KnockbackFrom(float a,float b,float c,float d){++knocks;kx=a;ky=b;sx=c;sz=d;}bool IsPlayer(){return player;}Unit* ToPlayer(){return player?this:nullptr;}bool IsAlive(){return alive;}void SendSpellDamageImmune(Unit*,uint32){++immunes;}bool CanReceivePowerFromSpell(int){return eligible;}int GetEntry(){return entry;}Unit* GetOwner(){return owner;}uint32 GetCreateMana(){return 100;}uint32 GetMaxPower(int){return max;}void EnergizeBySpell(Unit*t,uint32,uint32 n,int){t->power=std::min(t->max,t->power+int(n));}bool HasAuraOrTwin(uint32){return glyph;}};
using Creature=Unit;namespace ObjectAccessor{Unit* GetUnit(Unit&,Guid){return nullptr;}}
namespace AscensionFreePick{bool Applies(Unit*p){return p;}}
struct Scripts{void AnticheatSetUnderACKmount(Unit*){}}scripts;Scripts*sScriptMgr=&scripts;
struct Target{int check=0;int GetCheckType()const{return check;}};
struct Effect{Target TargetA,TargetB;int Effect=98,MiscValue=150;};
struct Info{uint32 Id=51490;Effect Effects[3];bool HasAttribute(int)const{return false;}};
struct Targets{bool dst=false;bool HasDst(){return dst;}};
struct Spell{int effectHandleMode=1,damage=60;Unit*m_caster=nullptr,*unitTarget=nullptr,*m_reflectionTarget=nullptr;Guid m_reflectionTargetGuid;Position m_reflectionTargetPosition;Targets m_targets;Position*destTarget=nullptr;Info*m_spellInfo;Info*GetSpellInfo(){return m_spellInfo;}void EffectKnockBack(int);void EffectEnergizePct(int);};
template<class T,class V>auto CalculatePct(T a,V b){return a*b/100;}
struct Typhoon{Unit*caster;int prevented=-1;Unit*GetCaster(){return caster;}void PreventHitDefaultEffect(int e){prevented=e;}@TYPHOON@};
struct Thunder{Unit*caster;int prevented=-1;Unit*GetCaster(){return caster;}void PreventHitDefaultEffect(int e){prevented=e;}@THUNDER@};
@KNOCK@
@MANA@
int main(){Unit caster,target;Info info;Spell s;s.m_spellInfo=&info;s.m_caster=&caster;s.unitTarget=&target;s.EffectKnockBack(0);assert(target.knocks==1&&target.interrupts==1&&target.sx==15&&target.sz==6);for(int reason=0;reason<6;++reason){Unit blocked;blocked.creature=true;switch(reason){case 0:blocked.boss=true;break;case 1:blocked.dungeon=true;break;case 2:blocked.immune=true;break;case 3:blocked.type=5;break;case 4:blocked.state=1;break;case 5:blocked.riding=true;break;}s.unitTarget=&blocked;s.EffectKnockBack(0);assert(!blocked.knocks&&!blocked.interrupts);}s.unitTarget=&target;Unit reflected;reflected.x=11;reflected.y=12;s.m_reflectionTarget=&reflected;s.EffectKnockBack(0);assert(target.kx==11&&target.ky==12);Typhoon ty{&caster};Thunder th{&caster};ty.HandleKnockBack(0);th.HandleKnockBack(2);assert(ty.prevented==-1&&th.prevented==-1);caster.glyph=true;ty.HandleKnockBack(0);th.HandleKnockBack(2);assert(ty.prevented==0&&th.prevented==2);
info.Effects[1].MiscValue=0;s.damage=10;target.player=true;target.max=12340;s.EffectEnergizePct(1);assert(target.power==1234);target.power=12300;s.EffectEnergizePct(1);assert(target.power==12340);target.power=0;target.eligible=false;s.EffectEnergizePct(1);assert(target.power==0);target.eligible=true;target.alive=false;s.EffectEnergizePct(1);assert(target.power==0);target.alive=true;target.state=2;s.EffectEnergizePct(1);assert(target.power==0&&caster.immunes==1);
}
