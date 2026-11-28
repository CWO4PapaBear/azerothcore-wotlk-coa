from pathlib import Path
import subprocess,tempfile,sys,struct
ROOT=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(ROOT/'apps/coa-tests'))
from client_data import dbc_dir
def block(s,m):
 a=s.index(m);b=s.index('{',a);e=b+1;n=1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
s=(ROOT/'src/server/coa/Area52ConsecratedGround.inc').read_text();amount=block(s,'int Area52ConsecratedGroundAmount(');summon=block(s,'void IsSummonedBy(').replace(' override','')
u=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text();a=u.index('            if (auraType == SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE)');b=u.index('            if (checkHarvest',a);branch=u[a:b]
b=(dbc_dir()/'SpellRadius.dbc').read_bytes();_,n,f,size,_=struct.unpack_from('<4s4I',b);radius=next(struct.unpack_from('<f',b,20+i*size+4)[0] for i in range(n) if struct.unpack_from('<I',b,20+i*size)[0]==14);assert radius==8
b=(dbc_dir()/'SpellDuration.dbc').read_bytes();_,n,f,size,_=struct.unpack_from('<4s4I',b);duration=next(struct.unpack_from('<i',b,20+i*size+4)[0] for i in range(n) if struct.unpack_from('<I',b,20+i*size)[0]==31);assert duration==8000
code=r"""
#include <cassert>
#include <algorithm>
#include <cmath>
#include <vector>
using int32=int;constexpr int SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE=137,REACT_PASSIVE=0,UNIT_FLAG_NON_ATTACKABLE=1,UNIT_FLAG_NOT_SELECTABLE=2,UNIT_FLAG_DISABLE_MOVE=4;
struct Player;struct Creature;struct Unit{unsigned guid=1;virtual Player*ToPlayer(){return nullptr;}virtual Creature*ToCreature(){return nullptr;}unsigned GetGUID()const{return guid;}};using WorldObject=Unit;
struct Player:Unit{bool area=true;Player*ToPlayer()override{return this;}unsigned GetFaction(){return 35;}};
struct Creature:Unit{unsigned entry=300055,ownerGuid=0,faction=0,flags=0,cast=0;Unit*owner=nullptr;Creature*ToCreature()override{return this;}Unit*GetOwner(){return owner;}unsigned GetEntry(){return entry;}void SetOwnerGUID(unsigned g){ownerGuid=g;}void SetFaction(unsigned f){faction=f;}void SetReactState(unsigned){}void SetUnitFlag(unsigned f){flags=f;}void CastSpell(Creature*t,unsigned id,bool b){assert(t==this&&b);cast=id;}};
struct AuraEffect{unsigned id=965903;Unit*caster;unsigned GetId()const{return id;}Unit*GetCaster()const{return caster;}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
@@AMOUNT@@
struct Probe{Creature*me;@@SUMMON@@};
struct Target:Unit{float Aggregate(std::vector<AuraEffect*>effects){int consecrated=0;int auraType=137;float multiplier=1.1f;for(auto*aurEff:effects){@@BRANCH@@}if(consecrated)multiplier*=1+consecrated/100.f;return multiplier;}};
bool close(float a,float b){return std::abs(a-b)<0.000001f;}
int main(){Player owner;owner.guid=1;Player other;other.guid=2;Creature first;first.owner=&owner;Creature second;second.owner=&other;AuraEffect a{965903,&first},b{965903,&second};Target target;target.guid=1;
assert(Area52ConsecratedGroundAmount(&a,&target)==12);assert(Area52ConsecratedGroundAmount(&b,&target)==5);assert(close(target.Aggregate({&a,&b}),1.232f));assert(close(target.Aggregate({&b}),1.155f));assert(close(target.Aggregate({}),1.1f));assert(close(target.Aggregate({&a,&a}),1.232f));target.guid=3;assert(close(target.Aggregate({&a,&b}),1.155f));owner.area=false;assert(!Area52ConsecratedGroundAmount(&a,&target));owner.area=true;first.entry=1;assert(!Area52ConsecratedGroundAmount(&a,&target));first.entry=300055;Probe p{&first};p.IsSummonedBy(&owner);assert(first.ownerGuid==1&&first.faction==35&&first.flags==7&&first.cast==965903);}
""".replace('@@AMOUNT@@',amount).replace('@@SUMMON@@',summon).replace('@@BRANCH@@',branch)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS: actual ground summon ownership and native aggregate branch, owner12/allies5, strongest overlap, removal fallback, otherrealm/template exclusion; actualDBC8yard/8second. Native UnitAura target map/removal is separate integration gate.')
