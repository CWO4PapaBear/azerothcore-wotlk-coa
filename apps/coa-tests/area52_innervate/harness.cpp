#include <algorithm>
#include <cassert>
#include <cstdint>
using int32=int32_t;using uint32=uint32_t;using Powers=int;
constexpr int POWER_MANA=0,SPELL_ATTR7_ONLY_IN_SPELLBOOK_UNTIL_LEARNED=1;
#define LOG_DEBUG(...)
template<class T,class V>auto CalculatePct(T a,V b){return a*b/100;}
struct Info{bool HasAttribute(int)const{return false;}};
struct Threat{float value=0;template<class...T>void ForwardThreatForAssistingMe(void*,float v,T...){value+=v;}};
struct Unit{bool player=true,alive=true,eligible=true,immune=false;int base=1000,power=0,max=10000,logs=0;Threat threat;int GetCreatePowers(int){return base;}bool IsPlayer(){return player;}bool CanReceivePowerFromSpell(int){return eligible;}bool IsAlive(){return alive;}int GetMaxPower(int){return max;}int GetPower(int){return power;}bool IsImmunedToAuraPeriodicTick(Unit*,Info const*){return immune;}template<class T>void SendPeriodicAuraLog(T*){++logs;}int ModifyPower(int,int delta){int old=power;power=std::clamp(power+delta,0,max);return power-old;}bool GainsManaWithoutThreat(){return false;}Threat& GetThreatMgr(){return threat;}};
struct Base{bool IsPermanent()const{return false;}};
struct AuraEffect{int m_amount=0,ticks=10;Info info;Info const* m_spellInfo=&info;Base base;int GetTotalTicks()const{return ticks;}int GetMiscValue()const{return 0;}Info const* GetSpellInfo()const{return &info;}void SendTickImmune(Unit*,Unit*)const{}Base const* GetBase()const{return &base;}void HandlePeriodicEnergizeAuraTick(Unit*,Unit*)const;};
struct SpellPeriodicAuraLogInfo{template<class...T>SpellPeriodicAuraLogInfo(T...){}};
struct Script{Unit* caster;Unit* GetCaster(){return caster;}@AMOUNT@};
@TICK@
int main(){for(int base:{100,1000,1234,5000}){Unit caster;caster.base=base;caster.max=90000;Unit target;target.max=100000;AuraEffect effect;Script script{&caster};int amount=150;bool recalc=true;script.CalculateAmount(&effect,amount,recalc);assert(amount==base*150/100/10);effect.m_amount=amount;for(int i=0;i<10;++i)effect.HandlePeriodicEnergizeAuraTick(&target,&caster);assert(target.power==10*amount);assert(target.threat.value==target.power*.5f);}
Unit caster,target;AuraEffect effect;effect.m_amount=150;target.power=9950;effect.HandlePeriodicEnergizeAuraTick(&target,&caster);assert(target.power==10000&&target.threat.value==25);target.power=0;target.eligible=false;effect.HandlePeriodicEnergizeAuraTick(&target,&caster);assert(target.power==0);target.eligible=true;target.alive=false;effect.HandlePeriodicEnergizeAuraTick(&target,&caster);assert(target.power==0);target.alive=true;target.immune=true;effect.HandlePeriodicEnergizeAuraTick(&target,&caster);assert(target.power==0);int amount=150;bool recalc=true;Script absent{nullptr};absent.CalculateAmount(&effect,amount,recalc);assert(amount==0);
}
