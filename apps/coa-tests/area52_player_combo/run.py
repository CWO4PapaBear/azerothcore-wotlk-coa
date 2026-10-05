import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]

GATE = r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
using uint32 = std::uint32_t;
class Player;
struct Unit { virtual ~Unit() = default; virtual Player const* ToPlayer() const { return nullptr; } };
constexpr int CLASS_HERO = 10;
struct Player : Unit {
    int Class = 10;
    bool Alive = true;
    uint32 Points = 0, Saved = 0;
    Player const* ToPlayer() const override { return this; }
    int getClass() const { return Class; }
    bool IsAlive() const { return Alive; }
    struct Setting { uint32 value; };
    Setting GetPlayerSetting(char const*, int) { return {Saved}; }
    void UpdatePlayerSetting(char const*, int, uint32 value) { Saved = value; }
    void SetPlayerComboPoints(uint32 value) { Points = value; }
    uint32 GetComboPoints() const { return Points; }
};
enum Hook { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_SAVE };
struct PlayerScript {
    PlayerScript(char const*, std::initializer_list<Hook>) {}
    virtual ~PlayerScript() = default;
    virtual void OnPlayerLogin(Player*) {}
    virtual void OnPlayerSave(Player*) {}
};
struct WorldScript { WorldScript(char const*) {} virtual ~WorldScript() = default; virtual void OnStartup() {} };
struct Config {
    bool Enabled = true, Feature = true;
    std::string Model = "hero", Realm = "live";
    template<class T> T GetOption(char const* key, T) {
        if constexpr (std::is_same_v<T, bool>) return std::string(key) == "CoA.Enable" ? Enabled : Feature;
        else return std::string(key) == "CoA.ClassModel" ? Model : Realm;
    }
} Configuration;
auto sConfigMgr = &Configuration;
struct Scripts {
    std::optional<uint32> Mask;
    std::optional<uint32> OnPlayerGetGameModeMask(Player const*) { return Mask; }
} ScriptsInstance;
auto sScriptMgr = &ScriptsInstance;
#include "AscensionPlayerComboPoints.cpp"
int main() {
    AscensionPlayerComboPoints::Realm realm;
    AscensionPlayerComboPoints::Heroes hooks;
    Player player;
    Unit creature;
    realm.OnStartup();
    assert(AscensionPlayerComboPoints::Applies(&player));
    assert(!AscensionPlayerComboPoints::Applies(&creature));
    assert(!AscensionPlayerComboPoints::Applies(nullptr));
    ScriptsInstance.Mask = 0x400;
    assert(AscensionPlayerComboPoints::Applies(&player));
    ScriptsInstance.Mask = 1;
    assert(!AscensionPlayerComboPoints::Applies(&player));
    ScriptsInstance.Mask = 0;
    player.Class = 1;
    assert(!AscensionPlayerComboPoints::Applies(&player));
    player.Class = 10;
    for (auto mode : {"seasonal", "unknown"}) {
        Configuration.Realm = mode; realm.OnStartup();
        assert(!AscensionPlayerComboPoints::Applies(&player));
    }
    Configuration.Realm = "live"; Configuration.Model = "coa"; realm.OnStartup();
    assert(!AscensionPlayerComboPoints::Applies(&player));
    Configuration.Model = "hero"; Configuration.Feature = false; realm.OnStartup();
    assert(!AscensionPlayerComboPoints::Applies(&player));
    Configuration.Feature = true; Configuration.Enabled = false; realm.OnStartup();
    assert(!AscensionPlayerComboPoints::Applies(&player));
    Configuration.Enabled = true; realm.OnStartup();
    player.Saved = 99; hooks.OnPlayerLogin(&player); assert(player.Points == 5);
    player.Points = 3; hooks.OnPlayerSave(&player); assert(player.Saved == 3);
    player.Points = 0; hooks.OnPlayerLogin(&player); assert(player.Points == 3);
    player.Alive = false; hooks.OnPlayerLogin(&player); assert(player.Points == 0);
    player.Points = 3; hooks.OnPlayerSave(&player); assert(player.Saved == 0);
}
'''


def function(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def main():
    source = (ROOT / 'src/server/game/Entities/Unit/Unit.cpp').read_text()
    methods = '\n'.join(function(source, name) for name in (
        'void Unit::SetPlayerComboPoints', 'void Unit::AddComboPoints',
        'void Unit::AddTargetComboPoints', 'void Unit::ClearComboPoints',
        'void Unit::ClearTargetComboPoints', 'void Unit::ClearComboPointHolders'))
    header = (ROOT / 'src/server/game/Entities/Unit/Unit.h').read_text()
    getters = '\n'.join(function(header, name) for name in (
        'uint8 GetTargetComboPoints(', 'uint8 GetComboPoints(Unit', 'uint8 GetComboPoints(ObjectGuid'))
    packet = function(source, 'void Unit::SendComboPoints()').split('    ObjectGuid ownerGuid')[0] + '}\n'
    code = r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <unordered_set>
using uint8 = std::uint8_t;
using int8 = std::int8_t;
struct PackedGuid { int Id = 0; int size() const { return 1; } };
struct ObjectGuid {
    int Id = 0;
    PackedGuid WriteAsPacked() const { return {Id}; }
    bool operator==(ObjectGuid const& other) const { return Id == other.Id; }
};
constexpr int SMSG_UPDATE_COMBO_POINTS = 1;
struct WorldPacket {
    int Guid = 0, Points = 0;
    WorldPacket(int, int) {}
    WorldPacket& operator<<(PackedGuid value) { Guid = value.Id; return *this; }
    WorldPacket& operator<<(uint8 value) { Points = value; return *this; }
};
struct Player;
constexpr int SPELL_AURA_RETAIN_COMBO_POINTS = 1;
struct Unit {
    bool Shared = true;
    bool m_cleanupDone = false;
    int Id = 1;
    Unit* m_comboTarget = nullptr;
    int8 m_comboPoints = 0;
    uint8 m_playerComboPoints = 0;
    std::unordered_set<Unit*> m_ComboPointHolders;
    bool HasPlayerComboPoints() const { return Shared; }
    virtual ~Unit() = default;
    virtual Player* ToPlayer() { return nullptr; }
    ObjectGuid GetGUID() const { return {Id}; }
    PackedGuid GetPackGUID() const { return {Id}; }
    void SendComboPoints();
    void RemoveAurasByType(int) {}
    void AddComboPointHolder(Unit* u) { m_ComboPointHolders.insert(u); }
    void RemoveComboPointHolder(Unit* u) { m_ComboPointHolders.erase(u); }
    void SetPlayerComboPoints(uint8);
    void AddComboPoints(Unit*, int8);
    void AddTargetComboPoints(Unit*, int8);
    void ClearComboPoints();
    void ClearTargetComboPoints();
    void ClearComboPointHolders();
GETTERS
};
struct Player : Unit {
    int Selection = 0, SentGuid = -1, SentPoints = -1;
    Player* ToPlayer() override { return this; }
    ObjectGuid GetTarget() const { return {Selection}; }
    void SendDirectMessage(WorldPacket* packet) { SentGuid = packet->Guid; SentPoints = packet->Points; }
};
PACKET
METHODS
int main() {
    Player player;
    Unit first, second;
    first.Id = 2; second.Id = 3;
    player.Selection = first.Id;
    player.AddComboPoints(&first, 3);
    assert(player.SentGuid == 2 && player.SentPoints == 3);
    player.Selection = second.Id;
    player.SendComboPoints();
    assert(player.SentGuid == 3 && player.SentPoints == 3);
    assert(player.GetComboPoints(&second) == 3);
    assert(player.GetComboPoints(second.GetGUID()) == 3);
    assert(first.m_ComboPointHolders.empty());
    player.AddComboPoints(&second, 4);
    assert(player.GetComboPoints() == 5);
    first.ClearComboPointHolders(); second.ClearComboPointHolders();
    assert(player.GetComboPoints() == 5);
    player.AddTargetComboPoints(&first, 1);
    assert(player.GetTargetComboPoints(&first) == 1);
    assert(player.GetTargetComboPoints(&second) == 0);
    player.ClearTargetComboPoints();
    assert(player.GetComboPoints() == 5);
    assert(player.SentGuid == 3 && player.SentPoints == 5);
    player.ClearComboPoints();
    assert(player.GetComboPoints() == 0);
    assert(player.SentPoints == 0);
    player.AddComboPoints(&first, -3);
    assert(player.GetComboPoints() == 0);
    player.SetPlayerComboPoints(255);
    assert(player.GetComboPoints() == 5);
    player.Shared = false;
    player.AddComboPoints(&first, 3);
    assert(player.GetComboPoints(&second) == 0);
    player.AddComboPoints(&second, 1);
    assert(player.GetComboPoints(&second) == 1);
    second.ClearComboPointHolders();
    assert(player.GetComboPoints() == 0);
}
'''.replace('GETTERS', getters).replace('METHODS', methods).replace('PACKET', packet)
    player = (ROOT / 'src/server/game/Entities/Player/Player.cpp').read_text()
    assert 'SendComboPoints();' in function(player, 'void Player::SetSelection')
    assert 'ClearComboPoints();' in function(player, 'void Player::setDeathState')
    storage = (ROOT / 'src/server/game/Entities/Player/PlayerStorage.cpp').read_text()
    assert storage.index('sScriptMgr->OnPlayerSave(this)') < storage.index('_SavePlayerSettings(trans)')
    assert 'ClearTargetComboPoints();' in function(source, 'void Unit::CleanupBeforeRemoveFromMap')
    spell = (ROOT / 'src/server/game/Spells/Spell.cpp').read_text()
    assert 'm_caster->GetTargetComboPoints(m_targets.GetUnitTarget())' in spell
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        (path / 'test.cpp').write_text(code)
        compiler = os.environ.get('CXX') or shutil.which('c++') or shutil.which('g++')
        assert compiler, 'C++ compiler required'
        subprocess.run([compiler, '-std=c++20', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
        subprocess.run([str(path / 'test')], check=True)
        for name in ('Config.h', 'Player.h', 'ScriptMgr.h'):
            (path / name).write_text('')
        (path / 'gate.cpp').write_text(GATE)
        subprocess.run([compiler, '-std=c++20', '-I', str(path), '-I', str(ROOT / 'src/server/coa'),
                        str(path / 'gate.cpp'), '-o', str(path / 'gate')], check=True)
        subprocess.run([str(path / 'gate')], check=True)
    print('Player pool, target cleanup, spend, bounds, reactive isolation and legacy target behavior passed')


if __name__ == '__main__':
    main()
