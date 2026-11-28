from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[3];s=(r/'src/server/coa/AscensionHeroWarrior.cpp').read_text();a=s.index('    bool _bloodsurgeAtCast',s.index('class spell_hero_warrior_bloodsurge_bloodthirst'));b=s.index('    void Register()',a);methods=s[a:b]
code=r"""
#include <cassert>
#include <cstdint>
using uint32=uint32_t;constexpr uint32 Bloodsurge=46916;
struct Aura{unsigned refreshed=0;void RefreshDuration(){++refreshed;}};
struct Player{bool realm=true,buff=true;uint32 guid=1,cooldown=6000,removes=0;Player*ToPlayer(){return this;}bool HasAura(uint32 id){assert(id==Bloodsurge);return buff;}uint32 GetGUID(){return guid;}void RemoveSpellCooldown(uint32 id,bool){assert(id==23881);cooldown=0;}void RemoveAurasDueToSpell(uint32 id){assert(id==Bloodsurge);buff=false;++removes;}};
struct Unit{Aura bleed;uint32 owner=1;bool has=true;Aura*GetAura(uint32 id,uint32 caster){assert(id==954815);return has&&owner==caster?&bleed:nullptr;}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->realm;}}
struct Info{uint32 Id=23881;};
struct Script{Player caster;Unit victim;Unit*target=&victim;Info info;int damage=100;Player*GetCaster(){return &caster;}Unit*GetHitUnit(){return target;}Info*GetSpellInfo(){return &info;}int GetHitDamage(){return damage;}
@@METHODS@@
};
int main(){for(bool delayed:{false,true}){Script s;s.Capture();if(delayed)s.HandleAfterCast();s.RefreshBleed();if(!delayed)s.HandleAfterCast();assert(s.victim.bleed.refreshed==1&&!s.caster.buff&&s.caster.cooldown==0&&s.caster.removes==1);s.HandleAfterCast();assert(s.caster.removes==1);}
 for(unsigned excluded=0;excluded<6;++excluded){Script s;if(excluded==0)s.caster.buff=false;s.Capture();if(excluded==1)s.caster.realm=false;if(excluded==2)s.damage=0;if(excluded==3)s.victim.owner=2;if(excluded==4)s.victim.has=false;if(excluded==5)s.target=nullptr;s.RefreshBleed();assert(s.victim.bleed.refreshed==0);}
}
""".replace('@@METHODS@@',methods).replace('#include <cassert>','#include <cassert>\n#include <initializer_list>')
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS actual Bloodsurge snapshot, immediate/delayed hit ordering, own Bloodbath-only refresh, zero damage and other-mode exclusion, single buff consumption/cooldown reset. Trigger scope is covered by area52_dragon_approved_procs.')
