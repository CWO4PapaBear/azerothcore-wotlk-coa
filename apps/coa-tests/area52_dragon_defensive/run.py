from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[3]
def method(path,cls,signature):
 s=(r/path).read_text();a=s.index(signature,s.index('class '+cls));b=s.index('{',a);depth=1;i=b+1
 while depth:
  depth += (s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]
second=method('src/server/scripts/Spells/spell_warrior.cpp','spell_warr_second_wind','bool CheckProc(')
absorb=method('src/server/scripts/Spells/spell_dk.cpp','spell_dk_will_of_the_necropolis','void Absorb(')
s=r"""
#include <cassert>
#include <cstdint>
using uint32=uint32_t;using uint8=uint8_t;using int32=int32_t;
constexpr int EFFECT_0=0,SPELL_AURA_PROC_TRIGGER_SPELL=42,MECHANIC_ROOT=7,MECHANIC_STUN=12;
constexpr uint32 PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS=16,SPELL_DK_WILL_OF_THE_NECROPOLIS_TALENT_R1=49189;
struct Unit{uint32 health=1000,max=1000;bool HealthBelowPct(unsigned n){return uint64_t(health)*100<uint64_t(max)*n;}uint32 GetHealth(){return health;}uint32 CountPctFromMaxHealth(unsigned p){return max*p/100;}};
struct Effect{int ApplyAuraName=42,value=35;int CalcValue(Unit*)const{return value;}};
struct SpellInfo{Effect Effects[3];uint64_t mask=0;uint8 rank=1;uint64_t GetAllEffectsMechanicMask()const{return mask;}uint8 GetRank()const{return rank;}};
struct DamageInfo{uint32 damage=1;uint32 GetDamage(){return damage;}};
struct ProcEventInfo{SpellInfo*spell=nullptr;Unit*actor=nullptr,*target=nullptr;DamageInfo*damage=nullptr;uint32 flags=16;SpellInfo*GetSpellInfo(){return spell;}Unit*GetActor(){return actor;}Unit*GetActionTarget(){return target;}uint32 GetTypeMask(){return flags;}DamageInfo*GetDamageInfo(){return damage;}};
struct AuraEffect{};
uint32 CalculatePct(uint32 v,uint32 p){return uint64_t(v)*p/100;}
struct Manager{SpellInfo talent;uint32 GetSpellWithRank(uint32 id,uint8){return id;}SpellInfo const*AssertSpellInfo(uint32){return &talent;}}manager,*sSpellMgr=&manager;
struct Script{Unit owner;SpellInfo info;uint32 absorbPct=5;Unit*GetTarget(){return &owner;}Unit*GetCaster(){return &owner;}SpellInfo const*GetSpellInfo(){return &info;}
@@SECOND@@
@@ABSORB@@
};
int main(){
 Script s;Unit enemy;SpellInfo spell;DamageInfo damage;ProcEventInfo e{&spell,&s.owner,&enemy,&damage,16};
 for(unsigned hp:{349u,350u,351u,1000u}){s.owner.health=hp;assert(s.CheckProc(e)==(hp<350));}
 s.owner.health=349;e.actor=&enemy;assert(!s.CheckProc(e));e.actor=&s.owner;
 e.flags=4;assert(!s.CheckProc(e));e.flags=16;damage.damage=0;assert(!s.CheckProc(e));damage.damage=1;
 e.damage=nullptr;assert(!s.CheckProc(e));e.damage=&damage;e.spell=nullptr;assert(!s.CheckProc(e));e.spell=&spell;
 s.info.Effects[0].ApplyAuraName=4;spell.mask=1ULL<<MECHANIC_ROOT;assert(s.CheckProc(e));e.target=e.actor;assert(!s.CheckProc(e));e.target=&enemy;spell.mask=0;assert(!s.CheckProc(e));
 for(unsigned rank=1;rank<=3;++rank){s.info.rank=rank;s.absorbPct=rank*5;
 for(unsigned hp:{200u,350u,500u,1000u})for(unsigned hit:{0u,1u,150u,151u,800u,1200u}){
  s.owner.health=hp;damage.damage=hit;uint32 result=0;s.Absorb(nullptr,damage,result);
  assert(result==((int64_t(hp)-hit<350)?uint64_t(hit)*rank*5/100:0));
 }}
}
""".replace('@@SECOND@@',second).replace('@@ABSORB@@',absorb)
s=s.replace('#include <cstdint>','#include <cstdint>\n#include <initializer_list>')
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(s)
 subprocess.run(['c++','-std=c++20','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS: production Second Wind admission; Necropolis 3 ranks across threshold, crossing and lethal damage. Dispatcher, data and restoration gates remain separate.')
