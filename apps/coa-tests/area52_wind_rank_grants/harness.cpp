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
    uint32 GetFirstSpellInChain(uint32 id) { for (auto const& [a,b]:next) if (b==id) return GetFirstSpellInChain(a); return id; }
    bool GetItemTemplate(uint32) { return true; }
} manager, sSpellStore;
auto* sSpellMgr = &manager;
auto* sObjectMgr = &manager;
namespace AscensionMysticEnchant {
bool GrantsActive(Player const* p, uint32 e) { return p->applies && (e == 978232 || e == 997082); }
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

struct GrantRow {uint32 enchant,spell,required,replaces,level;};
int main(){
std::vector<GrantRow> rows={{978232,978945,17364,0,1},{978232,979920,17373,0,10},{978232,979921,17374,0,20},{978232,979922,17375,0,30},{978232,979923,17376,0,40},{978232,979924,17377,0,50},{978232,979925,17378,0,60},{978232,979926,17379,0,70},{978232,983982,16914,0,40},{978232,984046,17401,0,50},{978232,984047,17402,0,60},{978232,984048,27012,0,70},{978232,984049,48467,0,80},{997082,997083,403,403,1},{997082,997085,529,529,8},{997082,997086,548,548,14},{997082,997087,915,915,20},{997082,997088,943,943,26},{997082,997089,6041,6041,32},{997082,997090,10391,10391,38},{997082,997091,10392,10392,44},{997082,997092,15207,15207,50},{997082,997093,15208,15208,56},{997082,997094,25448,25448,62},{997082,997095,25449,25449,67},{997082,997096,49237,49237,73},{997082,997097,49238,49238,79}};
for(auto const& row:rows){manager.spells[row.required]={row.level,row.level};manager.spells[row.spell]={1,1};}
for(unsigned i=1;i<rows.size();++i)if(rows[i].enchant==rows[i-1].enchant && !(rows[i].spell==983982))manager.next[rows[i-1].spell]=rows[i].spell;
sSpellStore.spells=manager.spells;
for(auto const& row:rows)for(bool restoredFirst:{false,true}){
 Player p;p.level=row.level;p.learnSpell(row.required);
 auto restore=[&]{for(auto const& grant:AscensionMysticEnchant::SpellGrants)if(grant.Enchant==row.enchant)
 SetGrantedSpell(&p,grant.Enchant,grant.Spell,AscensionMysticEnchant::ExactGrantPrerequisiteMet(grant,[&](uint32 x){return p.HasSpell(x);}),true);};
 if(restoredFirst)restore();LearnAvailableRanks(&p);restore();
 assert(p.HasSpell(row.spell));assert(p.addActionButton(61,row.spell,0));
 auto learned=p.learns,removed=p.removes;
 for(unsigned i=0;i<3;++i){LearnAvailableRanks(&p);restore();}
 assert(p.learns==learned && p.removes==removed);
 auto bars=p.m_actionButtons;p.m_actionButtons.clear();restore();for(auto const& [slot,bar]:bars)assert(p.addActionButton(slot,bar.action,bar.type));
 assert(p.m_actionButtons.at(61).action==row.spell);
 SetGrantedSpell(&p,row.enchant,row.spell,false,true);assert(!p.HasSpell(row.spell)&&p.HasSpell(row.required));
 Player independent;independent.learnSpell(row.spell);SetGrantedSpell(&independent,row.enchant,row.spell,true,true);SetGrantedSpell(&independent,row.enchant,row.spell,false,true);assert(independent.HasSpell(row.spell));
}
}
