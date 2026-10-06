#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cmath>
#include <map>
#include <set>
using uint8 = uint8_t;
using uint32 = uint32_t;
using int32 = int32_t;
constexpr uint32 SpearMastery = 901203;
int rolls = 0;
bool proc = true;
bool roll_chance_i(int chance) { assert(chance == 15); ++rolls; return proc; }
DAMAGE_FUNCTION
SCALING_FUNCTION
IMPALED_FUNCTION
struct Player
{
    std::set<uint32> permanent;
    std::set<uint32> temporary;
    std::map<uint32, uint32> replacements;
    bool HasSpell(uint32 id) { return permanent.count(id) || temporary.count(id); }
    bool HasActiveSpell(uint32 id) { return HasSpell(id); }
    void learnSpell(uint32 id, bool temp) { assert(temp); temporary.insert(id); }
    void removeSpell(uint32 id, int, bool temp) { assert(temp); temporary.erase(id); }
    uint32 GetTemporarySpellReplacement(uint32 id) { return replacements.count(id) ? replacements[id] : id; }
    void SetTemporarySpellReplacement(uint32 id, uint32 value)
    { if (value) replacements[id] = value; else replacements.erase(id); }
    int cooldown = 0;
    int buffs = 0;
    Player* ToPlayer() { return this; }
    void ModifySpellCooldown(uint32 id, int32 value) { assert(id == 2687); cooldown += value; }
    void CastSpell(Player* target, uint32 id, bool triggered)
    { assert(target == this && id == SpearMastery && triggered); ++buffs; }
};
constexpr int SPEC_MASK_ALL = 3;
struct Manager
{
    uint32 GetNextSpellInChain(uint32 id) { return id == 10 ? 11 : 0; }
} manager;
Manager* sSpellMgr = &manager;
REPLACE_FUNCTION
struct Strike
{
    uint8 Points = 5;
    bool Resolved = false;
    int damage = 1;
    bool target = true;
    Player player;
    Player* GetCaster() { return &player; }
    Player* GetHitUnit() { return target ? &player : nullptr; }
    int GetHitDamage() { return damage; }
    HIT_FUNCTION
};
int main()
{
    assert(std::abs(FlurryScaling(1, 1000, 500) * 7 - 243) < 0.01f);
    assert(std::abs(FlurryScaling(5, 1000, 500) * 7 - 1215) < 0.01f);
    assert(FlurryScaling(8, 1000, 500) == FlurryScaling(5, 1000, 500));
    assert(ImpaledBonus(12000, 12000, false) == 0);
    assert(ImpaledBonus(12000, 9100, false) == 40);
    assert(ImpaledBonus(12000, 9000, false) == 60);
    assert(ImpaledBonus(12000, 9000, true) == 15);
    assert(ImpaledBonus(12000, -5000, false) == 240);
    assert(ImpaledBonus(12000, 13000, false) == 0);
    assert(ShieldStrikeDamage(1, 1000) == 1099);
    assert(ShieldStrikeDamage(5, 1000) == 2143);
    assert(ShieldStrikeDamage(5, 0) == 1123);
    assert(ShieldStrikeDamage(9, 1000) == ShieldStrikeDamage(5, 1000));
    for (uint8 points = 1; points <= 5; ++points)
    {
        Strike s; s.Points = points; int before = rolls;
        s.Hit(); s.Hit();
        assert(rolls == before + 1 && s.player.buffs == 1);
        assert(s.player.cooldown == -2000 * points);
    }
    for (int scenario = 0; scenario < 3; ++scenario)
    {
        Strike s;
        if (scenario == 0) s.damage = 0;
        if (scenario == 1) s.target = false;
        if (scenario == 2) s.Points = 0;
        int before = rolls; s.Hit();
        assert(rolls == before && s.player.cooldown == 0 && s.player.buffs == 0);
    }
    Player p;
    p.permanent.insert(11);
    ReplaceChain(&p, 10, 99, true);
    assert(p.HasSpell(99) && p.GetTemporarySpellReplacement(11) == 99);
    assert(p.GetTemporarySpellReplacement(10) == 10);
    ReplaceChain(&p, 10, 99, true);
    assert(p.temporary.size() == 1);
    ReplaceChain(&p, 10, 99, false);
    assert(!p.HasSpell(99) && p.HasSpell(11) && p.replacements.empty());
    p.permanent.insert(99);
    ReplaceChain(&p, 10, 99, false);
    assert(p.HasSpell(99));
    proc = false;
    Strike s; s.Hit();
    assert(s.player.cooldown == -10000 && s.player.buffs == 0);
}
