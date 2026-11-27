from pathlib import Path
import tempfile,subprocess,struct
r=Path(__file__).resolve().parents[3]
def method(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
inc=(r/'src/server/coa/Area52Worldforged.inc').read_text();native=(r/'src/server/coa/AscensionMysticKnightEpics.cpp').read_text()
code=r"""
#include "Area52WorldforgedCatalog.h"
#include <cassert>
#include <map>
#include <set>
#include <array>
#include <random>
using uint32=uint32_t;using int32=int32_t;using namespace Area52Worldforged;
constexpr int EFFECT_0=0,POWER_ENERGY=0,POWER_RAGE=1,POWER_MANA=2;
struct AuraEffect{};struct Aura{uint32 owner=1,stack=1;AuraEffect effect;uint32 GetCasterGUID(){return owner;}uint32 GetStackAmount(){return stack;}AuraEffect*GetEffect(int){return &effect;}};
struct Application{Aura aura;Aura*GetBase(){return &aura;}};
struct SpellInfo{uint32 Id=1464;};
struct Player;struct Unit{Player*player=nullptr;Player*ToPlayer(){return player;}};
struct Player:Unit{bool realm=true,alive=true,owned=true,matches=true;uint32 level=80;std::map<uint32,Application*>auras;std::map<uint32,int32>casts;Player(){player=this;}bool IsAlive(){return alive;}bool HasAura(uint32 id){return casts.contains(id);}uint32 GetGUID(){return 1;}uint32 GetLevel(){return level;}auto const&GetAppliedAuras(){return auras;}Aura*GetAura(uint32 id,uint32){return auras.contains(id)?&auras.at(id)->aura:nullptr;}void CastSpell(Player*,uint32 id,bool){casts[id]=1;}void CastCustomSpell(Player*,uint32 id,int32*amount,void*,void*,bool,void*,AuraEffect*){casts[id]=*amount;}void EnergizeBySpell(Player*,uint32,int32,int){assert(false);}uint32 GetCreateMana(){return 1000;}};
bool Applies(Player*p){return p&&p->realm;}
namespace AscensionMysticEnchant{bool GrantsActive(Player*p,uint32){return p->owned;}}
bool WorldforgedMatches(Player*p,SpellInfo const*,Definition const&){return p->matches;}
bool WorldforgedRegistered=false,rollResult=true;uint32 rollChance=0;
bool roll_chance_i(uint32 chance){rollChance=chance;return rollResult;}
struct RandomEngine{static std::mt19937&Instance(){static std::mt19937 e(1);return e;}};
constexpr std::array<uint32,11>WorldforgedBuffs={781000,781300,781600,781900,782200,782201,782500,782501,782800,783100,783101};
@@AMOUNT@@
@@PROC@@
@@DAMAGE@@
int main(){auto entry=Find(781616);assert(entry&&entry->chance==50&&entry->family==Family::Primal&&entry->event==EventKind::Damage);
 for(unsigned copies=1;copies<=3;++copies){Player p;Unit target;SpellInfo spell;Application a;a.aura.stack=copies;p.auras[781616]=&a;
 WorldforgedRegistered=false;Damage(&p,&target,&spell,100,1000,1000,false,false);assert(p.casts.empty());WorldforgedRegistered=true;
 rollResult=false;Damage(&p,&target,&spell,100,1000,1000,false,false);assert(p.casts.empty()&&rollChance==50);
 rollResult=true;Damage(&p,&target,&spell,100,1000,1000,false,false);assert(p.casts.at(781600)==int(24*copies)&&p.casts.at(781601)==int(136*copies)&&p.HasAura(780999));
 p.casts[781600]=999;Damage(&p,&target,&spell,100,1000,1000,false,false);assert(p.casts.at(781600)==999);
 }
 for(unsigned exclusion=0;exclusion<8;++exclusion){Player p;Unit target;SpellInfo spell;Application a;p.auras[781616]=&a;uint32 damage=1,hp=100;if(exclusion==0)p.realm=false;if(exclusion==1)p.alive=false;if(exclusion==2)p.owned=false;if(exclusion==3)p.matches=false;if(exclusion==4)a.aura.owner=2;if(exclusion==5)a.aura.stack=0;if(exclusion==6)damage=0;if(exclusion==7)hp=0;Damage(&p,&target,&spell,damage,hp,100,false,false);assert(p.casts.empty());}
}
""".replace('@@AMOUNT@@',method(inc,'int32 WorldforgedAmount(')).replace('@@PROC@@',method(inc,'void ProcWorldforged(')).replace('@@DAMAGE@@',method(native,'void Area52Worldforged::Damage(').replace('Area52Worldforged::Damage','Damage'))
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-I'+str(r/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
def rows(name):
 b=(Path('/data/dbc')/(name+'.dbc')).read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b)
 return {struct.unpack_from('<I',b,20+i*z)[0]:struct.unpack_from('<'+str(z//4)+'I',b,20+i*z) for i in range(n)}
spells=rows('Spell');durations=rows('SpellDuration')
assert spells[781617][35]==50 and spells[781617][116:118]==(781600,781601)
assert durations[spells[781600][40]][1]==12000
assert durations[spells[780999][40]][1]==20000
print('PASS actual Primal Slam dispatcher/application: 50% chance independent of 1-3 copies, buff/heal scaling, shared cooldown, unregistered startup and owner/realm/event exclusions. Matching and storage adapters used; inheritance/persistence separate.')
