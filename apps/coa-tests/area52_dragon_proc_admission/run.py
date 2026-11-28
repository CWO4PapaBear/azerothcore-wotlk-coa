from pathlib import Path
import tempfile,subprocess,re,struct
r=Path(__file__).resolve().parents[3]
def method(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
header=(r/'src/server/game/Spells/SpellMgr.h').read_text();enums='\n'.join(method(header,'enum '+n)+';' for n in ['ProcFlags','ProcFlagsSpellType','ProcFlagsSpellPhase','ProcFlagsHit'])
f=method((r/'src/server/game/Spells/SpellMgr.cpp').read_text(),'bool SpellMgr::CanSpellTriggerProcOnEvent(')
flags=f[f.index('    if (!(eventInfo.GetTypeMask() & procEntry.ProcFlags))'):f.index('    // check XP')]
phase=f[f.index('    // check spell type mask'):f.rindex('}')]
code=r"""
#include <cassert>
using uint32=unsigned;
@@ENUMS@@
struct Event{unsigned flags=16,type=1,phase=2,hit=1;unsigned GetTypeMask(){return flags;}unsigned GetSpellTypeMask(){return type;}unsigned GetSpellPhaseMask(){return phase;}unsigned GetHitMask(){return hit;}};
struct Entry{unsigned ProcFlags=16,SpellTypeMask=1,SpellPhaseMask=2,HitMask=3;};
bool eligible(Entry const&procEntry,Event&eventInfo){@@FLAGS@@ @@PHASE@@}
int main(){Entry w{0x400010,1,2,2};Event e;e.flags=PROC_FLAG_DONE_MAINHAND_ATTACK|PROC_FLAG_DONE_MELEE_AUTO_ATTACK;e.hit=PROC_HIT_CRITICAL;assert(eligible(w,e));e.flags=PROC_FLAG_DONE_OFFHAND_ATTACK|PROC_FLAG_DONE_MELEE_AUTO_ATTACK;assert(!eligible(w,e));Entry old=w;old.ProcFlags|=PROC_FLAG_DONE_MELEE_AUTO_ATTACK;assert(eligible(old,e));
 for(unsigned hand:{PROC_FLAG_DONE_MAINHAND_ATTACK,PROC_FLAG_DONE_OFFHAND_ATTACK}){e.flags=hand|PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS;assert(eligible(w,e));e.hit=PROC_HIT_NORMAL;assert(!eligible(w,e));e.hit=PROC_HIT_CRITICAL;}
 for(unsigned bad:{PROC_FLAG_DONE_RANGED_AUTO_ATTACK,PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG,PROC_FLAG_DONE_PERIODIC}){e.flags=bad;assert(!eligible(w,e));}
 Entry b{16,1,2,3};e.flags=16;for(unsigned hit:{PROC_HIT_NORMAL,PROC_HIT_CRITICAL}){e.hit=hit;assert(eligible(b,e));}e.phase=1;assert(!eligible(b,e));e.phase=2;e.hit=PROC_HIT_MISS;assert(!eligible(b,e));e.hit=1;e.type=2;assert(!eligible(b,e));
}
""".replace('@@ENUMS@@',enums).replace('@@FLAGS@@',flags).replace('@@PHASE@@',phase).replace('#include <cassert>','#include <cassert>\n#include <initializer_list>')
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'t.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'t.cpp'),'-o',str(p/'t')],check=True);subprocess.run([str(p/'t')],check=True)
print('PASS native proc flags/phase/type/hit admission with actual enums: Wrecking Crew original offhand leak reproduced, corrected mainhand/special crit contract; Bloodbath direct melee damage hit boundary. School/family/ownership checked separately.')
