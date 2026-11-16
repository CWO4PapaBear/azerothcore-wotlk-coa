from pathlib import Path
import re
import sqlite3
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
source = r'''
#include "Area52LegendaryBatchRules.h"
#include <cassert>
int main()
{
    using namespace Area52LegendaryBatch;
    for (unsigned mask = 0; mask < 32; ++mask)
        assert(Direct(mask & 1, mask & 2, mask & 4, mask & 8, mask & 16) == (mask == 7));
    assert(Overheating(true, false, 0, 5));
    assert(!Overheating(false, true, 4, 5));
    assert(Overheating(false, true, 5, 5));
    assert(!Overheating(false, false, 5, 5));
    assert(!Overheating(false, true, 0, 0));
    assert(Overheating(false, true, 3, 3));
    assert(Clearcast(84510) == 84748);
    assert(Clearcast(965503) == 983908);
    assert(Clearcast(965504) == 0);
    assert(Threshold(true) == 2 && Threshold(false) == 10);
}
'''
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)
    (path / 'test.cpp').write_text(source)
    subprocess.run(['c++', '-std=c++20', '-Wall', '-Wextra', '-Werror', '-I', str(root / 'src/server/coa'),
                    str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)

sql = (root / 'data/sql/updates/pending_db_world/rev_20261009_04_area52_legendary_batch.sql').read_text()
database = sqlite3.connect(':memory:')
for table in ('spell_proc', 'spell_script_names', 'spell_bonus_data', 'spell_ranks'):
    columns = re.search(r'INSERT INTO `' + table + r'` \(([^)]+)\)', sql)[1]
    database.execute('CREATE TABLE ' + table + ' (' + columns + ')')
database.execute("INSERT INTO spell_script_names VALUES (5487, 'preserved_form_handler')")
for _ in range(2):
    database.executescript(sql)
    assert database.execute('SELECT COUNT(*) FROM spell_proc').fetchone()[0] == 12
    assert database.execute('SELECT COUNT(*) FROM spell_script_names').fetchone()[0] == 25
    for spell, cooldown in ((84510, 4000), (965503, 6000), (965504, 12000), (965505, 0)):
        assert database.execute('SELECT Cooldown FROM spell_proc WHERE SpellId=?', (spell,)).fetchone()[0] == cooldown
    for spell, charges in ((84748, 1), (983908, 2)):
        assert database.execute('SELECT SpellPhaseMask,Charges FROM spell_proc WHERE SpellId=?',
                                (spell,)).fetchone() == (1, charges)
    assert database.execute('SELECT dot_bonus,ap_dot_bonus FROM spell_bonus_data WHERE entry=965316').fetchone() == (
        0.015, 0.01035)
    preserved = database.execute(
        "SELECT COUNT(*) FROM spell_script_names WHERE ScriptName='preserved_form_handler'").fetchone()[0]
    assert preserved == 1

print('PASS: owner/direct-event eligibility, stack thresholds, clearcast mapping, repeatable bindings and coefficients.')

implementation = (root / 'src/server/coa/Area52Felfire.inc').read_text().split(
    'class spell_area52_felfire_equipment')[0]
fixture = r'''
#include <array>
#include <map>
#include <set>
#include <cassert>
#include <cstdint>
using uint32 = std::uint32_t;
struct Session { bool logout = false; bool PlayerLogout() { return logout; } };
struct Player {
    Session session;
    bool freePick = true, equipped = true;
    std::set<uint32> known, active, auras;
    int form = 0;
    Player* ToPlayer() { return this; }
    int GetShapeshiftForm() { return form; }
    uint32 GetGUID() { return 1; }
    bool HasAura(uint32 id, uint32 = 0) { return auras.contains(id); }
    void CastSpell(Player*, uint32 id, bool) { auras.insert(id); }
    void RemoveAurasDueToSpell(uint32 id, uint32) { auras.erase(id); }
    std::map<uint32,uint32> replacements;
    std::map<uint32,bool> grants;
    Session* GetSession() { return &session; }
    bool HasSpell(uint32 id) { return known.contains(id); }
    bool HasActiveSpell(uint32 id) { return active.contains(id); }
    uint32 GetTemporarySpellReplacement(uint32 id) { return replacements[id]; }
    void SetTemporarySpellReplacement(uint32 id, uint32 other) { replacements[id] = other; }
};
using Unit = Player;
constexpr int FORM_BEAR = 5, FORM_DIREBEAR = 8;
namespace AscensionFreePick { bool Applies(Player* p) { return p && p->freePick; } }
namespace AscensionMysticEnchant {
bool GrantsActive(Player* p, uint32 enchant) { assert(enchant == 965315); return p->equipped; }
void SetGrantedSpell(Player* p, uint32 enchant, uint32 spell, bool enabled) {
    assert(enchant == 965315); p->grants[spell] = enabled;
}
}
'''
fixture += implementation
bear = (root / 'src/server/coa/Area52LegendaryBatch.inc').read_text()
fixture += 'namespace Area52CorruptedBear\n' + bear.split('namespace Area52CorruptedBear\n')[1].split('class spell_area52_corrupted_bear')[0]
fixture += r'''
int main() {
    using Area52Felfire::Synchronize;
    Synchronize(nullptr);
    Player p;
    p.known = {50796}; p.active = {50796};
    Synchronize(&p);
    assert(p.grants[944811] && !p.grants[944812] && p.replacements[50796] == 944811);
    p.known.insert(59170); p.active = {59170};
    Synchronize(&p);
    assert(p.grants[944812] && p.replacements[59170] == 944812 && p.replacements[50796] == 0);
    p.session.logout = true; p.equipped = false;
    Synchronize(&p);
    assert(p.grants[944811] && p.grants[944812]);
    p.session.logout = false; p.replacements[59170] = 12345;
    Synchronize(&p);
    assert(!p.grants[944811] && !p.grants[944812] && p.replacements[59170] == 12345);
    Player bear; bear.form = FORM_BEAR; bear.auras = {99634};
    Area52CorruptedBear::Synchronize(&bear);
    assert(bear.HasAura(954340) && !bear.HasAura(300600));
    bear.auras.insert(17941); Area52CorruptedBear::Synchronize(&bear);
    assert(bear.HasAura(300600));
    bear.auras.erase(17941); Area52CorruptedBear::Synchronize(&bear);
    assert(!bear.HasAura(300600));
    bear.auras.insert(34936); Area52CorruptedBear::Synchronize(&bear);
    assert(bear.HasAura(300600));
    bear.form = 1; Area52CorruptedBear::Synchronize(&bear);
    assert(!bear.HasAura(954340) && !bear.HasAura(300600));
    bear.form = FORM_DIREBEAR; bear.auras.erase(99634); Area52CorruptedBear::Synchronize(&bear);
    assert(!bear.HasAura(300600));
    Player other; other.freePick = false; other.known = {50796};
    Synchronize(&other); assert(other.grants.empty());
}
'''
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)
    (path / 'ownership.cpp').write_text(fixture)
    subprocess.run(['c++', '-std=c++20', '-Wall', '-Wextra', '-Werror', str(path / 'ownership.cpp'),
                    '-o', str(path / 'ownership')], check=True)
    subprocess.run([str(path / 'ownership')], check=True)
assert database.execute('SELECT spell_id FROM spell_ranks ORDER BY rank').fetchall() == [
    (944811,), (944812,), (944813,), (944814,)]
print('PASS: production replacement synchronization preserves logout grants and follows base ranks and ownership.')
