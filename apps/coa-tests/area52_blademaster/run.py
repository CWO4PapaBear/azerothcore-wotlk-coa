from pathlib import Path
import subprocess,tempfile,sqlite3
root=Path(__file__).resolve().parents[3]
source=r'''#include "Area52BlademasterRules.h"
#include <cassert>
#include <limits>
int main() {
using namespace Area52Blademaster;
assert(DeferredDamage(1000,0,false)==300);
assert(DeferredDamage(1000,20,false)==500);
assert(DeferredDamage(1000,0,true)==150);
assert(DeferredDamage(1000,20,true)==250);
for (unsigned bonus : {0u,20u,100u}) {
 auto maximum=std::numeric_limits<unsigned>::max();
 assert(DeferredDamage(maximum,bonus,false)<=maximum);
}
for (unsigned initial : {0u,1u,9u,10u,11u,301u,2147483647u}) {
 unsigned debt=initial;unsigned long long paid=0;
 for (unsigned ticks=10;ticks;--ticks) {
  unsigned payment=DebtPayment(debt,ticks);assert(payment<=debt);
  paid+=payment;debt-=payment;
 }
 assert(debt==0 && paid==initial);
}
for (unsigned flags=0;flags<64;++flags)
 assert(Eligible(flags&1,flags&2,flags&4,flags&8,flags&16,flags&32)==(flags==1));
}'''
cpp=(root/'src/server/coa/AscensionBlademaster.cpp').read_text()
start=cpp.index('    bool Check(ProcEventInfo& event)');end=cpp.index('    void Proc(',start)
gate=r'''using uint32=unsigned;
constexpr unsigned PROC_HIT_PARRY=32,PROC_HIT_CRITICAL=2,PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS=16;
unsigned now=0;
unsigned getMSTime(){return now;}
unsigned getMSTimeDiff(unsigned old,unsigned current){return current-old;}
struct Cast { bool triggered=false; bool IsTriggered(){return triggered;} };
struct ProcEventInfo {
 void* actor=nullptr;void* target=nullptr;unsigned hit=0,type=0;Cast* cast=nullptr;void* info=nullptr;
 void* GetActor(){return actor;} void* GetActionTarget(){return target;}
 unsigned GetHitMask(){return hit;} unsigned GetTypeMask(){return type;}
 void* GetSpellInfo(){return info;} Cast* GetProcSpell(){return cast;}
};
struct Gate {
 bool _hasCritical=false;unsigned _lastCritical=0;void* owner=this;
 void* GetTarget(){return owner;}
'''+cpp[start:end]+r'''};
void checkGate(){
 Gate g;Cast cast;ProcEventInfo e;
 e.target=g.owner;e.hit=32;assert(g.Check(e));
 e.target=nullptr;e.actor=g.owner;e.hit=2;e.type=4;e.info=&cast;
 assert(!g.Check(e));e.type=16;assert(g.Check(e));
 e.cast=&cast;cast.triggered=true;assert(!g.Check(e));cast.triggered=false;
 g._hasCritical=true;g._lastCritical=1000;now=8999;assert(!g.Check(e));
 now=9000;assert(g.Check(e));
 e.actor=nullptr;e.target=g.owner;e.hit=32;now=1001;assert(g.Check(e));
 e.target=nullptr;assert(!g.Check(e));
}
'''
source=source.replace('int main() {',gate+'\nint main() { checkGate();')
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(source)
 subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-Werror','-I',str(root/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
db=sqlite3.connect(':memory:')
db.execute('CREATE TABLE spell_proc(SpellId,SchoolMask,SpellFamilyName,SpellFamilyMask0,SpellFamilyMask1,SpellFamilyMask2,ProcFlags,SpellTypeMask,SpellPhaseMask,HitMask,AttributesMask,DisableEffectsMask,ProcsPerMinute,Chance,Cooldown,Charges)')
db.execute('CREATE TABLE spell_ranks(first_spell_id,spell_id,rank)')
db.execute('CREATE TABLE spell_script_names(spell_id,ScriptName)')
db.execute("INSERT INTO spell_script_names VALUES (53385,'spell_pal_divine_storm')")
db.execute("INSERT INTO spell_script_names VALUES (53385,'unrelated')")
sql=(root/'data/sql/updates/pending_db_world/rev_20261010_90_area52_blademaster.sql').read_text()
for _ in range(2):
 db.executescript(sql)
 assert db.execute("SELECT spell_id FROM spell_script_names WHERE ScriptName='spell_pal_divine_storm'").fetchall()==[(-53385,)]
 assert db.execute("SELECT COUNT(*) FROM spell_script_names WHERE ScriptName='unrelated'").fetchone()[0]==1
 assert db.execute('SELECT ProcFlags,HitMask,Chance,Cooldown FROM spell_proc').fetchall()==[(56,34,100,0)]
 assert db.execute('SELECT COUNT(*) FROM spell_script_names').fetchone()[0]==11
 assert db.execute('SELECT COUNT(*) FROM spell_ranks WHERE first_spell_id=53385').fetchone()[0]==7
 assert db.execute('SELECT COUNT(*) FROM spell_ranks WHERE first_spell_id=20243').fetchone()[0]==9
print('PASS: Tactical eligibility, PvP stagger reduction, exact debt conservation and idempotent all-rank bindings.')

import importlib.util,struct
spec=importlib.util.spec_from_file_location('patcher',root/'apps/coa-dbc/patch_area52_blademaster.py')
patcher=importlib.util.module_from_spec(spec);spec.loader.exec_module(patcher)
ids=sorted(patcher.VERIFIED_SPELLS|patcher.NUMERIC.keys()|{965863,123456})
fields=234;pool=bytearray(b'\0');rows=bytearray()
original='Visible text\n\n@ext:Original SHIFT text\n\n|cffff0000NOT VERIFIED|r:ext@'
for sid in ids:
 row=[0]*fields;row[0]=sid;row[170]=len(pool)
 pool.extend(original.encode()+b'\0');rows.extend(struct.pack('<'+'I'*fields,*row))
raw=struct.pack('<4s4I',b'WDBC',len(ids),fields,fields*4,len(pool))+rows+pool
result=patcher.patch(raw);assert patcher.patch(result)==result
start=20+len(ids)*fields*4;strings=result[start:]
for i,sid in enumerate(ids):
 row=struct.unpack_from('<'+'I'*fields,result,20+i*fields*4)
 text=strings[row[170]:strings.index(0,row[170])].decode()
 assert text==(original.replace('|cffff0000NOT VERIFIED|r','|cffffff00VERIFIED|r') if sid in patcher.VERIFIED_SPELLS else original)
 if sid==990042:assert row[49]==3 and row[95]==4 and row[46]==0
 if sid in {965865,965867,965869}:assert row[49]==1
print('PASS: Client patch preserves SHIFT prose and spacing, is idempotent and changes only selected statuses.')
