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
using SpellEntry=SpellInfo;struct Store{SpellEntry const* LookupEntry(unsigned){return nullptr;}}sSpellStore;
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
struct Grant { uint32 Enchant,Spell,Requires=0; };
Grant const* PistolRankGrant(uint32) { return nullptr; }
template<class F> bool ExactGrantPrerequisiteMet(Grant const&,F const&) { return false; }
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
code += 'std::set<uint32> TalentSpells;bool IsTalentRank(Player const* p,uint32 id){return Applies(p) && TalentSpells.contains(id);} std::set<uint32> AuthorizedTalentSpells(Player const*); std::vector<uint32> SpecializationSpells;\n'
code += method(freepick, 'std::uint32_t LinkedSpellRequiredLevel(') + '\n'
for signature in ('void ExpandGrantedSpells(', 'std::set<std::uint32_t> SelectedSpells(', 'bool CanLearnSpell(',
                  'bool RemoveQuestGrantedClassSpells('):
    code += method(freepick, signature) + '\n'
code += 'std::set<uint32> AuthorizedTalentSpells(Player const* p){return SelectedSpells(KnownEntries(p));}\n'
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
    @CASES@
}

'''
import struct,json
def rows(name):
    b=(Path('/data/dbc')/(name+'.dbc')).read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b)
    return [struct.unpack_from('<'+str(z//4)+'I',b,20+i*z) for i in range(n)]
spells={r[0]:r for r in rows('Spell')};chains={}
for r in rows('SpellRank'):chains.setdefault(r[1],{})[r[3]]=r[2]
for first,spell,rank in json.loads(Path(__file__).with_name('database-rank-chains.json').read_text()):
    existing=chains.setdefault(first,{})
    if rank in existing:assert existing[rank]==spell
    else:existing[rank]=spell
cases=[]
advancement={r[0]:r for r in rows("CharacterAdvancement")}
for entry,first,minimum in [(10001, 35512, 1), (1324, 92169, 4), (2955, 20271, 4), (3020, 53408, 4), (3594, 35395, 6), (2656, 17, 10), (2661, 78, 12), (2691, 498, 14), (3735, 53385, 16), (2738, 879, 20), (2940, 19750, 20), (1366, 954707, 22), (3410, 20375, 22), (3043, 818045, 24), (1632, 760053, 28), (3019, 53407, 28), (1369, 954711, 30), (2938, 19742, 30), (2710, 642, 34), (2773, 1706, 34), (1621, 955070, 36), (1608, 954817, 40), (1190, 954500, 41), (2736, 853, 43), (2952, 20217, 44), (1637, 760106, 46), (102, 1152, 48), (3453, 29801, 50), (2885, 7328, 51), (2723, 710, 53), (3042, 801494, 55), (3367, 19506, 57), (3622, 46917, 60), (1312, 92155, 61), (2979, 30482, 61), (2937, 19740, 62), (1314, 92157, 63), (2984, 32223, 64), (3395, 20911, 65), (2762, 1459, 67), (2701, 587, 68), (2983, 31884, 70), (2968, 26573, 72), (2884, 7294, 73), (3647, 47540, 75), (1602, 954803, 77), (2927, 14752, 79), (2939, 19746, 80)]:
    chain=[v for k,v in sorted(chains.get(first,{1:first}).items())]
    assert chain[0]==first and all(x in spells for x in chain)
    setup='Data.Entries.clear();Data.Entries['+str(entry)+']={'+str(advancement[entry][14])+','+str(advancement[entry][15])+',{'+str(first)+'}};ClassSpells={'+','.join(map(str,chain))+'};LinkedSpellGroups.clear();AscensionMysticEnchant::SpellGrants.clear();manager.next.clear();manager.first.clear();manager.infos.clear();'
    for i,spell in enumerate(chain):
        r=spells[spell];setup+='manager.infos['+str(spell)+']={'+str(r[38])+','+str(r[39])+',{}};manager.first['+str(spell)+']='+str(first)+';'
        if i+1<len(chain):setup+='manager.next['+str(spell)+']='+str(chain[i+1])+';'
    setup+='TalentSpells={'+(str(first) if advancement[entry][15] else '')+'};'
    checks=''
    for spell in chain:
        threshold=max(spells[spell][38],spells[spell][39]) if spell!=first else 0
        checks+='assert(p.HasSpell('+str(spell)+')=='+('true' if threshold==0 else '(level>='+str(threshold)+')')+');'
    cases.append('{'+setup+'for(unsigned level='+str(minimum)+';level<=80;++level){Player p;p.level=level;assert(!CanLearnSpell(&p,'+str(first)+',false,false));selections[&p]={{'+str(entry)+',1}};for(int load=0;load<2;++load){p.spells.clear();{auto allowed=SelectedSpells(KnownEntries(&p));SpellLearningScope scope(&p,allowed);p.learnSpell('+str(first)+');}LearnAvailableRanks(&p);'+checks+'assert(LearnAvailableRanks(&p)==0);}selections.erase(&p);assert(!CanLearnSpell(&p,'+str(first)+',false,false));}Player other;other.applies=false;other.level=80;selections[&other]={{'+str(entry)+',1}};assert(LearnAvailableRanks(&other)==0);selections.erase(&other);}')
code=code.replace('@CASES@',''.join(cases))
with tempfile.TemporaryDirectory() as directory:
    out = Path(directory)
    (out / 'test.cpp').write_text(code)
    subprocess.run(['c++', '-std=c++20', '-Wall', '-Wextra', '-Werror', str(out / 'test.cpp'), '-o', str(out / 'test')], check=True)
    subprocess.run([str(out / 'test')], check=True)
print('PASS: all assigned Crusader ordinary and active-talent ability selections and eligible rank chains restore repeatedly through native authorization; this does not test linked helper ownership or live gameplay.')
