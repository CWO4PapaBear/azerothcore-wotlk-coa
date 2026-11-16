from pathlib import Path
import subprocess,tempfile,sqlite3,re
root=Path(__file__).resolve().parents[3]
source = r'''#include "Area52AdvancementProcRules.h"
#include <cassert>
using namespace Area52AdvancementProcs;
int main()
{
    Event e; e.FreePick=true; e.OwnerIsActor=true;
    e.SpellRoot=17; assert(Accept(14769,e)); e.SpellRoot=2061; assert(!Accept(14769,e));
    e.SpellRoot=116; assert(Accept(31682,e) && Accept(31683,e) && Accept(31684,e));
    e.SpellRoot=133; assert(!Accept(31684,e));
    e.Damage=true; e.Periodic=true;
    for (auto root : {703u,1943u,954809u}) {e.SpellRoot=root; assert(Accept(14142,e));}
    e.SpellRoot=1752; assert(!Accept(14142,e));
    e.School=2; assert(Accept(33225,e)); e.School=32; assert(Accept(33225,e));
    e.School=4; assert(!Accept(33225,e)); e.Periodic=false; e.School=32; assert(!Accept(33225,e));
    e.Family=4; assert(Accept(12308,e)&&Accept(12810,e)&&Accept(12811,e));
    e.Family=8; assert(!Accept(12811,e)); e.Family=4; e.Periodic=true; assert(!Accept(12811,e));
    e.Periodic=false; e.SpellRoot=5221; e.Behind=true; assert(Accept(816968,e));
    e.SpellRoot=6785; assert(Accept(816968,e)); e.Behind=false; assert(!Accept(816968,e));
    e.Dodged=true; assert(!Accept(12753,e)); e.OwnerIsActor=false; assert(Accept(12753,e));
    e.Dodged=false; assert(!Accept(12753,e)); e.Dodged=true; e.FreePick=false; assert(!Accept(12753,e));
    e.OwnerIsActor=true; e.SpellRoot=17; assert(!Accept(14769,e));
    e.FreePick=true; assert(!Accept(99999,e));
}
'''
source=source.replace('#include <cassert>','#include <cassert>\n#include <initializer_list>')
with tempfile.TemporaryDirectory() as directory:
    path=Path(directory);(path/'test.cpp').write_text(source)
    subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-Werror','-I',str(root/'src/server/coa'),str(path/'test.cpp'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True)
print('PASS: source predicates accept required events and reject unrelated spells, periodic/direct mismatches, targets and other modes.')
sql = (root/'data/sql/updates/pending_db_world/rev_20261008_09_area52_advancement_procs.sql').read_text()
connection = sqlite3.connect(':memory:')
for table in ('spell_proc', 'spell_script_names'):
    columns = re.search(r'INSERT INTO `' + table + r'` \(([^)]+)\)', sql)[1]
    connection.execute('CREATE TABLE ' + table + ' (' + columns + ')')
connection.execute('INSERT INTO spell_proc (SpellId, Chance) VALUES (99999999, 73)')
connection.execute("INSERT INTO spell_script_names VALUES (14769, 'unrelated_handler')")
connection.executescript(sql)
first = [connection.execute('SELECT * FROM ' + table).fetchall() for table in ('spell_proc', 'spell_script_names')]
connection.executescript(sql)
second = [connection.execute('SELECT * FROM ' + table).fetchall() for table in ('spell_proc', 'spell_script_names')]
assert first == second
assert len(first[0]) == 12 and len(first[1]) == 12
assert connection.execute('SELECT Chance FROM spell_proc WHERE SpellId=99999999').fetchone() == (73,)
assert connection.execute("SELECT COUNT(*) FROM spell_script_names WHERE ScriptName='unrelated_handler'").fetchone() == (1,)
assert connection.execute('SELECT COUNT(*) FROM spell_proc WHERE SpellId<0').fetchone() == (0,)
print('PASS: SQL statement replay is idempotent, preserves unrelated bindings, and uses explicit positive spell IDs.')
