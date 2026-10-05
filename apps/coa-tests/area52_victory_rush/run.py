import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
MAIN = r'''
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <set>
#include <string>
#include <type_traits>
using uint32 = std::uint32_t;
using int32 = std::int32_t;
using SpellEffIndex = int;
constexpr int EFFECT_0 = 0, SPELL_EFFECT_HEAL_PCT = 136, SPELL_EFFECT_SCHOOL_DAMAGE = 2, CLASS_HERO = 10;
struct Player;
struct Unit { virtual ~Unit() = default; virtual Player* ToPlayer() { return nullptr; } bool IsPlayer() { return ToPlayer(); } };
struct Player : Unit {
    int Class = CLASS_HERO, Health = 400, Maximum = 1000, Casts = 0;
    std::set<uint32> Spells;
    Player* ToPlayer() override { return this; }
    int getClass() const { return Class; }
    bool HasSpell(uint32 spell) const { return Spells.contains(spell); }
    void CastCustomSpell(Player* target, uint32 spell, int32 const* percent, void*, void*, bool triggered) {
        assert(target == this && spell == 965439 && triggered);
        Health = std::min(Maximum, Health + Maximum * *percent / 100); ++Casts;
    }
};
struct ProcEventInfo {};
struct SpellInfo { struct EffectData { int Effect = SPELL_EFFECT_HEAL_PCT; } Effects[1]; } Info;
struct Manager { SpellInfo const* GetSpellInfo(uint32) { return &Info; } } ManagerObject;
auto sSpellMgr = &ManagerObject;
struct Config {
    bool Enabled = true;
    std::string Model = "hero", Realm = "live";
    template<class T> T GetOption(char const* key, T) {
        if constexpr (std::is_same_v<T, bool>) return Enabled;
        else return std::string(key) == "CoA.ClassModel" ? Model : Realm;
    }
} Configuration;
auto sConfigMgr = &Configuration;
struct WorldScript { WorldScript(char const*) {} virtual ~WorldScript() = default; virtual void OnStartup() {} };
struct Event { template<class T> void operator+=(T) {} };
struct AuraScript {
    Unit* Target = nullptr;
    Event DoCheckProc;
    Unit* GetTarget() { return Target; }
    virtual ~AuraScript() = default;
    virtual void Register() {}
};
struct SpellScript {
    Unit* Caster = nullptr; Unit* Target = nullptr;
    Event OnEffectHitTarget;
    Unit* GetCaster() { return Caster; } Unit* GetHitUnit() { return Target; }
    bool ValidateSpellInfo(std::initializer_list<uint32>) { return true; }
    virtual ~SpellScript() = default;
    virtual bool Validate(SpellInfo const*) { return true; }
    virtual void Register() {}
};
#define PrepareAuraScript(name) public:
#define PrepareSpellScript(name) public:
#define AuraCheckProcFn(fn) &fn
#define SpellEffectFn(fn, index, effect) &fn
#define RegisterSpellScript(name) static_cast<void>(sizeof(name))
#include "AscensionVictoryRush.cpp"
int main() {
    AscensionVictoryRushRealm realm; realm.OnStartup();
    Player player, enemy; Unit creature; ProcEventInfo event;
    aura_wildcard_victorious_state aura; aura.Target = &player;
    assert(!aura.KnowsVictoryRush(event));
    for (uint32 id : {34428u, 634428u, 1134428u}) {
        player.Spells = {id}; assert(aura.KnowsVictoryRush(event));
        player.Spells.clear(); assert(!aura.KnowsVictoryRush(event));
    }
    aura.Target = &creature; assert(!aura.KnowsVictoryRush(event));
    spell_area52_victory_rush_heal heal; heal.Caster = &player; heal.Target = &creature;
    assert(heal.Validate(nullptr));
    heal.Heal(0); assert(player.Health == 500 && player.Casts == 1);
    heal.Target = &enemy; heal.Heal(0); assert(player.Health == 580 && player.Casts == 2);
    player.Health = 990; heal.Heal(0); assert(player.Health == 1000);
    Info.Effects[0].Effect = 0; assert(!heal.Validate(nullptr));
    for (int mode = 0; mode < 4; ++mode) {
        Configuration.Enabled = mode != 0;
        Configuration.Model = mode == 1 ? "coa" : "hero";
        Configuration.Realm = mode == 2 ? "seasonal" : "live";
        player.Class = mode == 3 ? 1 : CLASS_HERO;
        realm.OnStartup(); player.Health = 400;
        heal.Heal(0); assert(player.Health == 400);
    }
}
'''


def main():
    source = ROOT / 'src/server/coa/AscensionVictoryRush.cpp'
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261005_00_area52_victory_rush.sql').read_text()
    assert "(32215, 'aura_wildcard_victorious_state')" in sql
    assert "(34428, 'spell_area52_victory_rush_heal')" in sql
    compiler = shutil.which(os.environ.get('CXX', 'c++'))
    assert compiler
    with tempfile.TemporaryDirectory(prefix='area52-victory-rush-') as directory:
        out = Path(directory)
        shutil.copyfile(source, out / source.name)
        for name in ('Config.h', 'Player.h', 'ScriptMgr.h', 'SpellAuraEffects.h', 'SpellInfo.h',
                     'SpellMgr.h', 'SpellScript.h'):
            (out / name).write_text('')
        (out / 'main.cpp').write_text(MAIN)
        executable = out / 'test'
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror', '-I', str(out),
                        str(out / 'main.cpp'), '-o', str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print('PASS: ownership, refund, 10/8 percent heal dispatch, invalid heal shape and mode isolation')


if __name__ == '__main__':
    main()
