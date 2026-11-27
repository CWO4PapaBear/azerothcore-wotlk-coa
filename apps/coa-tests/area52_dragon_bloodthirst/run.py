from pathlib import Path
import tempfile,subprocess,resource
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
r=Path(__file__).resolve().parents[3]
s=(r/'src/server/scripts/Spells/spell_warrior.cpp').read_text();a=s.index('void HandleDamage(',s.index('class spell_warr_bloodthirst'));b=s.index('void HandleDummy(',a);method=s[a:b]
code=r"""
#include "Area52DragonReviewRules.h"
#include <cassert>
#include <cstdint>
#include <map>
#include <vector>
using uint32=uint32_t;using int32=int32_t;using SpellEffIndex=int;
constexpr int MECHANIC_BLEED=15,SPELL_DIRECT_DAMAGE=1;
struct SpellInfo{uint64_t mask=0;uint64_t GetAllEffectsMechanicMask()const{return mask;}};
struct Aura{uint32 owner=1;SpellInfo info;uint32 GetCasterGUID()const{return owner;}SpellInfo const*GetSpellInfo()const{return &info;}};
struct Application{Aura aura;Aura const*GetBase()const{return &aura;}};
struct Unit{uint32 guid=1;bool realm=true;std::map<uint32,Application*>auras;Unit*ToPlayer(){return this;}auto const&GetAppliedAuras(){return auras;}uint32 GetGUID(){return guid;}bool HasAura(uint32){return false;}
 uint32 SpellDamageBonusDone(Unit*,SpellInfo const*,uint32 n,int,int,float,int,float*){return n;}
 uint32 SpellDamageBonusTaken(Unit*,SpellInfo const*,uint32 n,int,int,float*){return n;}
};
namespace AscensionFreePick{bool Applies(Unit*u){return u&&u->realm;}}
struct Cast{bool IsCurrentUnitImpactCritical(){return false;}};
int32 WarriorHitAttackPower(Cast*,Unit*,Unit*){return 1000;}
void ApplyPct(int32&n,int32 p){n=n*p/100;}
struct Script{Unit caster,target;Cast cast;SpellInfo info;int32 hit=0;int32 GetEffectValue(){return 55;}Unit*GetCaster(){return &caster;}Unit*GetHitUnit(){return &target;}Cast*GetSpell(){return &cast;}SpellInfo const*GetSpellInfo(){return &info;}void SetHitDamage(int32 n){hit=n;}
@@METHOD@@
};
int main(){Script s;std::vector<Application>a(10);for(auto&x:a)x.aura.info.mask=1ULL<<MECHANIC_BLEED;
 for(unsigned n=0;n<=8;++n){s.target.auras.clear();for(unsigned i=0;i<n;++i)s.target.auras[i]=&a[i];s.HandleDamage(0);int expected=550+44*std::min(n,5u);assert(s.hit>=expected-1&&s.hit<=expected);}
 s.target.auras.clear();a[0].aura.owner=2;s.target.auras[0]=&a[0];a[1].aura.info.mask=0;s.target.auras[1]=&a[1];s.HandleDamage(0);assert(s.hit==550);
 a[0].aura.owner=1;s.HandleDamage(0);assert(s.hit==594);
 s.caster.realm=false;s.HandleDamage(0);assert(s.hit==550);
}
""".replace('@@METHOD@@',method)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code)
 subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-I'+str(r/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
 baseline=subprocess.check_output(['git','show','HEAD:src/server/scripts/Spells/spell_warrior.cpp'],cwd=r,text=True)
 a=baseline.index('void HandleDamage(',baseline.index('class spell_warr_bloodthirst'));b=baseline.index('void HandleDummy(',a)
 (p/'baseline.cpp').write_text(code.replace(method,baseline[a:b]))
 subprocess.run(['c++','-std=c++20','-I'+str(r/'src/server/coa'),str(p/'baseline.cpp'),'-o',str(p/'baseline')],check=True)
 result=subprocess.run([str(p/'baseline')],capture_output=True,text=True)
 assert result.returncode!=0 and 's.hit>=expected-1' in result.stderr,result.stderr
 print('Confirmed HEAD baseline fails owned-bleed assertion; candidate passes same cases.')
print('PASS: production Bloodthirst handler 55% AP, owned bleeds 0-8, cap, foreign bleed/nonbleed exclusion and non-Area52 isolation; downstream bonus functions stubbed identity.')
