from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[3];s=(r/'src/server/coa/AscensionHeroWarrior.cpp').read_text();a=s.index('void RefreshArea52TwoHandMastery(');b=s.index('{',a)+1;n=1
while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
method=s[a:b]
code=r"""
#include <cassert>
#include <set>
#include <initializer_list>
#include <cstdint>
using uint32=uint32_t;constexpr int INVENTORY_SLOT_BAG_0=0,EQUIPMENT_SLOT_OFFHAND=16;
struct Session{bool logout=false;bool PlayerLogout(){return logout;}};
struct Player{bool realm=true,world=false,alive=true,feral=false,twohand=true,offhand=false;unsigned casts=0;Session session;std::set<uint32>parents,helpers,known,authorized={29623,829633};bool HasSpell(uint32 id){return known.contains(id);}Session*GetSession(){return &session;}bool IsInWorld(){return world;}bool IsAlive(){return alive;}bool IsInFeralForm(){return feral;}bool IsTwoHandUsed(){return twohand;}void*GetItemByPos(int,int){return offhand?this:nullptr;}bool HasAura(uint32 id){return parents.contains(id);}void RemoveAurasDueToSpell(uint32 id,uint32){helpers.erase(id);}uint32 GetGUID(){return 1;}void*GetAura(uint32 id,uint32){return helpers.contains(id)?this:nullptr;}void CastSpell(Player*,uint32 id,bool){helpers.insert(id);++casts;}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->realm;}bool CanLearnSpell(Player*p,uint32 id,bool,bool){return p->authorized.contains(id);}}
@@METHOD@@
int main(){for(bool auraFirst:{false,true}){Player p;if(auraFirst)p.parents={29623};else p.world=true;RefreshArea52TwoHandMastery(&p);assert(p.helpers.empty());p.world=true;p.parents={29623};RefreshArea52TwoHandMastery(&p);assert(p.helpers==std::set<uint32>{829623});RefreshArea52TwoHandMastery(&p);assert(p.casts==1);
 p.parents={829633};RefreshArea52TwoHandMastery(&p);assert(p.helpers==std::set<uint32>{829634});p.parents={29623};RefreshArea52TwoHandMastery(&p);assert(p.helpers==std::set<uint32>{829623});
 for(unsigned exclusion=0;exclusion<4;++exclusion){p.alive=exclusion!=0;p.feral=exclusion==1;p.twohand=exclusion!=2;p.offhand=exclusion==3;RefreshArea52TwoHandMastery(&p);assert(p.helpers.empty());p.alive=true;p.feral=false;p.twohand=true;p.offhand=false;RefreshArea52TwoHandMastery(&p);assert(p.helpers==std::set<uint32>{829623});}
 p.parents.clear();RefreshArea52TwoHandMastery(&p);assert(p.helpers.empty());p.parents={829633};p.session.logout=true;RefreshArea52TwoHandMastery(&p);assert(p.helpers.empty());p.session.logout=false;p.helpers={829623,829634};RefreshArea52TwoHandMastery(&p);assert(p.helpers==std::set<uint32>{829634});
 }Player stale;stale.world=true;stale.parents={29623,829633};stale.authorized={29623};RefreshArea52TwoHandMastery(&stale);assert(stale.helpers==std::set<uint32>{829623});stale.authorized.clear();RefreshArea52TwoHandMastery(&stale);assert(stale.helpers.empty());Player independent;independent.world=true;independent.helpers={829623};independent.known={829623};RefreshArea52TwoHandMastery(&independent);assert(independent.helpers.contains(829623));Player other;other.world=true;other.parents={29623};other.realm=false;RefreshArea52TwoHandMastery(&other);assert(other.helpers.empty());RefreshArea52TwoHandMastery(nullptr);
}
""".replace('@@METHOD@@',method)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS actual Two-Handed Mastery helper sync: both readiness orders, ranks1/2/downgrade/removal, missing/stale helpers, equipment/feral/death/logout exclusions and other realm isolation. IsTwoHandUsed uses existing shared native equipment predicate.')
