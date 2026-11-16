from pathlib import Path
import subprocess,tempfile,sqlite3,re
root=Path(__file__).resolve().parents[3]
source=r'''#include "Area52FollowupProcRules.h"
#include <cassert>
#include <initializer_list>
using namespace Area52FollowupProcs;
int main()
{
    assert(HealingOnly(true,760192));
    assert(!HealingOnly(false,760192) && !HealingOnly(true,31882));
    assert(FrostKill(true,true,true,true,true));
    assert(!FrostKill(false,true,true,true,true));
    assert(!FrostKill(true,false,true,true,true));
    assert(!FrostKill(true,true,false,true,true));
    assert(!FrostKill(true,true,true,false,true));
    assert(!FrostKill(true,true,true,true,false));
    assert(WindRush(true,true,57994,true));
    assert(!WindRush(true,true,57994,false));
    assert(!WindRush(true,false,57994,true));
    assert(!WindRush(false,true,57994,true));
    assert(!WindRush(true,true,1766,true));
    assert(FlameReady(true,false,false));
    assert(!FlameReady(true,true,false) && !FlameReady(true,false,true) && !FlameReady(false,false,false));
    assert(TotemPower(80,1,true)==24 && TotemPower(80,1,false)==12);
    assert(TotemPower(80,2,true)==48 && TotemPower(80,3,false)==36);
    assert(TotemPower(1,3,false)==0 && TotemPower(80,4,true)==0);
    Event e; e.FreePick=true; e.OwnerIsActor=true;
    e.Root=2645; assert(Trigger(16288,e)==965489);
    e.Root=2061; assert(Trigger(63506,e)==415011); assert(!Trigger(16288,e));
    e.Root=954500; assert(Trigger(20361,e)==20362);
    e.Root=133; assert(Trigger(811100,e)==811101 && !Trigger(20361,e));
    e.Root=19740; assert(!Trigger(20042,e)); e.OtherTarget=true;
    assert(Trigger(20042,e)==920043 && Trigger(20045,e)==920043);
    e.Root=20217; assert(!Trigger(20045,e));
    for (auto root : {5143u,44425u,30451u,955032u,8921u,2912u,954800u})
    {
        e.Root=root;
        assert(Trigger(44400,e)==966120 && Trigger(44402,e)==966121 && Trigger(44403,e)==966122);
    }
    e.Root=31589; e.NetherwindStacks=2; assert(!Trigger(44403,e));
    e.NetherwindStacks=3; assert(Trigger(44403,e)==844405 && !Trigger(44402,e));
    e.Root=379; e.Healing=true; assert(!Trigger(851561,e));
    e.FinalTidalWaves=true; assert(Trigger(851561,e)==851562);
    e.Healing=false; assert(!Trigger(851561,e));
    e.Root=760106; assert(!Trigger(760191,e));
    e.Healing=true; assert(Trigger(760191,e)==760192);
    e.OwnerIsActor=false; assert(!Trigger(760191,e) && !Trigger(851561,e));
    e.OwnerIsActor=true; e.Periodic=true; assert(!Trigger(760191,e));
    e.Periodic=false;
    e.Damage=true; e.Critical=true;
    e.Root=1495; assert(Trigger(19298,e)==983791);
    e.Root=2973; assert(Trigger(19298,e)==983791);
    e.Root=6603; assert(!Trigger(19298,e));
    e.Root=1495; e.Critical=false; assert(!Trigger(19298,e));
    e.Critical=true; e.Periodic=true; assert(!Trigger(19298,e));
    e.Periodic=false; e.Damage=false; assert(!Trigger(19298,e));
    for (auto root : {5676u,29722u,2948u,133u}) {e.Root=root; assert(Trigger(17958,e)==411119);}
    e.Root=116; assert(!Trigger(17958,e));
    e.Root=6673; assert(Trigger(12318,e)==812316 && Trigger(12857,e)==812318);
    e.Root=469; assert(Trigger(12318,e)==812317 && Trigger(12857,e)==812319);
    e.Healing=true; e.Critical=true;
    for (auto root : {331u,61295u,8004u,1064u}) {e.Root=root; assert(Trigger(16219,e)==986107);}
    assert(TidalExtension(331)==2000 && TidalExtension(61295)==2000);
    assert(TidalExtension(8004)==1000 && TidalExtension(1064)==500);
    e.Critical=false; assert(!Trigger(16219,e)); e.Critical=true; e.Periodic=true; assert(!Trigger(16219,e));
    e.Damage=true;
    for (auto root : {13797u,13812u,3674u}) {e.Root=root; assert(Trigger(34493,e)==734493);}
    e.Root=1978; assert(!Trigger(34493,e)); e.Periodic=false; e.Root=3674; assert(!Trigger(34493,e));
    e.Root=8042; assert(Trigger(863372,e)==863375);
    e.Root=8050; assert(Trigger(863372,e)==863376);
    e.Root=8056; assert(Trigger(863372,e)==863377);
    e.Periodic=true; assert(!Trigger(863372,e)); e.Periodic=false;
    e.Damage=false; assert(!Trigger(863372,e)); e.Damage=true;
    e.FreePick=false; assert(!Trigger(863372,e)); e.FreePick=true; e.OwnerIsActor=false; assert(!Trigger(863372,e));
}
'''
with tempfile.TemporaryDirectory() as directory:
    path=Path(directory);(path/'test.cpp').write_text(source)
    subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-Werror','-I',str(root/'src/server/coa'),str(path/'test.cpp'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True)
sql=(root/'data/sql/updates/pending_db_world/rev_20261008_10_area52_followup_procs.sql').read_text()
db=sqlite3.connect(':memory:')
for table in ('spell_proc','spell_script_names'):
    columns=re.search(r'INSERT INTO `'+table+r'` \(([^)]+)\)',sql)[1]
    db.execute('CREATE TABLE '+table+' ('+columns+')')
db.execute('INSERT INTO spell_proc (SpellId,Chance) VALUES (99999999,73)')
db.execute("INSERT INTO spell_script_names VALUES (16288,'unrelated')")
db.executescript(sql)
first=[db.execute('SELECT * FROM '+t).fetchall() for t in ('spell_proc','spell_script_names')]
db.executescript(sql)
assert first==[db.execute('SELECT * FROM '+t).fetchall() for t in ('spell_proc','spell_script_names')]
assert len(first[0])==9 and len(first[1])==9
assert db.execute('SELECT Cooldown FROM spell_proc WHERE SpellId=16288').fetchone()==(20000,)
assert db.execute('SELECT Cooldown FROM spell_proc WHERE SpellId=34493').fetchone()==(1000,)
assert db.execute('SELECT DisableEffectsMask FROM spell_proc WHERE SpellId=17958').fetchone()==(4,)
print('PASS: seven event policies, target ownership, critical/periodic exclusions, duration increments and SQL replay isolation.')
stateful=(root/'data/sql/updates/pending_db_world/rev_20261008_11_area52_stateful_procs.sql').read_text()
db.executescript(stateful)
first=[db.execute('SELECT * FROM '+t+' ORDER BY 1,2').fetchall() for t in ('spell_proc','spell_script_names')]
db.executescript(sql+stateful)
assert first==[db.execute('SELECT * FROM '+t+' ORDER BY 1,2').fetchall() for t in ('spell_proc','spell_script_names')]
for sid in (20362,811100):
    assert db.execute('SELECT Charges,AttributesMask,SpellPhaseMask FROM spell_proc WHERE SpellId=?',(sid,)).fetchone()==(1,8,4)
assert db.execute('SELECT DisableEffectsMask FROM spell_proc WHERE SpellId=20045').fetchone()==(2,)
print('PASS: combined migration replay, one-use spellmod charge consumption and single grant effect binding.')
