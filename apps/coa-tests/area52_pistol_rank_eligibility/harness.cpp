#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "AscensionMysticSpellGrants.h"
using uint32 = std::uint32_t;
enum { PLAYERSPELL_NEW, PLAYERSPELL_UNCHANGED, PLAYERSPELL_TEMPORARY, SPEC_MASK_ALL = 3 };
struct Setting { uint32 value = 0; };
struct Spell { int State = PLAYERSPELL_UNCHANGED; };
struct Session
{
    bool logout = false;
    bool PlayerLogout() const { return logout; }
};
using ActionButtonType = unsigned char;
struct ActionButton {
    uint32 action = 0;
    ActionButtonType type = 0;
    void SetActionAndType(uint32 a, ActionButtonType t) { action=a; type=t; }
};
struct Player
{
    Session session;
    bool applies = true;
    unsigned level = 80;
    bool IsInWorld() const { return true; }
    bool IsAlive() const { return true; }
    unsigned GetLevel() const { return level; }
    void SaveToDB(bool, bool) { }
    bool IsActionButtonDataValid(unsigned char, uint32, unsigned char);
    std::map<unsigned char, ActionButton> m_actionButtons;
    ActionButton* addActionButton(unsigned char, uint32, unsigned char);
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

bool Applies(Player const* p) { return p->applies; }
struct RankUpgrade { uint32 Spell, Previous, Level; };
struct Selected { unsigned EntryId, Rank; };
struct Entry { std::vector<uint32> Spells; bool TE = false, AE = true; };
struct { std::map<unsigned, Entry> Entries; } Data;
std::vector<Selected> KnownEntries(Player const*) { return {}; }
void ExpandGrantedSpells(std::set<uint32>&, bool) { }
struct SpellInfo { unsigned BaseLevel = 0, SpellLevel = 0; };
using SpellEntry = SpellInfo;
struct Manager {
    std::map<uint32, SpellInfo> spells;
    std::map<uint32, uint32> next;
    auto GetSpellInfo(uint32 id) const { auto i = spells.find(id); return i == spells.end() ? nullptr : &i->second; }
    auto LookupEntry(uint32 id) const { return GetSpellInfo(id); }
    uint32 GetNextSpellInChain(uint32 id) { return next[id]; }
    uint32 GetFirstSpellInChain(uint32) { return 997048; }
    bool GetItemTemplate(uint32) { return true; }
} manager, sSpellStore;
auto* sSpellMgr = &manager;
auto* sObjectMgr = &manager;
namespace AscensionMysticEnchant {
bool GrantsActive(Player const* p, uint32 e) { return p->applies && e == 997047; }
void SetGrantedSpell(Player* p, uint32 e, uint32 s, bool on) { ::SetGrantedSpell(p,e,s,on,true); }
}
using AscensionFreePick::SpellLearningScope;
using uint8 = unsigned char;
constexpr unsigned MAX_ACTION_BUTTONS = 144, MAX_ACTION_BUTTON_ACTION_VALUE = 0x1000000;
constexpr unsigned ACTION_BUTTON_SPELL = 0, ACTION_BUTTON_ITEM = 128;
#define LOG_ERROR(...) ((void)0)
#define LOG_DEBUG(...) ((void)0)
RANK_FUNCTIONS
ACTION_FUNCTION
ADD_ACTION_FUNCTION
int main() {
    std::vector<uint32> originals{1752,1757,1758,1759,1760,8621,11293,11294,26861,26862,48637,48638};
    std::vector<uint32> pistols{997048,998100,998101,998102,998103,998104,998105,998106,998107,998108,998109,998110};
    std::vector<unsigned> levels{1,6,14,22,30,38,46,54,62,70,76,80};
    for (unsigned i=0;i<pistols.size();++i) {
        manager.spells[originals[i]] = {levels[i],levels[i]};
        manager.spells[pistols[i]] = {i==3 ? 20u : levels[i],0};
        if (i) manager.next[pistols[i-1]]=pistols[i];
    }
    sSpellStore.spells=manager.spells;
    for (unsigned i=0;i<pistols.size();++i) for (bool restoredFirst : {false,true}) {
        Player p; p.level=levels[i];
        for (unsigned j=0;j<=i;++j) p.learnSpell(originals[j]);
        auto restore = [&] {
            for (unsigned j=0;j<pistols.size();++j) {
                auto* g=AscensionMysticEnchant::PistolRankGrant(pistols[j]);
                assert(g && g->Requires==originals[j]);
                bool eligible=AscensionMysticEnchant::ExactGrantPrerequisiteMet(*g,[&](uint32 s){return p.HasSpell(s);});
                SetGrantedSpell(&p,997047,pistols[j],eligible,true);
            }
        };
        if (restoredFirst) restore();
        else SetGrantedSpell(&p,997047,pistols[0],true,true);
        LearnAvailableRanks(&p); restore();
        assert(p.addActionButton(60,originals[i],0));
        assert(p.addActionButton(61,pistols[i],0));
        unsigned removed=p.removes, learned=p.learns;
        for (unsigned repeat=0;repeat<3;++repeat) { LearnAvailableRanks(&p); restore(); }
        assert(p.removes==removed && p.learns==learned);
        assert(p.m_actionButtons.at(60).action==originals[i]);
        assert(p.m_actionButtons.at(61).action==pistols[i]);
        auto savedBars=p.m_actionButtons;
        p.m_actionButtons.clear();
        restore();
        for (auto const& [slot,bar]:savedBars) assert(p.addActionButton(slot,bar.action,bar.type));
        assert(p.m_actionButtons.at(60).action==originals[i]);
        assert(p.m_actionButtons.at(61).action==pistols[i]);
        assert(p.IsActionButtonDataValid(60,originals[i],0));
        assert(p.IsActionButtonDataValid(61,pistols[i],0));
        for (unsigned j=0;j<pistols.size();++j) assert(p.HasSpell(pistols[j])==(j<=i));
        for (unsigned j=0;j<pistols.size();++j) SetGrantedSpell(&p,997047,pistols[j],false,true);
        assert(!p.HasSpell(pistols[i]) && p.HasSpell(originals[i]));
    }
    for (unsigned i=1;i<pistols.size();++i) {
        Player below; below.level=levels[i]-1;
        for (unsigned j=0;j<=i;++j) below.learnSpell(originals[j]);
        for (unsigned j=0;j<i;++j) SetGrantedSpell(&below,997047,pistols[j],true,true);
        assert(!LearnRankUpgrade(&below,pistols[i],false));
        below.level=levels[i];
        assert(LearnRankUpgrade(&below,pistols[i],false));
    }
    Player lil; lil.level=20;
    for (unsigned i=0;i<3;++i) { lil.learnSpell(originals[i]); SetGrantedSpell(&lil,997047,pistols[i],true,true); }
    assert(!LearnRankUpgrade(&lil,998102,false));
    assert(LearnAvailableRanks(&lil)==0 && lil.removes==0);
    assert(lil.IsActionButtonDataValid(60,1758,0));
    lil.level=22; lil.learnSpell(1759);
    assert(LearnRankUpgrade(&lil,998102,false));
    assert(lil.IsActionButtonDataValid(61,998102,0));
    lil.applies=false; assert(RankUpgrades(&lil).empty());
    Player independent; independent.learnSpell(998102);
    SetGrantedSpell(&independent,997047,998102,true,true);
    SetGrantedSpell(&independent,997047,998102,false,true);
    assert(independent.HasSpell(998102));
}
