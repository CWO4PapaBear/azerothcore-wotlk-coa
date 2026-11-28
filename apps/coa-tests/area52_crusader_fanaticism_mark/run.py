from pathlib import Path
import tempfile,subprocess
ROOT=Path(__file__).resolve().parents[3]
def block(s,m):
 a=s.index(m);b=s.index('{',a);n=1;e=b+1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
s=(ROOT/'src/server/coa/Area52CrusaderFanaticism.inc').read_text();seal=block(s,'bool Area52FanaticismSeal(');mark=block(s,'bool Area52FanaticismMarkAffected(')
u=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text();checks=[]
for actor in ('caster','attacker'):
 a=u.index('            if (aurEff->GetId() == 31882 && AscensionFreePick::Applies('+actor+'->ToPlayer()))');b=u.index(';',a)+1
 checks.append(u[a:b].replace(actor,'caster'))
assert checks[0]==checks[1]
code=r'''
#include <cassert>
#include <cstdint>
#include <map>
#include <vector>
using uint32=uint32_t;constexpr unsigned SPELLFAMILY_PALADIN=10;
struct flag96{unsigned a,b,c;flag96(unsigned x,unsigned y,unsigned z):a(x),b(y),c(z){}};
struct SpellInfo{unsigned Id=0,family=10,a=0,b=0,c=0;bool IsAffected(unsigned f,flag96 m)const{return family==f&&((a&m.a)||(b&m.b)||(c&m.c));}};
struct Player;struct Unit{virtual Player*ToPlayer(){return nullptr;}unsigned guid=1;unsigned GetGUID(){return guid;}};
struct Player:Unit{bool area=true;std::map<unsigned,std::vector<unsigned>>origins;Player*ToPlayer()override{return this;}auto GetSpellInteractionOrigins(unsigned id){return origins[id];}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct Manager{std::map<unsigned,SpellInfo>rows;SpellInfo const*GetSpellInfo(unsigned id){auto i=rows.find(id);return i==rows.end()?nullptr:&i->second;}}manager;auto*sSpellMgr=&manager;
struct AuraEffect{unsigned id=31882,caster=1;bool fallback=false;unsigned GetId(){return id;}unsigned GetCasterGUID(){return caster;}bool IsAffectedOnSpell(SpellInfo const*){return fallback;}};
@@SEAL@@
@@MARK@@
bool Predicate(Unit*caster,SpellInfo const*spellProto,AuraEffect*aurEff){@@CHECK@@ return aurEff->GetCasterGUID()==caster->GetGUID()&&aurEff->IsAffectedOnSpell(spellProto);}
int main(){Player p;AuraEffect aura;SpellInfo direct{20424,10,0x2000000,0,0},dot{31803,10,0,0x800,0},other{35395,10,0,0x8000,0},replacement{900001,0,0,0,0};manager.rows[direct.Id]=direct;manager.rows[dot.Id]=dot;
assert(Predicate(&p,&direct,&aura));assert(Predicate(&p,&dot,&aura));assert(!Predicate(&p,&other,&aura));assert(!Predicate(&p,&replacement,&aura));p.origins[replacement.Id]={123,dot.Id};assert(Predicate(&p,&replacement,&aura));aura.caster=2;assert(!Predicate(&p,&replacement,&aura));aura.caster=1;p.area=false;assert(!Predicate(&p,&replacement,&aura));aura.fallback=true;assert(Predicate(&p,&replacement,&aura));p.area=true;aura.id=123;assert(Predicate(&p,&other,&aura));aura.fallback=false;assert(!Predicate(&p,&direct,&aura));}
'''.replace('@@SEAL@@',seal).replace('@@MARK@@',mark).replace('@@CHECK@@',checks[0])
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS: both actual native mark predicates, own caster, direct and periodic seals, recursive-origin lookup, unrelated spells, other auras and other-realm fallback.')
