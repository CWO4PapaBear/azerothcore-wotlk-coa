from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[3];s=(r/'src/server/coa/AscensionHeroWarrior.cpp').read_text();a=s.index('void RefreshArea52DragonTalentHelpers(');b=s.index('{',a)+1;n=1
while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
method=s[a:b]
code=r"""
#include <cassert>
#include <array>
#include <algorithm>
#include <map>
#include <set>
#include <cstdint>
using uint32=uint32_t;struct Info{uint32 Id=0;};struct Aura{Info*source=nullptr;uint32 owner=1;Info*GetTriggeredByAuraSpellInfo(){return source;}};
struct Session{bool logout=false;bool PlayerLogout(){return logout;}};
struct Player{bool realm=true,world=true;Session session;std::set<uint32>parents,known,authorized;std::map<uint32,Aura>buffs;Session*GetSession(){return &session;}bool IsInWorld(){return world;}bool HasAura(uint32 id){return parents.contains(id);}bool HasSpell(uint32 id){return known.contains(id);}uint32 GetGUID(){return 1;}Aura*GetAura(uint32 id,uint32 owner){auto i=buffs.find(id);return i!=buffs.end()&&i->second.owner==owner?&i->second:nullptr;}void RemoveAurasDueToSpell(uint32 id,uint32 owner){if(GetAura(id,owner))buffs.erase(id);}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->realm;}bool IsTalentRank(Player*,uint32){return true;}bool CanLearnSpell(Player*p,uint32 id,bool,bool){return p->authorized.contains(id);}}
@@METHOD@@
int main(){for(auto pair:std::array<std::pair<uint32,uint32>,9>{{{57518,46867},{57519,56611},{57520,56612},{29841,29834},{29842,29838},{829839,829838},{46916,46913},{52437,29723},{820504,820503}}}){
 auto[helper,parent]=pair;Player p;p.buffs[helper]={};p.world=false;RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.contains(helper));p.authorized={parent};p.world=true;RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.contains(helper));p.authorized.clear();p.parents={parent};p.world=true;RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.empty());p.buffs[helper]={};p.authorized={parent};RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.contains(helper));p.authorized.clear();p.parents.clear();RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.empty());
 p.buffs[helper]={};p.known={helper};RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.contains(helper));p.known.clear();Info independent{999999};p.buffs[helper].source=&independent;p.parents={999999};RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.contains(helper));p.parents.clear();RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.empty());
 p.buffs[helper]={nullptr,2};RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.contains(helper));
 p.buffs[helper]={};p.realm=false;RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.contains(helper));p.realm=true;p.session.logout=true;RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.contains(helper));
 }
 Player p;p.buffs={{57518,{}},{57519,{}},{57520,{}}};p.parents={46867};p.authorized={46867};RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.size()==1&&p.buffs.contains(57518));p.parents={56611};p.authorized={56611};RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.empty());
 for(unsigned parent:{46913u,46914u,46915u}){p.buffs[46916]={};p.parents={parent};p.authorized={parent};RefreshArea52DragonTalentHelpers(&p);assert(p.buffs.contains(46916));}
 RefreshArea52DragonTalentHelpers(nullptr);
}
""".replace('@@METHOD@@',method)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS actual finite-helper cleanup after rank removal/downgrade, saved helpers without provenance, independent known/source grants, foreign ownership and logout/realm exclusions. Current selection authorization protects saved helpers before a missing parent aura restores.')
