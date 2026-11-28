from pathlib import Path
import struct,sys,tempfile,subprocess
ROOT=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(ROOT/'apps/coa-tests'))
from client_data import dbc_dir
def block(s,m):
 a=s.index(m);b=s.index('{',a);e=b+1;n=1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
m=(ROOT/'src/server/game/Spells/SpellMgr.cpp').read_text();h=(ROOT/'src/server/game/Spells/SpellMgr.h').read_text()
f=block(m,'bool SpellMgr::CanSpellTriggerProcOnEvent(')
enums='\n'.join(block(h,'enum '+n)+';' for n in ['ProcFlags','ProcFlagsSpellType','ProcFlagsSpellPhase','ProcFlagsHit','ProcAttributes'])
s=(ROOT/'src/server/game/Spells/Spell.cpp').read_text();a=s.index('void Spell::prepareDataForTriggerSystem(');a=s.index('case SPELL_DAMAGE_CLASS_MELEE:',a);b=s.index('        default:',a);producer=s[a:b]
b=(dbc_dir()/'Spell.dbc').read_bytes();_,n,fields,size,_=struct.unpack_from('<4s4I',b);wanted={278745,53380,53381,53382};rows={r[0]:r for i in range(n) if (r:=struct.unpack_from('<'+str(fields)+'I',b,20+i*size))[0] in wanted};assert rows.keys()==wanted
fixtures='\n'.join(f'rows[{i}]=SpellInfo{{{i},{r[208]},flag96({r[209]}u,{r[210]}u,{r[211]}u),{r[213]},{r[7]}u}};' for i,r in rows.items())
assert all(rows[i][34] for i in [53380,53381,53382])
code=r'''
#include <cassert>
#include <cstdint>
#include <map>
#include <vector>
using uint32=uint32_t;
@@ENUMS@@
constexpr int BASE_ATTACK=0,OFF_ATTACK=1,SPELLFAMILY_PALADIN=10,SPELL_SCHOOL_MASK_MAGIC=126,SPELL_DIRECT_DAMAGE=1,DIRECT_DAMAGE=0,SPELL_DAMAGE_CLASS_MELEE=2,SPELL_DAMAGE_CLASS_RANGED=3,SPELL_ATTR2_AUTO_REPEAT=32;
struct flag96{unsigned a,b,c;flag96(unsigned x=0,unsigned y=0,unsigned z=0):a(x),b(y),c(z){}};
struct SpellInfo{unsigned Id=0,SpellFamilyName=0;flag96 mask;unsigned DmgClass=0,attr3=0,ManaCost=0,ManaCostPercentage=0;bool IsAffected(unsigned f,flag96 m)const{return (!f||f==SpellFamilyName)&&(!(m.a|m.b|m.c)||(mask.a&m.a)||(mask.b&m.b)||(mask.c&m.c));}bool HasAttribute(unsigned)const{return false;}};
struct Player;struct Unit{Player*ToPlayer();bool IsValidAttackTarget(Unit*){return true;}float GetExactDist(Unit*){return 30;}};struct Player:Unit{std::vector<unsigned>GetSpellInteractionOrigins(unsigned)const{return {};}bool MatchesSpellInteraction(unsigned,unsigned)const{return false;}bool isHonorOrXPTarget(Unit*){return true;}};Player*Unit::ToPlayer(){return static_cast<Player*>(this);}
namespace AscensionFreePick{bool Enabled(){return true;}bool Applies(Player const*p){return p;}}
namespace AscensionMysticEnchant{bool DefiasPistolProcEligible(Player*,unsigned){return false;}}
struct DamageInfo{unsigned GetDamage()const{return 100;}int GetAttackType()const{return 0;}unsigned GetSchoolMask()const{return 2;}int GetDamageType()const{return 1;}};
struct ProcEventInfo{Player actor;Unit target;DamageInfo damage;SpellInfo const*spell;unsigned type,spellType=1,phase=2,hit=1;Unit*GetActor(){return &actor;}Unit*GetActionTarget(){return &target;}DamageInfo*GetDamageInfo(){return &damage;}SpellInfo const*GetSpellInfo(){return spell;}unsigned GetTypeMask(){return type;}unsigned GetSchoolMask(){return 2;}unsigned GetSpellTypeMask(){return spellType;}unsigned GetSpellPhaseMask(){return phase;}unsigned GetHitMask(){return hit;}};
struct SpellProcEntry{unsigned ProcFlags=0,AttributesMask=0,SchoolMask=0,SpellFamilyName=10;flag96 SpellFamilyMask;unsigned SpellTypeMask=1,SpellPhaseMask=2,HitMask=0;};
struct SpellMgr{std::map<unsigned,SpellInfo>rows;SpellProcEntry const*GetSpellProcEntry(unsigned)const{return nullptr;}SpellInfo const*GetSpellInfo(unsigned id)const{auto i=rows.find(id);return i==rows.end()?nullptr:&i->second;}bool CanSpellTriggerProcOnEvent(SpellProcEntry const&,ProcEventInfo&)const;};
@@FUNC@@
unsigned Produce(SpellInfo const* m_spellInfo){unsigned m_procAttacker=0,m_procVictim=0;int m_attackType=BASE_ATTACK;switch(m_spellInfo->DmgClass){@@PRODUCER@@ default:break;}return m_procAttacker;}
int main(){SpellMgr manager;auto&rows=manager.rows;@@FIXTURES@@
for(unsigned flags:{@@RVFLAGS@@}){SpellProcEntry vengeance;vengeance.ProcFlags=flags;vengeance.SpellFamilyMask=flag96();vengeance.HitMask=PROC_HIT_CRITICAL;vengeance.AttributesMask=PROC_ATTR_TRIGGERED_CAN_PROC;
ProcEventInfo event;event.spell=&rows[278745];event.type=Produce(event.spell);event.hit=PROC_HIT_CRITICAL;assert(manager.CanSpellTriggerProcOnEvent(vengeance,event));event.hit=PROC_HIT_NORMAL;assert(!manager.CanSpellTriggerProcOnEvent(vengeance,event));event.hit=PROC_HIT_MISS;assert(!manager.CanSpellTriggerProcOnEvent(vengeance,event));event.hit=PROC_HIT_CRITICAL;event.phase=PROC_SPELL_PHASE_CAST;assert(!manager.CanSpellTriggerProcOnEvent(vengeance,event));}
}

'''
for k,v in [('ENUMS',enums),('FUNC',f),('PRODUCER',producer),('FIXTURES',fixtures),('RVFLAGS',','.join(str(rows[i][34]) for i in [53380,53381,53382]))]:code=code.replace('@@'+k+'@@',v)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS native Templar producer and RighteousVengeance all3rank proc admission:critical hit accepted,normal/miss/castphase rejected. Uses capturedproc familymask0,criticalmask2,triggered permission; actualeffecthandler separately qualified.')
