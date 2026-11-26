#include <cassert>
#include <cmath>
#include <initializer_list>
constexpr int ITEM_SUBCLASS_WEAPON_DAGGER=15;
void AddPct(float&v,float pct){v*=1+pct/100;}
struct Item{struct Template{int SubClass=0;}info;Template*GetTemplate(){return &info;}};
struct Player{bool area=true,titan=false;Item weapons[2];Player*ToPlayer(){return this;}Item*GetWeaponForAttack(int hand,bool){return &weapons[hand];}bool HasSpell(int id){assert(id==46917);return titan;}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct Info{unsigned Id=57841;};
float calculate(Player*m_caster,Info*m_spellInfo,int m_attackType){float totalDamagePercentMod=1;@BLOCK@ return 130*totalDamagePercentMod;}
int main(){Player player;Info info;for(int hand:{0,1}){info.Id=hand?57842:57841;assert(std::abs(calculate(&player,&info,hand)-130)<0.001);player.weapons[hand].info.SubClass=15;assert(std::abs(calculate(&player,&info,hand)-195)<0.001);player.titan=true;assert(std::abs(calculate(&player,&info,hand)-146.25)<0.001);player.weapons[hand].info.SubClass=0;assert(std::abs(calculate(&player,&info,hand)-97.5)<0.001);player.titan=false;}player.area=false;player.titan=true;player.weapons[0].info.SubClass=15;assert(calculate(&player,&info,0)==130);player.area=true;info.Id=1;assert(calculate(&player,&info,0)==130);}
