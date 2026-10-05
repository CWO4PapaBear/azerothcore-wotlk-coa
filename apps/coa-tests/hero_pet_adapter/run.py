import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
MAIN = r'''
#include <array>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <set>
#include <string>
using uint32 = std::uint32_t;
template<class T> using Optional = std::optional<T>;
enum Classes { CLASS_HUNTER = 3, CLASS_WARLOCK = 9, CLASS_HERO = 10 };
enum ClassContext { CLASS_CONTEXT_PET, CLASS_CONTEXT_PET_CHARM, CLASS_CONTEXT_STATS };
enum PetType { SUMMON_PET, HUNTER_PET };
enum Hook { PLAYERHOOK_ON_PLAYER_IS_CLASS, PLAYERHOOK_ON_BEFORE_GUARDIAN_INIT_STATS_FOR_LEVEL,
    PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LEARN_SPELL, PLAYERHOOK_ON_FORGOT_SPELL };
constexpr int SPEC_MASK_ALL = 3;
std::set<uint32> const* Authorized = nullptr;
struct Player {
    Classes Class = CLASS_HERO;
    std::set<uint32> Spells;
    Classes getClass() const { return Class; }
    bool HasSpell(uint32 id) const { return Spells.contains(id); }
    void learnSpell(uint32 id) { assert(Authorized && Authorized->contains(id)); Spells.insert(id); }
    void removeSpell(uint32 id, int, bool) { Spells.erase(id); }
};
struct Pet { PetType Type = HUNTER_PET; PetType getPetType() { return Type; } };
struct Guardian { bool PetObject = true; Pet Saved; bool IsPet() { return PetObject; } Pet* ToPet() { return &Saved; } };
struct CreatureTemplate {};
struct PlayerScript {
    PlayerScript(char const*, std::initializer_list<Hook>) {}
    virtual ~PlayerScript() = default;
    virtual void OnPlayerLogin(Player*) {}
    virtual void OnPlayerLearnSpell(Player*, uint32) {}
    virtual void OnPlayerForgotSpell(Player*, uint32) {}
    virtual Optional<bool> OnPlayerIsClass(Player const*, Classes, ClassContext) { return {}; }
    virtual void OnPlayerBeforeGuardianInitStatsForLevel(Player*, Guardian*, CreatureTemplate const*, PetType&) {}
};
struct WorldScript { WorldScript(char const*) {} virtual ~WorldScript() = default; virtual void OnStartup() {} };
struct Config {
    bool Enable = true;
    std::string Model = "hero", Realm = "live";
    template<class T> T GetOption(char const* key, T) {
        if constexpr (std::is_same_v<T, bool>) return Enable;
        else return std::string(key) == "CoA.ClassModel" ? Model : Realm;
    }
} Configuration;
auto sConfigMgr = &Configuration;
#include "AscensionHeroPets.cpp"
int main() {
    AscensionHeroPets::Realm realm;
    AscensionHeroPets::Heroes hooks;
    Player player;
    realm.OnStartup();
    assert(AscensionHeroPets::Applies(&player));
    assert(!AscensionHeroPets::Applies(nullptr));
    assert(hooks.OnPlayerIsClass(&player, CLASS_HUNTER, CLASS_CONTEXT_PET) == true);
    assert(hooks.OnPlayerIsClass(&player, CLASS_WARLOCK, CLASS_CONTEXT_PET_CHARM) == true);
    assert(!hooks.OnPlayerIsClass(&player, CLASS_HUNTER, CLASS_CONTEXT_STATS).has_value());
    for (auto const& package : AscensionHeroPets::Packages) {
        player.Spells = {package.Parent};
        hooks.OnPlayerLearnSpell(&player, package.Parent);
        for (auto child : package.Children) if (child) assert(player.HasSpell(child));
        auto expected = player.Spells;
        hooks.OnPlayerLogin(&player);
        assert(expected == player.Spells);
        player.Spells.erase(package.Parent);
        hooks.OnPlayerForgotSpell(&player, package.Parent);
        assert(player.Spells.empty());
    }
    Guardian guardian;
    PetType type = SUMMON_PET;
    hooks.OnPlayerBeforeGuardianInitStatsForLevel(&player, &guardian, nullptr, type);
    assert(type == HUNTER_PET);
    guardian.Saved.Type = SUMMON_PET;
    hooks.OnPlayerBeforeGuardianInitStatsForLevel(&player, &guardian, nullptr, type);
    assert(type == SUMMON_PET);
    for (int mode = 0; mode < 4; ++mode) {
        Configuration.Enable = mode != 0;
        Configuration.Model = mode == 1 ? "coa" : "hero";
        Configuration.Realm = mode == 2 ? "seasonal" : "live";
        player.Class = mode == 3 ? CLASS_HUNTER : CLASS_HERO;
        realm.OnStartup();
        player.Spells = {965200};
        hooks.OnPlayerLogin(&player);
        assert(player.Spells.size() == 1);
        assert(!AscensionHeroPets::Applies(&player));
    }
}
'''
SCOPE = r'''
namespace AscensionFreePick {
struct SpellLearningScope {
    std::set<uint32> const* Previous;
    SpellLearningScope(Player*, std::set<uint32> const& spells) : Previous(Authorized) { Authorized = &spells; }
    ~SpellLearningScope() { Authorized = Previous; }
};
}
'''


def main():
    compiler = shutil.which(os.environ.get('CXX', 'c++'))
    assert compiler
    with tempfile.TemporaryDirectory(prefix='hero-pet-adapter-') as directory:
        out = Path(directory)
        for name in ('AscensionHeroPets.cpp', 'AscensionHeroPets.h'):
            shutil.copyfile(ROOT / 'src/server/coa' / name, out / name)
        for name in ('Config.h', 'Pet.h', 'Player.h', 'ScriptMgr.h'):
            (out / name).write_text('')
        (out / 'AscensionFreePick.h').write_text(SCOPE)
        (out / 'main.cpp').write_text(MAIN)
        executable = out / 'test'
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror', '-I', str(out),
            '-I', str(ROOT / 'src/server/coa'), str(out / 'main.cpp'), '-o', str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print('PASS: five pet packages, scoped grants, removal, mode isolation and captured/summoned pet types')


if __name__ == '__main__':
    main()
