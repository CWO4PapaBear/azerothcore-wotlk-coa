#include <algorithm>
#include <cassert>
#include <cmath>
#include <initializer_list>
using int32=int;using T=float;
enum{SPELLMOD_FLAT=107,SPELLMOD_PCT=108,SPELLMOD_CRITICAL_CHANCE=7,SPELLMOD_THREAT=2,SPELLMOD_RANGE=3,SPELLMOD_CASTING_TIME=10,SPELLMOD_DURATION=1,SPELLMOD_GLOBAL_COOLDOWN=21,SPELLMOD_DAMAGE=0,SPELLMOD_DOT=22,SPELLMOD_COST=14};
struct Aura{bool IsUsingCharges(){return false;}int GetStackAmount(){return 1;}};
struct SpellModifier{int type,value,spellId,op;Aura*ownerAura=nullptr;};
namespace AscensionFreePick{bool Applies(void*){return true;}}
struct Unit{unsigned healthPct=100;bool HealthBelowPct(unsigned n) const{return healthPct<n;}bool IsPlayer(){return false;}};
bool AuraAlsoModifiesCastingTime(Aura*){return false;}
float CalculatePct(float base,float n){return base*n/100;}
bool close(float a,float b){return std::abs(a-b)<.0002;}
struct Player{bool MatchesSpellInteraction(int spell,int original){return spell==original;}bool HasSpellModApplied(SpellModifier*,void*){return true;}void ApplyModToSpell(SpellModifier*,void*){}
float calc(int op,float basevalue,std::initializer_list<SpellModifier> mods,int spellId=0,Unit*victim=nullptr){float totalmul=1;int totalflat=0,shadowReach=0,similarRange=0;void*spell=nullptr;bool temporaryPet=false;@LAMBDA@ for(auto mod:mods)calculateSpellMod(&mod);@END@ return basevalue;}};
int main(){Player player;@TESTS@}
