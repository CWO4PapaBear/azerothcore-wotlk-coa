from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
freepick = (root / 'src/server/coa/AscensionFreePick.cpp').read_text()
header = (root / 'src/server/coa/AscensionFreePick.h').read_text()
rules = (root / 'src/server/coa/AscensionFreePickRules.cpp').read_text()
book = (root / 'modules/mod-spellbook/src/spellbook.cpp').read_text()
trainer = (root / 'src/server/game/Entities/Creature/Trainer.cpp').read_text()

def method(text, signature):
    start = text.index(signature)
    end = text.index('{', start) + 1
    depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]

code = r'''
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
using uint32=std::uint32_t;
constexpr uint32 SPEC_MASK_ALL=255, SPELL_EFFECT_LEARN_SPELL=36, SPELL_EFFECT_TRIGGER_SPELL=64;
struct Setting { uint32 value=0; };
struct Player {
    bool applies=true,alive=true,inWorld=true;
    uint32 level=1,saves=0;
    std::set<uint32> spells,quests,enchants,racials;
    std::map<std::string,std::vector<Setting>> settings;
    bool HasSpell(uint32 s) const { return spells.contains(s); }
    bool IsInWorld() const { return inWorld; }
    bool IsAlive() const { return alive; }
    uint32 GetLevel() const { return level; }
    void learnSpell(uint32);
    void removeSpell(uint32 s,uint32,bool) { spells.erase(s); }
    void SaveToDB(bool,bool) { ++saves; }
    auto const& getRewardedQuests() const { return quests; }
    auto FindPlayerSettings(std::string const& key) const -> std::vector<Setting> const* {
        auto it=settings.find(key); return it==settings.end()?nullptr:&it->second;
    }
};
struct Effect { uint32 type=0,TriggerSpell=0; bool IsEffect(uint32 t) const { return type==t; } };
struct SpellInfo {
    uint32 BaseLevel=0,SpellLevel=0;
    std::vector<Effect> effects;
    auto const& GetEffects() const { return effects; }
};
struct SpellMgr {
    std::map<uint32,uint32> next,first;
    std::map<uint32,SpellInfo> infos;
    uint32 GetNextSpellInChain(uint32 id) { return next[id]; }
    uint32 GetFirstSpellInChain(uint32 id) { return first.contains(id)?first[id]:id; }
    SpellInfo const* GetSpellInfo(uint32 id) { return infos.contains(id)?&infos.at(id):nullptr; }
} manager;
auto* sSpellMgr=&manager;
struct Quest {
    int cast=0; uint32 display=0;
    int GetRewSpellCast() const { return cast; }
    uint32 GetRewSpell() const { return display; }
};
struct ObjectMgr {
    std::map<uint32,Quest> quests;
    Quest const* GetQuestTemplate(uint32 id) { return quests.contains(id)?&quests.at(id):nullptr; }
} objects;
auto* sObjectMgr=&objects;
namespace AscensionMysticEnchant {
struct Grant { uint32 Enchant,Spell; };
std::vector<Grant> SpellGrants={{900,300}};
bool GrantsActive(Player const* p,uint32 e) { return p->enchants.contains(e); }
void SetGrantedSpell(Player* p,uint32,uint32 s,bool) { p->spells.insert(s); }
}
namespace AscensionFreePick {
struct Entry { uint32 AE,TE; std::vector<uint32> Spells; };
struct KnownEntry { uint32 EntryId,Rank; };
struct Catalog { std::map<uint32,Entry> Entries; } Data;
struct Linked { uint32 Parent; std::array<uint32,2> Children; };
std::vector<Linked> LinkedSpellGroups={{100,{200,0}}};
std::set<uint32> ClassSpells={100,101,102,200,201,300,301,400,401,500,501,600};
std::map<Player const*,std::vector<KnownEntry>> selections;
auto KnownEntries(Player const* p) { return selections[p]; }
bool Applies(Player const* p) { return p && p->applies; }
bool RacialSpell(Player const* p,uint32 s) { return p->racials.contains(s); }
'''
code += method(header, 'class SpellLearningScope') + ';\n'
code += rules[rules.index('thread_local SpellLearningScope'):rules.index('\nnamespace\n{')]
for signature in ('void ExpandGrantedSpells(', 'std::set<std::uint32_t> SelectedSpells(', 'bool CanLearnSpell(',
                  'bool RemoveQuestGrantedClassSpells('):
    code += method(freepick, signature) + '\n'
code += method(header, 'struct RankUpgrade') + ';\n'
code += 'bool LearnRankUpgrade(Player*,std::uint32_t,bool save=true);\n'
for signature in ('std::vector<RankUpgrade> RankUpgrades(', 'bool LearnRankUpgrade(', 'std::uint32_t LearnAvailableRanks('):
    code += method(freepick, signature) + '\n'
code += r'''
}
void Player::learnSpell(uint32 s) {
    if (AscensionFreePick::CanLearnSpell(this,s,false,HasSpell(s))) spells.insert(s);
}
int main() {
    using namespace AscensionFreePick;
    Player p,other;
    Data.Entries[1]={1,0,{100}};
    Data.Entries[2]={0,1,{500}};
    manager.next={{100,101},{101,102},{200,201},{300,301},{400,401}};
    manager.first={{101,100},{102,100},{201,200},{301,300},{401,400}};
    for (uint32 id : {100u,101u,102u,200u,201u,300u,301u,400u,401u,500u,501u,600u})
        manager.infos[id]={0,8,{}};
    manager.infos[102].SpellLevel=16;
    manager.infos[500].effects={{36,501}};
    selections[&p]={{1,1},{2,1}};
    p.spells={100,200,500,501,300};
    p.enchants={900};
    assert(!CanLearnSpell(&p,400,false,false));
    assert(!CanLearnSpell(&p,400,true,false));
    assert(!CanLearnSpell(&other,100,true,false));
    assert(CanLearnSpell(&p,200,true,false));
    assert(CanLearnSpell(&p,501,true,false));
    assert(CanLearnSpell(&p,777,false,false));
    p.racials={600};
    assert(CanLearnSpell(&p,600,false,false));
    {
        std::set<uint32> allowed={400}; SpellLearningScope scope(&p,allowed);
        assert(CanLearnSpell(&p,400,false,false));
        assert(!CanLearnSpell(&other,400,false,false));
    }
    assert(!CanLearnSpell(&p,400,false,false));
    auto ranks=RankUpgrades(&p);
    assert(ranks.size()==4);
    assert(RankUpgrades(&other).empty());
    assert(!LearnRankUpgrade(&p,101));
    p.level=8;
    assert(LearnAvailableRanks(&p)==3);
    assert(p.HasSpell(101) && p.HasSpell(201) && p.HasSpell(301) && !p.HasSpell(102));
    assert(LearnAvailableRanks(&p)==0);
    p.level=16;
    assert(LearnAvailableRanks(&p)==1 && p.HasSpell(102));
    assert(!LearnRankUpgrade(&p,400));
    manager.infos[9000].effects={{64,9001}};
    manager.infos[9001].effects={{36,400},{36,100},{36,300},{36,600},{36,777}};
    objects.quests[1]={9000,0};
    p.quests={1};
    p.spells.insert({400,401,600,777});
    assert(RemoveQuestGrantedClassSpells(&p));
    assert(!p.HasSpell(400) && !p.HasSpell(401));
    for (uint32 id : {100u,101u,102u,200u,201u,300u,301u,500u,501u,600u,777u}) assert(p.HasSpell(id));
    assert(!RemoveQuestGrantedClassSpells(&p));
    p.spells.insert(400);p.settings["core.freepick.mystic.grant.400"]={{1},{900}};
    assert(!RemoveQuestGrantedClassSpells(&p) && p.HasSpell(400));
    p.enchants.clear();assert(RemoveQuestGrantedClassSpells(&p) && !p.HasSpell(400));
    other.applies=false;other.spells={400};other.quests={1};
    assert(CanLearnSpell(&other,400,false,false));
    assert(!RemoveQuestGrantedClassSpells(&other) && other.HasSpell(400));
}
'''
with tempfile.TemporaryDirectory() as directory:
    out = Path(directory)
    (out / 'test.cpp').write_text(code)
    subprocess.run(['c++', '-std=c++20', '-Wall', '-Wextra', '-Werror', str(out / 'test.cpp'), '-o', str(out / 'test')], check=True)
    subprocess.run([str(out / 'test')], check=True)
summon = method(book, 'void OnCreatureAddWorld(')
level = method(book, 'void OnPlayerLevelChanged(')
opened = method(book, 'bool OpenTrainer(')
for hook in (summon, level):
    assert 'LearnFreePickBookRanks(' in hook and 'SendTrainerWindow' not in hook
assert 'GetOwnerGUID() != player->GetGUID()' in level
assert 'player->GetLevel() <= oldLevel' in level
assert 'SendTrainerList(book)' in opened and 'rank.Level >= 10' in trainer
assert 'LearnAvailableRanks(player)' in method(book, 'uint32 LearnFreePickBookRanks(')
assert 'SendPlaySpellVisual(179)' in method(book, 'uint32 LearnFreePickBookRanks(')
assert '(!WildcardRankRowsOf && !freepick) || !IsRankTrainerHero(player)' in trainer
assert 'if (_trainerId == WILDCARD_RANK_TRAINER_ID && AscensionFreePick::Applies(player))' in method(trainer, 'void Trainer::TeachSpell(')
assert 'AscensionFreePick::LearnRankUpgrade(player, spellId)' in trainer
assert 'row.ReqAbility[0] = rank.Previous' in trainer
assert '_trainerId != WILDCARD_RANK_TRAINER_ID || !AscensionFreePick::Applies(player)' in trainer
print('PASS: production rank traversal, guarded learning, quest cleanup and trainer/book hook wiring. No gameplay claim.')
