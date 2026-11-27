from pathlib import Path
import tempfile,subprocess,struct
r=Path(__file__).resolve().parents[3]
def method(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
native=(r/'src/server/scripts/Spells/spell_dk.cpp').read_text();aura=(r/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text();start=aura.index('        bool secondWindBaseHealth');end=aura.index('    }',start);tick=aura[start:end]
heal=method(native[native.index('class spell_dk_improved_blood_presence_triggered'):],'void HandleProc(')
raw=Path('/data/dbc/Spell.dbc').read_bytes();_,count,fields,size,strings=struct.unpack_from('<4s4I',raw)
rows={struct.unpack_from('<I',raw,20+i*size)[0]:struct.unpack_from('<'+str(fields)+'I',raw,20+i*size) for i in range(count)}
for rank,id in enumerate([29841,29842,829839],1):
 x=rows[id];assert x[71:73]==(6,6) and x[95:97]==(24,20) and x[110:112]==(1,1)
 assert x[80]+x[74]==rank*30 and x[81]+x[75]==rank*2
assert rows[63611][96]==42 and rows[48266][80]+rows[48266][74]==10
code=r"""
#include <algorithm>
#include <cassert>
#include <cstdint>
using uint32=unsigned;using int32=int;constexpr int BLOOD_PRESENCE_HEAL_MAX_HEALTH_PCT=5,SPELL_DK_IMPROVED_BLOOD_PRESENCE_HEAL=50475,SPELLVALUE_BASE_POINT0=0;
int CalculatePct(int value,int pct){return int64_t(value)*pct/100;}
struct AuraEffect{int amount=4;int GetAmount()const{return amount;}};
struct Unit{unsigned base=1000,max=10000;bool realm=true;int healed=0;Unit*ToPlayer(){return this;}unsigned CountPctFromMaxHealth(unsigned n){return max*n/100;}unsigned GetCreateHealth(){return base;}void CastCustomSpell(int id,int,int amount,Unit*,bool,void*,AuraEffect const*){assert(id==50475);healed=amount;}};
namespace AscensionFreePick{bool Applies(Unit*p){return p->realm;}}
struct DamageInfo{unsigned amount=0;unsigned GetDamage(){return amount;}};struct ProcEventInfo{Unit*actor;DamageInfo*damage;Unit*GetActor(){return actor;}DamageInfo*GetDamageInfo(){return damage;}};
struct Presence{void PreventDefaultAction(){}@@HEAL@@};
struct Tick{unsigned id=29841;int misc=1;unsigned GetId(){return id;}int GetMiscValue(){return misc;}unsigned calc(Unit*target,unsigned damage,float TakenTotalMod){@@TICK@@ return damage;}};
int main(){Unit target;Presence p;AuraEffect aura;DamageInfo damage;ProcEventInfo event{&target,&damage};for(unsigned n:{0u,100u,10000u,100000u}){damage.amount=n;target.realm=true;p.HandleProc(&aura,event);assert(target.healed==int(n*4/100));target.realm=false;p.HandleProc(&aura,event);assert(target.healed==int(std::min(n*4/100,500u)));}
Tick t;for(unsigned i=0;i<3;++i){t.id=std::array<unsigned,3>{29841,29842,829839}[i];for(unsigned base:{1000u,2000u}){target.base=base;for(unsigned maximum:{10000u,30000u}){target.max=maximum;assert(t.calc(&target,(i+1)*2,1.0f)==base*(i+1)*2/100);assert(t.calc(&target,(i+1)*2,0.5f)==base*(i+1)/100);}}}t.id=999;assert(t.calc(&target,2,1)==target.max*2/100);}
""".replace('#include <algorithm>','#include <algorithm>\n#include <array>').replace('@@HEAL@@',heal).replace('@@TICK@@',tick)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'t.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'t.cpp'),'-o',str(p/'t')],check=True);subprocess.run([str(p/'t')],check=True)
print('PASS production Second Wind base-health tick branch for all3 ranks, gear/max-health independence and healing modifier; current Area52 Blood Presence4% damage heal and other-realm5% cap preserved. Original client/version differences recorded separately.')
