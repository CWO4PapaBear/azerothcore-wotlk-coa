from pathlib import Path
import subprocess,tempfile,sqlite3,re
root=Path(__file__).resolve().parents[3]
inc=(root/'src/server/coa/Area52Battlemage.inc').read_text()
start=inc.index('    std::vector<uint32> KnownChoices(');end=inc.index('\n    bool CheckProc',start)
choices=inc[start:end]
source=r'''#include <vector>
#include <set>
#include <map>
using uint32=unsigned;
struct Player { std::set<uint32> known; bool HasSpell(uint32 id) const { return known.contains(id); } };
struct Manager { std::map<uint32,uint32> next; uint32 GetNextSpellInChain(uint32 id) { return next[id]; } } manager;
auto* sSpellMgr=&manager;
'''+'struct Selector {\n'+choices+'\n};\n'+r''' #include "Area52BattlemageRules.h"
#include <cassert>
#include <limits>
int main() {
using namespace Area52Battlemage;
Player p;Selector selector;
assert(selector.KnownChoices(&p).empty());
manager.next={{2136,2137},{2137,27079},{44425,44780},{44780,44781}};
p.known={2136,2137,44425,44781};
assert((selector.KnownChoices(&p)==std::vector<uint32>{44781,2137}));
p.known={27079,30455};assert((selector.KnownChoices(&p)==std::vector<uint32>{27079,30455}));
manager.next[27079]=2136;assert(selector.KnownChoices(&p).size()==2);
assert(Accept(true,true,true,false,false));
for (unsigned i=0;i<32;++i) assert(Accept(i&1,i&2,i&4,i&8,i&16)==(i==7));
assert(Damage(100,true)==80);assert(Damage(100,false)==100);assert(Damage(0,true)==0);
assert(Damage(std::numeric_limits<std::int32_t>::max(),true)==1717986917);
}'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(source)
 subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-Werror','-I',str(root/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
sql=(root/'data/sql/updates/pending_db_world/rev_20261009_03_area52_battlemage.sql').read_text()
db=sqlite3.connect(':memory:')
for table in ('spell_proc','spell_script_names'):
 columns=re.search(r'INSERT INTO `'+table+r'` \(([^)]+)\)',sql)[1];db.execute('CREATE TABLE '+table+' ('+columns+')')
db.execute('CREATE TABLE spell_ranks(first_spell_id,spell_id)')
db.executemany('INSERT INTO spell_ranks VALUES (?,?)',[(44425,44425),(44425,44780),(2136,2136),(2136,2137),(30455,30455)])
db.execute("INSERT INTO spell_script_names VALUES (2136,'unrelated')")
for _ in range(2):
 db.executescript(sql)
 assert db.execute('SELECT COUNT(*) FROM spell_script_names').fetchone()[0]==7
 assert db.execute('SELECT Chance,Cooldown,SpellFamilyName FROM spell_proc WHERE SpellId=81258').fetchone()==(100,4000,4)
 assert db.execute("SELECT COUNT(*) FROM spell_script_names WHERE ScriptName='unrelated'").fetchone()[0]==1
print('PASS: Battlemage eligibility truth table, PvP damage arithmetic and idempotent rank binding SQL.')
