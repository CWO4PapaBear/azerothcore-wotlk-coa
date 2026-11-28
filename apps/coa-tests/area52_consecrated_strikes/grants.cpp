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
    std::map<unsigned,unsigned> actions;
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
    void learnSpell(uint32 spell, bool = false, bool = false, bool = true)
    {
        ++learns;
        spells[spell] = &storage[spell];
    }
    void removeSpell(uint32 spell, int mask, bool temporary)
    {
        assert(mask == SPEC_MASK_ALL && !temporary);
        ++removes;
        spells.erase(spell);
        std::erase_if(actions,[spell](auto const& pair){return pair.second==spell;});
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
    for (unsigned spell : {278745u})
    {
        Player p;
        SetGrantedSpell(&p,965295,spell,true,true);
        p.actions[7]=spell;
        p.spells[spell]->State=PLAYERSPELL_UNCHANGED;
        p.session.logout=true;
        SetGrantedSpell(&p,965295,spell,false,true);
        assert(p.actions.at(7)==spell && p.HasSpell(spell));
        p.session.logout=false;
        for(unsigned repeat=0;repeat<3;++repeat)
            SetGrantedSpell(&p,965295,spell,true,true);
        assert(p.actions.at(7)==spell && p.learns==1 && p.removes==0);
        SetGrantedSpell(&p,999999,spell,true,true);
        SetGrantedSpell(&p,965295,spell,false,true);
        assert(p.actions.at(7)==spell);
        SetGrantedSpell(&p,999999,spell,false,true);
        assert(!p.HasSpell(spell) && p.actions.empty());
        Player independent;
        independent.learnSpell(spell);
        independent.actions[3]=spell;
        SetGrantedSpell(&independent,965295,spell,true,true);
        SetGrantedSpell(&independent,965295,spell,false,true);
        assert(independent.HasSpell(spell) && independent.actions.at(3)==spell);
    }
}
