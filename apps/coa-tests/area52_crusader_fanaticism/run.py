from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def block(s,m):
 a=s.index(m);b=s.index('{',a);e=b+1;n=1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
s=(ROOT/'src/server/coa/Area52CrusaderFanaticism.inc').read_text()
a=block(s,'void OnSpellCalculatedTarget(').replace(' override','');b=block(s,'void OnSpellHitResult(').replace(' override','')
periodic=block(s,'uint32 Area52FanaticismPeriodicBonus(')
code=r'''
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <vector>
using uint32=uint32_t;using uint8=uint8_t;using int32=int32_t;using int64=int64_t;
constexpr int SPELL_MISS_NONE=0,PLAYERSPELL_REMOVED=1,FanaticismDebuff=31882;
struct Player;struct SpellInfo{unsigned Id;};struct Entry{int State=0;};
struct Unit{bool sentence=false,mark=false,creature=true,owned=false;virtual Player*ToPlayer(){return nullptr;}bool IsCreature(){return creature;}Unit*GetCharmerOrOwnerPlayerOrPlayerItself(){return owned?this:nullptr;}bool HasAura(int,unsigned){return mark;}};
struct Player:Unit{bool area=true;std::set<unsigned>auras;std::map<unsigned,Entry*>known;std::map<unsigned,int>cd;std::vector<unsigned>casts;Player*ToPlayer()override{return this;}bool HasAura(unsigned id){return auras.contains(id);}unsigned GetGUID(){return 1;}bool MatchesSpellInteraction(unsigned a,unsigned b){return a==b;}void CastSpell(Unit*,unsigned id,bool){casts.push_back(id);}auto const&GetSpellMap(){return known;}int GetSpellCooldownDelay(unsigned id){return cd[id];}void ModifySpellCooldown(unsigned id,int amount){cd[id]+=amount;}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct Spell{Player*p;SpellInfo info;Unit*GetCaster(){return p;}SpellInfo const*GetSpellInfo(){return &info;}};struct TargetInfo{int damage=1000,damageBeforeTakenMods=800;};
bool Area52FanaticismSentence(Player*,Unit*t){return t->sentence;}bool Area52FanaticismJudgement(Player*,SpellInfo const*i){return i->Id==54158;}bool Area52FanaticismSeal(Player*,SpellInfo const*i){return i->Id==20424;}
@@PERIODIC@@
struct Probe{
@@A@@
@@B@@
};
int main(){Probe p;Player owner;Unit target;Spell spell{&owner,{35395}};TargetInfo hit;
for(unsigned rank:{31879u,31880u,31881u}){owner.auras={rank};target.sentence=true;for(unsigned id:{35395u,54158u,20424u}){spell.info.Id=id;hit={1000,800};p.OnSpellCalculatedTarget(&spell,&target,hit);unsigned pct=rank==31879?7:rank==31880?14:20;assert(hit.damage==int(1000*(100+pct)/100));assert(hit.damageBeforeTakenMods==int(800*(100+pct)/100));}spell.info.Id=123;hit={1000,800};p.OnSpellCalculatedTarget(&spell,&target,hit);assert(hit.damage==1000);}
for(unsigned rank:{31879u,31880u,31881u}){owner.auras={rank};target.sentence=true;spell.info.Id=20424;assert(Area52FanaticismPeriodicBonus(&owner,&target,&spell.info)==(rank==31879?7u:rank==31880?14u:20u));target.sentence=false;assert(!Area52FanaticismPeriodicBonus(&owner,&target,&spell.info));target.sentence=true;spell.info.Id=35395;assert(!Area52FanaticismPeriodicBonus(&owner,&target,&spell.info));}
owner.auras={31881};target.sentence=false;spell.info.Id=35395;p.OnSpellHitResult(&spell,&target,0,100,0,false);assert(owner.casts==std::vector<unsigned>{31882});owner.casts.clear();Entry e;owner.known[954817]=&e;owner.cd[954817]=30000;spell.info.Id=54158;target.mark=true;p.OnSpellHitResult(&spell,&target,0,100,0,false);assert(owner.casts==std::vector<unsigned>{31890}&&owner.cd[954817]==20000);
for(int mode=0;mode<5;++mode){owner.casts.clear();target.creature=mode!=0;target.owned=mode==1;target.mark=mode!=2;owner.area=mode!=3;p.OnSpellHitResult(&spell,&target,0,mode==4?0:100,0,false);assert(owner.casts.empty());}
}
'''.replace('@@PERIODIC@@',periodic).replace('@@A@@',a).replace('@@B@@',b)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS: actual Fanaticism callbacks, all rank percentages, direct ability scope, mark, monster-only cooldown and speed, negative controls. Recursive family resolution and periodic seals are separate gates.')
