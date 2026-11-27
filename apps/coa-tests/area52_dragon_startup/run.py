from pathlib import Path
import tempfile,subprocess,struct
r=Path(__file__).resolve().parents[3]
def method(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
native=(r/'src/server/coa/AscensionMysticEnchant.cpp').read_text();dragon=(r/'src/server/coa/Area52DragonWarrior.inc').read_text()
startup=method(native[native.index('class MysticWorld'):],'void OnStartup(').replace(' override','')
resolve=method((r/'src/server/coa/AscensionMysticEnchantCatalog.cpp').read_text(),'bool ResolveNative(')
branch=method(native,'if (Area52Worldforged::Find(spell))')
special=method(native,'if (HasEldritchEquipmentHandlers(spell) || HasArea52EpicEquipmentHandlers(spell) ||')
a=native.index('        if (!ResolveNative(spell, graph))');b=native.index('    Active = true;',a);generic=native[a:b].rsplit('    }',1)[0]
raw=Path('/data/dbc/Spell.dbc').read_bytes();_,count,fields,size,strings=struct.unpack_from('<4s4I',raw)
parent=next(struct.unpack_from('<'+str(fields)+'I',raw,20+i*size) for i in range(count) if struct.unpack_from('<I',raw,20+i*size)[0]==954252)
assert parent[71:74]==(6,6,6) and parent[95:98]==(42,4,4)
assert 'aura == SPELL_AURA_DUMMY' in native and 'node.Supported = false;' in native
code=r"""
#include "Area52WorldforgedCatalog.h"
#include "Area52DragonWarriorRules.h"
#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <string>
#include <vector>
using uint32=unsigned;constexpr unsigned SPELL_EFFECT_APPLY_AURA=6;
struct Effect{unsigned Effect=6;bool IsEffect()const{return Effect!=0;}};
struct Info{unsigned StackAmount=1;std::array<::Effect,3> Effects={{{6},{0},{0}}};};
struct Manager{std::map<unsigned,Info>infos;std::set<unsigned>procs;Info*GetSpellInfo(unsigned id){auto it=infos.find(id);return it==infos.end()?nullptr:&it->second;}void*GetSpellProcEntry(unsigned id){return procs.contains(id)?this:nullptr;}}mgr;Manager*sSpellMgr=&mgr;
bool Enabled=false,WorldforgedRegistered=false,realm=false;
std::set<std::pair<unsigned,std::string>> bindings;
bool HasAuraScriptNamed(unsigned id,char const*n){return bindings.contains({id,n});}bool HasSpellScriptNamed(unsigned id,char const*n){return bindings.contains({id,n});}
@@DRAGON@@
bool HasArea52EpicEquipmentHandlers(unsigned spell){if(Area52Worldforged::Find(spell))return Enabled&&WorldforgedRegistered;return false;}
bool HasEldritchEquipmentHandlers(unsigned){return false;}bool HasArea52HopliteHandlers(unsigned){return false;}bool HasArea52BattlemageHandlers(unsigned){return false;}bool HasArea52MarksmanHandlers(unsigned){return false;}
struct SpellNode{bool Supported=false;std::set<unsigned>Dependencies;};std::map<unsigned,SpellNode>graph;std::map<unsigned,unsigned>Connected;
@@RESOLVE@@
void LoadConnections(){Connected.clear();if(!realm)return;for(unsigned spell:{81101u,95276u,781616u,911167u,916184u,946908u,954252u,956927u,964332u}){auto info=mgr.GetSpellInfo(spell);if(!info)continue;@@BRANCH@@ @@SPECIAL@@ @@GENERIC@@}}
unsigned refreshed=0;void RefreshTestAccess(){++refreshed;}
@@STARTUP@@
int main(){OnStartup();assert(Connected.empty());realm=true;OnStartup();assert(Connected.empty());
for(unsigned id:{81101u,95276u,781616u,911167u,916184u,946908u,954252u,956927u,964332u,179u,58567u}){mgr.infos[id]={};graph[id]={true,{}};}
for(unsigned id:{95276u,781616u,916184u,956927u})mgr.infos[id].StackAmount=3;
graph[954252].Supported=false;graph[81101].Dependencies={179};graph[964332].Dependencies={58567};OnStartup();assert(Connected.size()==7&&!Connected.contains(954252)&&!Connected.contains(781616));
Enabled=true;WorldforgedRegistered=true;mgr.procs={954252,954261};mgr.infos[954250]={};mgr.infos[954256]={};OnStartup();assert(Connected.size()==8);
bindings={{954256,"aura_area52_dragon_charge"},{954288,"spell_area52_dragon_charge_impact"},{954251,"spell_area52_dragon_roar_stun"}};OnStartup();assert(Connected.size()==9);auto before=Connected;OnStartup();assert(Connected==before);assert(Connected[781616]==3&&Connected[954252]==1);
graph.erase(179);OnStartup();assert(!Connected.contains(81101));graph[179]={true,{}};OnStartup();assert(Connected.size()==9);mgr.procs.erase(954261);OnStartup();assert(!Connected.contains(954252));realm=false;OnStartup();assert(Connected.empty());assert(refreshed==10);
}
""".replace('@@DRAGON@@',method(dragon,'bool HasArea52DragonWarriorHandlers(')).replace('@@RESOLVE@@',resolve).replace('@@BRANCH@@',branch).replace('@@SPECIAL@@',special).replace('@@GENERIC@@',generic).replace('@@STARTUP@@',startup)
world=(r/'src/server/game/World/World.cpp').read_text();main=(r/'src/server/apps/worldserver/Main.cpp').read_text();assert world.index('LoadSpellInfoStore()')<world.index('LoadSpellProcs()');assert main.index('sWorld->SetInitialWorldSettings()')<main.index('sScriptMgr->OnStartup()')
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'t.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-I'+str(r/'src/server/coa'),str(p/'t.cpp'),'-o',str(p/'t')],check=True);subprocess.run([str(p/'t')],check=True)
print('PASS scoped production MysticWorld startup, Dragon readiness, generic dependency resolver and connection branches: initially absent data, missing procs/bindings, all9 ready, repeats, missing dependencies and disabled realm. Dragon parent dummy-aura exclusion is checked against mounted DBC and production graph rejection; other graph nodes use audited fixtures; not a full database world boot.')
