#include <cassert>
#include <initializer_list>
constexpr int UNIT_FIELD_COMBATREACH=1;
struct Unit{float reach=1.5f,observed=0;bool area=true;Unit*owner=nullptr;Unit*ToPlayer(){return this;}float GetFloatValue(int){return reach;}void SetFloatValue(int,float v){reach=v;}int GetGUID(){return 1;}void CastSpell(Unit*,int,bool,void*,void*,int){observed=owner->reach;}};
namespace AscensionFreePick{bool Applies(Unit*p){return p&&p->area;}}
struct Spell{int Id=1;bool area=false;bool IsAffectingArea(){return area;}};
struct Probe{Unit*owner;Unit*GetUnitOwner(){return owner;}void cast(Unit*player,Unit*dancingRuneWeapon,Unit*target,Spell*procSpell,void*aurEff){@BODY@}};
int main(){Unit player,rune,target;rune.owner=&player;Probe probe{&player};Spell spell;for(bool aoe:{false,true}){spell.area=aoe;for(float reach:{0.01f,1.5f,3.0f}){player.reach=reach;probe.cast(&player,&rune,&target,&spell,nullptr);assert(player.reach==reach);assert(rune.observed==(aoe?reach:10.0f));}}player.area=false;player.reach=1.5f;probe.cast(&player,&rune,&target,&spell,nullptr);assert(player.reach==0.01f);}
