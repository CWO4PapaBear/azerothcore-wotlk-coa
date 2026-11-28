from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[3];s=(r/'src/server/coa/Area52Worldforged.inc').read_text()
def method(sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
code=r"""
#include "Area52WorldforgedCatalog.h"
#include <cassert>
#include <map>
#include <set>
#include <array>
using uint32=uint32_t;using int32=int32_t;constexpr int CLASS_HERO=1,EFFECT_0=0;
struct SpellInfo{uint32 Id=781616;};struct AuraEffect{int32 amount=0;unsigned zeroes=0;void ChangeAmount(int32 x){amount=x;zeroes+=x==0;}};
struct Aura{SpellInfo*source=nullptr;uint32 stack=1,stat=0;bool removed=false;AuraEffect effect;SpellInfo*GetTriggeredByAuraSpellInfo(){return source;}uint32 GetStackAmount(){return stack;}void Remove(){removed=true;}AuraEffect*GetEffect(int){return &effect;}uint32 GetScriptValue(uint32){return stat;}void SetScriptValue(uint32,uint32 s){stat=s;}};
struct Player{bool realm=true,ready=false,owned=true;uint32 cls=1,level=80,selected=0;std::map<uint32,Aura*>auras;uint32 getClass(){return cls;}uint32 GetGUID(){return 1;}uint32 GetLevel(){return level;}Aura*GetAura(uint32 id,uint32){return auras.contains(id)&&!auras[id]->removed?auras[id]:nullptr;}};
bool Enabled=true;bool Applies(Player*p){return p&&p->realm;}uint32 WorldforgedSelectedStat(Player*p){return p->selected;}
namespace Area52Worldforged{bool OwnerReady(Player*p){return p->ready;}}
namespace AscensionMysticEnchant{bool GrantsActive(Player*p,uint32){return p->owned;}}
constexpr std::array<uint32,11>WorldforgedBuffs={781000,781300,781600,781900,782200,782201,782500,782501,782800,783100,783101};
@@AMOUNT@@
@@REFRESH@@
int main(){for(bool auraFirst:{false,true})for(uint32 copies=1;copies<=3;++copies){Player p;SpellInfo source;Aura parent,buff;parent.stack=copies;buff.source=&source;buff.stat=1;if(auraFirst)p.auras[781600]=&buff;else p.auras[781616]=&parent;RefreshWorldforged(&p);assert(!buff.removed);p.auras[781600]=&buff;p.auras[781616]=&parent;p.ready=true;RefreshWorldforged(&p);assert(buff.effect.amount==int(24*copies));RefreshWorldforged(&p);assert(buff.effect.amount==int(24*copies));p.level=40;RefreshWorldforged(&p);assert(buff.effect.amount==int(12*copies));p.selected=1;RefreshWorldforged(&p);assert(buff.stat==2&&buff.effect.zeroes==1);parent.stack=1;RefreshWorldforged(&p);assert(buff.effect.amount==12);p.owned=false;RefreshWorldforged(&p);assert(buff.removed);}
for(unsigned invalid=0;invalid<4;++invalid){Player p;p.ready=true;SpellInfo source;Aura buff,parent;buff.source=&source;p.auras={{781600,&buff},{781616,&parent}};if(invalid==0)buff.source=nullptr;if(invalid==1)source.Id=781010;if(invalid==2)p.auras.erase(781616);if(invalid==3)p.realm=false;RefreshWorldforged(&p);assert(buff.removed);}
Player p;p.ready=true;Aura buff; p.auras[781600]=&buff;p.cls=2;RefreshWorldforged(&p);assert(!buff.removed);p.cls=1;Enabled=false;RefreshWorldforged(&p);assert(!buff.removed);
}
""".replace('@@AMOUNT@@',method('int32 WorldforgedAmount(')).replace('@@REFRESH@@',method('void RefreshWorldforged('))
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-I'+str(r/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS production Primal buff refresh: both readiness orders, saved source identity, copies1-3, reduction, level/primary changes, missing/foreign ownership cleanup and mode guards. Native SQL serialization separately traced; no gameplay.')
