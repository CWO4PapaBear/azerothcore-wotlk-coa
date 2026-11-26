#include <cassert>
#include <cmath>
#include <initializer_list>
using WeaponAttackType=int;using int32=int;
constexpr int RANGED_ATTACK=2,SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE=1,SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE=2,SPELL_AURA_DEFLECT_SPELLS=3,UNIT_STATE_STUNNED=4,SPELL_MISS_DEFLECT=5,SPELL_MISS_NONE=0;
struct Player{float GetMissPercentageFromDefence()const{return 0;}};
struct Unit{bool stun=false,front=true,ignore=true;int deflect=100,melee=-200;Player player;Player const*ToPlayer()const{return &player;}bool HasUnitState(int){return stun;}bool HasInArc(double,Unit*){return front;}bool HasIgnoreHitDirectionAura(){return ignore;}int GetTotalAuraModifier(int id)const{return id==3?deflect:id==2?melee:0;}float GetUnitMissChance(WeaponAttackType)const;int hit(Unit*victim,int rand){int tmp=0;@DEFLECT@ return SPELL_MISS_NONE;}};
@MISS@
int main(){Unit attacker,victim;assert(victim.GetUnitMissChance(0)==205);for(bool front:{false,true}){victim.front=front;for(int roll=0;roll<10000;++roll)assert(attacker.hit(&victim,roll)==SPELL_MISS_DEFLECT);}victim.stun=true;assert(attacker.hit(&victim,0)==SPELL_MISS_NONE);victim.stun=false;victim.ignore=false;victim.front=false;assert(attacker.hit(&victim,0)==SPELL_MISS_NONE);victim.deflect=0;assert(attacker.hit(&victim,9999)==SPELL_MISS_NONE);}
