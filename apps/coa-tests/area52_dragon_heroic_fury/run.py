from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[3]
s=(r/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text();a=s.index('        if (sSpellMgr->GetSpellTwinSource(GetId()) == 60970');b=s.index('\n        switch (',a);current=s[a:b]
base=subprocess.check_output(['git','show','HEAD:src/server/game/Spells/Auras/SpellAuras.cpp'],cwd=r,text=True);a=base.index('        if (sSpellMgr->GetSpellTwinSource(GetId()) == 60970');b=base.index('\n        switch (',a);base=base[a:b]
code=r"""
#include <cassert>
#include <map>
#include <set>
#include <vector>
using uint32=unsigned;
struct Player{bool realm=true,player=true;std::set<unsigned>cooldowns;std::map<unsigned,unsigned>replacement;bool IsPlayer(){return player;}Player*ToPlayer(){return this;}void RemoveSpellCooldown(unsigned n,bool){cooldowns.erase(n);}unsigned GetTemporarySpellReplacement(unsigned n){return replacement.contains(n)?replacement[n]:n;}};
namespace AscensionFreePick{bool Applies(Player*p){return p->realm;}}
struct Manager{unsigned GetSpellTwinSource(unsigned n){return n==1060970?60970:n;}std::vector<unsigned>GetSpellAndRelatives(unsigned id){return {id,id+1000000};}}mgr;auto*sSpellMgr=&mgr;
struct Script{unsigned id=60970;unsigned GetId(){return id;}void old(Player*target){@@OLD@@}void now(Player*target){@@NEW@@}};
int main(){Script s;Player p;p.cooldowns={20252,954876,954256,1020252,1954876,11111};p.replacement[20252]=954256;s.old(&p);assert(!p.cooldowns.contains(20252)&&p.cooldowns.contains(954876)&&p.cooldowns.contains(954256));
for(unsigned id:{60970u,1060970u}){s.id=id;p.cooldowns={20252,954876,954256,1020252,1954876,11111};s.now(&p);assert(p.cooldowns==std::set<unsigned>{11111});s.now(&p);assert(p.cooldowns==std::set<unsigned>{11111});}
p.realm=false;p.cooldowns={20252,954876,954256,1020252,1954876,11111};s.now(&p);assert(!p.cooldowns.contains(20252)&&!p.cooldowns.contains(1020252)&&p.cooldowns.contains(954876)&&p.cooldowns.contains(954256));
p.realm=true;p.replacement.clear();p.cooldowns={20252,954256,11111};s.now(&p);assert(p.cooldowns==std::set<unsigned>({954256,11111}));s.id=123;p.cooldowns={20252,954876};s.now(&p);assert(p.cooldowns.size()==2);}
""".replace('@@OLD@@',base).replace('@@NEW@@',current)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'t.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'t.cpp'),'-o',str(p/'t')],check=True);subprocess.run([str(p/'t')],check=True)
print('PASS extracted native aura application: original missing replacement/Siegebreaker reset reproduced; candidate original/twins/authorized replacement, repeats, unrelated cooldowns and other realms. Replacement ownership API independently qualified.')
