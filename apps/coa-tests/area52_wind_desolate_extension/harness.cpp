#include <algorithm>
#include <cassert>
#include <map>
#include <set>
using uint32=unsigned;using uint8=unsigned char;using int32=int;
constexpr int SPELL_MISS_NONE=0;
struct Player;
struct Aura {int duration=8000,max=8000,timer=1357;int GetDuration(){return duration;}int GetMaxDuration(){return max;}void SetDuration(int n){duration=n;}void SetMaxDuration(int n){max=n;}};
struct Unit {Player* player=nullptr;std::map<unsigned,Aura> dots;Player*ToPlayer(){return player;}Aura*GetAura(unsigned id,unsigned owner){auto i=dots.find(owner);return id==300065&&i!=dots.end()?&i->second:nullptr;}};
struct Player:Unit {bool area=true;std::set<unsigned> auras{997082};Player(){player=this;}bool HasAura(unsigned n)const{return auras.contains(n);}unsigned GetGUID(){return 7;}};
namespace AscensionFreePick {bool Applies(Player const*p){return p&&p->area;}}
struct SpellInfo {unsigned Id=997083;};
struct Spell {Player*caster;SpellInfo info;std::map<unsigned,unsigned> values;Unit*GetCaster(){return caster;}SpellInfo const*GetSpellInfo(){return &info;}unsigned GetScriptValue(unsigned k){return values[k];}void SetScriptValue(unsigned k,unsigned v){values[k]=v;}};
struct Manager {unsigned GetFirstSpellInChain(unsigned n){return n==997083||(n>=997085&&n<=997097)?997083:n;}} manager;Manager*sSpellMgr=&manager;
@ENABLED@
struct Probe {@HIT@};
int main(){Probe p;Player caster;Unit target;target.dots[7]={};target.dots[8]={};
for(unsigned rank=0;rank<14;++rank){target.dots[7]={};Spell spell{&caster};spell.info.Id=rank?997084+rank:997083;p.OnSpellHitResult(&spell,&target,0,10,0,false);assert(target.dots[7].duration==10000);assert(target.dots[7].timer==1357);assert(target.dots[8].duration==8000);p.OnSpellHitResult(&spell,&target,0,10,0,false);assert(target.dots[7].duration==10000);}
for(int remaining:{1,7999,8000,10000,11000,12000}){target.dots[7].duration=remaining;Spell spell{&caster};p.OnSpellHitResult(&spell,&target,0,1,0,false);assert(target.dots[7].duration==std::min(12000,remaining+2000));assert(target.dots[7].timer==1357);}
for(int mode=0;mode<7;++mode){target.dots[7]={};Spell spell{&caster};caster.area=mode!=0;if(mode==1)caster.auras.clear();if(mode==2)caster.auras.insert(978231);if(mode==3)spell.info.Id=403;if(mode==6)target.dots[7].duration=0;int before=target.dots[7].duration;p.OnSpellHitResult(&spell,&target,mode==4?1:0,mode==5?0:1,0,false);assert(target.dots[7].duration==before);caster.auras={997082};}
caster.area=true;target.dots.clear();Spell absent{&caster};p.OnSpellHitResult(&absent,&target,0,1,0,false);assert(target.dots.empty());p.OnSpellHitResult(nullptr,&target,0,1,0,false);
}
