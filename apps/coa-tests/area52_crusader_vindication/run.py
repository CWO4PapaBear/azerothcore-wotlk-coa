from pathlib import Path
import subprocess,tempfile,sys,struct
ROOT=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(ROOT/'apps/coa-tests'))
from client_data import dbc_dir
def block(s,m):
 a=s.index(m);b=s.index('{',a);e=b+1;n=1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
s=(ROOT/'src/server/coa/Area52CrusaderVindication.inc').read_text();hit=block(s,'void Area52VindicatedHit(');remove=block(s,'void Area52VindicatedRemove(')
calc=(ROOT/'src/server/game/Spells/SpellInfo.cpp').read_text();a=calc.index('    float basePointsPerLevel = RealPointsPerLevel;',calc.index('int32 SpellEffectInfo::CalcValue('));b=calc.index('    float value = float(basePoints);',a);calc=calc[a:b]
b=(dbc_dir()/'Spell.dbc').read_bytes();_,n,f,size,_=struct.unpack_from('<4s4I',b);row=next(x for i in range(n) if (x:=struct.unpack_from('<'+str(f)+'I',b,20+i*size))[0]==200067)
assert row[71]==6 and row[95]==99 and row[72]==0 and row[40]==1
b=(dbc_dir()/'SpellDuration.dbc').read_bytes();_,n,f,size,_=struct.unpack_from('<4s4I',b);duration=next(x for i in range(n) if (x:=struct.unpack_from('<'+str(f)+'i',b,20+i*size))[0]==1);assert duration[1]==10000
levelscale=struct.unpack('<f',struct.pack('<I',row[77]))[0]
code=r"""
#include <cassert>
#include <algorithm>
#include <cstdint>
#include <set>
using uint32=uint32_t;using int32=int32_t;using uint8=uint8_t;using AuraRemoveMode=int;
constexpr unsigned SPELL_SCHOOL_MASK_HOLY=2,SPELL_MISS_NONE=0,ALLSPELLHOOK_ON_HIT_RESULT=1,UNITHOOK_ON_PERIODIC_DAMAGE_RESULT=2,UNITHOOK_ON_AURA_REMOVE=3;
struct AllSpellScript{AllSpellScript(char const*,std::initializer_list<unsigned>){}};
struct UnitScript{UnitScript(char const*,bool,std::initializer_list<unsigned>){}};
struct Player;struct Unit{virtual Player*ToPlayer(){return nullptr;}bool vindication=false;bool HasAura(unsigned){return vindication;}unsigned level=80;unsigned GetLevel()const{return level;}};
struct Player:Unit{bool area=true;std::set<unsigned>auras,known;unsigned casts=0,removed=0;Player*ToPlayer()override{return this;}bool HasAura(unsigned id){return auras.contains(id);}bool HasSpell(unsigned id){return known.contains(id);}unsigned GetGUID(){return 1;}void CastSpell(Player*p,unsigned id,bool triggered){assert(p==this&&id==200067&&triggered);++casts;}void RemoveAurasDueToSpell(unsigned id,unsigned owner){assert(id==200067&&owner==1);++removed;}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct SpellInfo{unsigned Id=1,school=2,MaxLevel=80,BaseLevel=1,SpellLevel=1;unsigned GetSchoolMask()const{return school;}};
struct Aura{unsigned id=209452;unsigned GetId(){return id;}};struct AuraApplication{Aura aura;Aura*GetBase(){return &aura;}};
struct Spell{Unit*caster;SpellInfo*info;Unit*GetCaster(){return caster;}SpellInfo const*GetSpellInfo(){return info;}};
@@HIT@@
@@REMOVE@@
int irand(int,int){assert(false);return 0;}
int Value(Unit const*caster){SpellInfo info;auto*_spellInfo=&info;float RealPointsPerLevel=@@SCALE@@f;int BasePoints=0,DieSides=1;int const*bp=nullptr;@@CALC@@return basePoints;}
int main(){Player p;Unit target;SpellInfo info;AuraApplication app;target.vindication=true;
for(unsigned rank:{9452u,26016u,209452u}){p.auras={rank};p.casts=0;Area52VindicatedHit(&p,&target,100,&info);assert(p.casts==(rank==209452));}
p.auras={209452};p.casts=0;target.vindication=false;Area52VindicatedHit(&p,&target,100,&info);assert(!p.casts);target.vindication=true;info.school=1;Area52VindicatedHit(&p,&target,100,&info);assert(!p.casts);info.school=2;Area52VindicatedHit(&p,&target,0,&info);assert(!p.casts);info.Id=200067;Area52VindicatedHit(&p,&target,100,&info);assert(!p.casts);info.Id=1;p.area=false;Area52VindicatedHit(&p,&target,100,&info);assert(!p.casts);p.area=true;
Area52VindicationCasts casts;Area52VindicationAuras auras;Spell spell{&p,&info};
casts.OnSpellHitResult(&spell,&target,1,100,0,false);assert(!p.casts);
casts.OnSpellHitResult(&spell,&target,0,0,100,false);assert(!p.casts);
casts.OnSpellHitResult(nullptr,&target,0,100,0,false);assert(!p.casts);
casts.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==1);
auras.OnPeriodicDamageResult(&target,&p,0,&info);assert(p.casts==1);
auras.OnPeriodicDamageResult(&target,&p,100,&info);assert(p.casts==2);
info.school=1;auras.OnPeriodicDamageResult(&target,&p,100,&info);assert(p.casts==2);info.school=2;
auras.OnPeriodicDamageResult(&target,&p,100,&info);assert(p.casts==3);
Area52VindicatedRemove(&p,&app);assert(p.removed==1);p.known.insert(200067);Area52VindicatedRemove(&p,&app);assert(p.removed==1);p.known.clear();app.aura.id=9452;Area52VindicatedRemove(&p,&app);assert(p.removed==1);app.aura.id=209452;p.area=false;Area52VindicatedRemove(&p,&app);assert(p.removed==1);
for(unsigned level:{1u,20u,40u,60u,80u,90u}){p.level=level;assert(Value(&p)==1+int((std::min(level,80u)-1)*@@SCALE@@f));}p.level=80;assert(Value(&p)==123);
}
""".replace('@@HIT@@',s.replace(' override','')).replace('@@REMOVE@@','').replace('@@SCALE@@',str(levelscale)).replace('@@CALC@@',calc)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS: actual Vindicated hit/removal handlers, rank and independent grant controls, native CalcValue level arithmetic and DBC AP-only 10-second helper. Saved restoration runs separately.')
