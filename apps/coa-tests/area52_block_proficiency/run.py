from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
source = (root / "src/server/coa/Area52BlockProficiency.h").read_text()
body = source[source.index("namespace Area52BlockProficiency"):source.index("\n#endif")]
stats = (root / "src/server/game/Entities/Unit/StatSystem.cpp").read_text()
start = stats.index("void Player::UpdateBlockPercentage()")
end = stats.index("\nvoid Player::UpdateCritPercentage", start)
calculation = stats[start:end]
code = r"""
#include <cassert>
#include <cstdint>
#include <set>
using uint32 = std::uint32_t;
using int32 = std::int32_t;
constexpr int SPELL_AURA_MOD_BLOCK_PERCENT=51, CR_BLOCK=15, PLAYER_BLOCK_PERCENTAGE=1;
struct Config { template<class T> T GetOption(char const*, T value) { return value; } } config;
auto* sConfigMgr=&config;
struct Player {
    bool area52=true, known=false, block=false;
    unsigned learns=0;
    float chance=0, bonus=0, rating=0;
    bool CanBlock() const { return block; }
    int GetDefenseSkillValue() const { return 400; }
    int GetMaxSkillValueForLevel() const { return 400; }
    float GetTotalAuraModifier(int) const { return bonus; }
    float GetRatingBonusValue(int) const { return rating; }
    void SetStatFloatValue(int, float value) { chance=value; }
    void UpdateBlockPercentage();
    bool HasSpell(uint32 id) const { return id==107 && known; }
    void learnSpell(uint32 id, bool temporary);
};
struct Manager {
    bool exists=true;
    bool GetSpellInfo(uint32 id) const { return id==107 && exists; }
} manager;
auto* sSpellMgr=&manager;
namespace AscensionFreePick {
Player* authorized=nullptr;
bool Applies(Player const* p) { return p && p->area52; }
struct SpellLearningScope {
    SpellLearningScope(Player* p, std::set<uint32> const& spells) { assert(spells==std::set<uint32>{107});authorized=p; }
    ~SpellLearningScope() { authorized=nullptr; }
};
}
void Player::learnSpell(uint32 id, bool temporary) {
    assert(id==107 && !temporary && AscensionFreePick::authorized==this);
    known=true;block=true;++learns;
}
""" + calculation + body + r"""
int main() {
    using Area52BlockProficiency::Synchronize;
    assert(!Synchronize(nullptr));
    Player p;
    assert(!p.block);
    p.bonus=25;p.rating=3;p.UpdateBlockPercentage();
    assert(p.chance==0);
    assert(Synchronize(&p));
    assert(p.block && p.known && p.learns==1);
    p.UpdateBlockPercentage();assert(p.chance==33);
    p.bonus=0;p.rating=0;p.UpdateBlockPercentage();assert(p.chance==5);
    assert(!Synchronize(&p) && p.learns==1);
    Player other;other.area52=false;
    assert(!Synchronize(&other) && !other.known);
    manager.exists=false;Player missing;
    assert(!Synchronize(&missing) && !missing.known);
    assert(AscensionFreePick::authorized==nullptr);
}
"""
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)
    (path / "test.cpp").write_text(code)
    subprocess.run(["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", str(path / "test.cpp"), "-o", str(path / "test")], check=True)
    subprocess.run([str(path / "test")], check=True)
compat = (root / "src/server/coa/AscensionCompat.cpp").read_text()
assert "if (Area52BlockProficiency::Synchronize(player))\n        player->SaveToDB(false, false);" in compat
print("PASS: Area 52-only permanent Block grant, explicit authorization, idempotence and login/save wiring.")
