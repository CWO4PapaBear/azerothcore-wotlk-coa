from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[3];s=(r/'src/server/scripts/Spells/spell_dk.cpp').read_text()
def method(cls,sig):
 a=s.index(sig,s.index('class '+cls));b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
code=r"""
#include <cassert>
#include <cstdint>
using uint32=uint32_t;using SpellCastResult=int;using AuraEffectHandleModes=int;struct AuraEffect{};
constexpr int SPELL_FAILED_BAD_TARGETS=1,SPELL_FAILED_TARGET_NOT_DEAD=2,SPELL_CAST_OK=0,EFFECT_0=0;
struct Unit{bool alive=false;uint32 entry=30230,despawns=0;Unit*charm=nullptr;bool IsAlive(){return alive;}Unit*GetCharm(){return charm;}uint32 GetEntry(){return entry;}Unit*ToCreature(){return this;}void DespawnOrUnsummon(){++despawns;}};
struct Effect{int MiscValue=30230;};struct Info{Effect Effects[3];};
struct Script{Unit*target=nullptr;Unit owner;Info info;Unit*GetExplTargetUnit(){return target;}Unit*GetUnitOwner(){return &owner;}Info*GetSpellInfo(){return &info;}
@@CHECK@@
@@REMOVE@@
};
int main(){Script s;assert(s.CheckCast()==1);Unit target;s.target=&target;assert(s.CheckCast()==0);target.alive=true;assert(s.CheckCast()==2);
 s.HandleEffectRemove(nullptr,0);Unit ghoul;s.owner.charm=&ghoul;s.HandleEffectRemove(nullptr,0);assert(ghoul.despawns==1);ghoul.entry=999;s.HandleEffectRemove(nullptr,0);assert(ghoul.despawns==1);ghoul.entry=30230;s.info.Effects[0].MiscValue=-1;s.HandleEffectRemove(nullptr,0);assert(ghoul.despawns==1);
}
""".replace('@@CHECK@@',method('spell_dk_raise_ally','SpellCastResult CheckCast()')).replace('@@REMOVE@@',method('spell_dk_raise_ally_trigger','void HandleEffectRemove('))
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS actual Raise Ally missing/living/dead target checks and owned matching-charm cleanup; full puppet stat initialization and client control not emulated.')
