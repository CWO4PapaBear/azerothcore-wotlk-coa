#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
using uint32 = std::uint32_t;
enum { PLAYERSPELL_NEW, PLAYERSPELL_UNCHANGED, PLAYERSPELL_TEMPORARY, SPEC_MASK_ALL = 3 };
struct Setting { uint32 value = 0; };
struct Spell { int State = PLAYERSPELL_UNCHANGED; };
struct Session
{
    bool logout = false;
    bool PlayerLogout() const { return logout; }
};
struct Player
{
    Session session;
    bool applies = true;
    unsigned learns = 0, removes = 0;
    std::map<uint32, Spell> storage;
    std::map<uint32, Spell*> spells;
    std::map<std::string, std::vector<Setting>> settings;
    Session* GetSession() { return &session; }
    auto const* FindPlayerSettings(std::string const& key) const
    {
        auto it = settings.find(key);
        return it == settings.end() ? nullptr : &it->second;
    }
    auto const& GetSpellMap() const { return spells; }
    bool HasSpell(uint32 spell) const { return spells.contains(spell); }
    void UpdatePlayerSetting(std::string const& key, uint32 index, uint32 value)
    {
        auto& row = settings[key];
        row.resize(std::max(row.size(), std::size_t(index + 1)));
        row[index].value = value;
    }
    void learnSpell(uint32 spell)
    {
        ++learns;
        spells[spell] = &storage[spell];
    }
    void removeSpell(uint32 spell, int mask, bool temporary)
    {
        assert(mask == SPEC_MASK_ALL && !temporary);
        ++removes;
        spells.erase(spell);
    }
};
bool Applies(Player* player) { return player->applies; }
namespace AscensionFreePick
{
struct SpellLearningScope
{
    SpellLearningScope(Player*, std::set<uint32> const&) { }
};
}
GRANT_FUNCTION
int main()
{
    Player p;
    SetGrantedSpell(&p, 1, 100, true);
    assert(p.HasSpell(100) && p.spells[100]->State == PLAYERSPELL_NEW);
    SetGrantedSpell(&p, 1, 100, true);
    assert(p.learns == 1 && p.removes == 0);
    SetGrantedSpell(&p, 2, 100, true);
    SetGrantedSpell(&p, 1, 100, false);
    assert(p.HasSpell(100) && p.removes == 0);
    p.session.logout = true;
    SetGrantedSpell(&p, 2, 100, false);
    assert(p.HasSpell(100) && p.removes == 0);
    p.session.logout = false;
    SetGrantedSpell(&p, 2, 100, true);
    assert(p.learns == 1 && p.removes == 0);
    SetGrantedSpell(&p, 2, 100, false);
    assert(!p.HasSpell(100) && p.removes == 1);
    p.learnSpell(101);
    SetGrantedSpell(&p, 1, 101, true);
    SetGrantedSpell(&p, 1, 101, false);
    assert(p.HasSpell(101) && p.removes == 1);
    p.learnSpell(102);
    p.spells[102]->State = PLAYERSPELL_TEMPORARY;
    unsigned const before = p.learns;
    SetGrantedSpell(&p, 1, 102, true);
    assert(p.spells[102]->State == PLAYERSPELL_NEW && p.learns == before && p.removes == 1);
    SetGrantedSpell(&p, 1, 102, false);
    assert(!p.HasSpell(102) && p.removes == 2);
    p.applies = false;
    SetGrantedSpell(&p, 1, 103, true);
    assert(!p.HasSpell(103));
    SetGrantedSpell(nullptr, 1, 103, true);
}
