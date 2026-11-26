#include <cassert>
#include <cmath>
#include <algorithm>
#include <cstdint>
using uint8=uint8_t;using uint32=uint32_t;using int32=int32_t;using WeaponAttackType=int;
constexpr int BASE_ATTACK=0,OFF_ATTACK=1,RANGED_ATTACK=2,UNIT_FIELD_BASEATTACKTIME=0,AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK=1,AURA_EFFECT_HANDLE_STAT=2,SPELL_AURA_MOD_MELEE_HASTE=138;
int AsUnderlyingType(int i){return i;}void ApplyPercentModFloatVar(float&v,float amount,bool apply){if(apply)v*=1+amount/100;else v/=1+amount/100;}
struct AuraEffect;
struct Unit{float speed[3]={2000,1800,2500},m_modAttackSpeedPct[3]={1,1,1};uint32 m_attackTimer[3]={1000,900,1250};int exclusive=0;float GetFloatValue(int i){return speed[i];}void SetFloatValue(int i,float v){speed[i]=v;}float GetAttackTime(int i){return i==0?2000:i==1?1800:2500;}int GetHighestExclusiveSameEffectSpellGroupValue(AuraEffect const*,int){return exclusive;}void ApplyAttackTimePercentMod(WeaponAttackType,float,bool);};
struct AuraApplication{Unit*unit;Unit*GetTarget()const{return unit;}};
struct AuraEffect{int amount=0;int GetAmount()const{return amount;}void HandleModMeleeSpeedPct(AuraApplication const*,uint8,bool)const;};
@TIMER@ @HASTE@
bool near(float a,float b){return std::abs(a-b)<0.01f;}
int main(){for(int amount:{4,6,10,1,2,3,5,9}){Unit unit;AuraApplication app{&unit};AuraEffect effect{amount};effect.HandleModMeleeSpeedPct(&app,1,true);float factor=1+amount/100.f;assert(near(unit.speed[0],2000/factor)&&near(unit.speed[1],1800/factor)&&near(unit.speed[2],2500));assert(std::abs(int(unit.m_attackTimer[0])-int(1000/factor))<=1);effect.HandleModMeleeSpeedPct(&app,1,false);assert(near(unit.speed[0],2000)&&near(unit.speed[1],1800));assert(std::abs(int(unit.m_attackTimer[0])-1000)<=2);}
 Unit unit;AuraApplication app{&unit};AuraEffect effect{10};effect.HandleModMeleeSpeedPct(&app,0,true);assert(unit.speed[0]==2000);unit.exclusive=15;effect.HandleModMeleeSpeedPct(&app,1,true);assert(unit.speed[0]==2000);
}
