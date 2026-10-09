from pathlib import Path
import os
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]

def method(source, signature):
    start = source.index(signature)
    end = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

def main():
    source = (ROOT / 'src/server/coa/AscensionFreePick.cpp').read_text()
    apply = method(source, 'Result Apply(')
    purge = method(source, 'Result Purge(')
    for body in (apply, purge):
        assert 'HasItemCount' not in body and 'DestroyItemCount' not in body
    assert 'RemoveSelectionBuffs(player, oldSpells, newSpells)' in apply
    assert 'PauseAutoLearnForReset(player)' in purge
    guards = apply[:apply.index('    auto const before')] + 'return {}; }'
    code = r"""
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <vector>
using uint32=std::uint32_t;
using int32=std::int32_t;
constexpr int SPELL_LINK_CAST=0, SPELL_LINK_HIT=100000000, SPELL_LINK_AURA=200000000;
constexpr int SPELL_EFFECT_LEARN_SPELL=1, SPELL_EFFECT_TRIGGER_SPELL=2;
struct Effect { uint32 TriggerSpell=0; int Type=0; bool IsEffect(int type) const {return Type==type;} };
struct SpellInfo { uint32 Id=0; std::vector<Effect> effects; auto const& GetEffects() const {return effects;} };
struct Manager {
 std::map<uint32,SpellInfo> spells;
 std::map<int32,std::vector<int32>> links;
 std::map<uint32,uint32> next,first;
 SpellInfo const* GetSpellInfo(uint32 id) {return spells.contains(id)?&spells.at(id):nullptr;}
 uint32 GetFirstSpellInChain(uint32 id) {return first.contains(id)?first.at(id):id;}
 uint32 GetNextSpellInChain(uint32 id) {return next[id];}
 std::vector<int32> const* GetSpellLinked(int32 id) {return links.contains(id)?&links.at(id):nullptr;}
} manager;
auto* sSpellMgr=&manager;
struct Group {uint32 Parent; std::vector<uint32> Children;};
std::vector<Group> LinkedSpellGroups;
struct Aura {uint32 caster=1; SpellInfo const* parent=nullptr; uint32 GetCasterGUID() const{return caster;} auto GetTriggeredByAuraSpellInfo() const{return parent;}};
struct Application {Aura aura; auto GetBase() const{return &aura;}};
struct Player {
 bool world=true,alive=true,combat=false,bg=false,arena=false;
 std::map<uint32,Application*> auras;
 auto GetGUID() const{return 1u;}
 auto const& GetAppliedAuras() const{return auras;}
 void RemoveAurasDueToSpell(uint32 id,uint32 caster) {if(auras.contains(id)&&auras.at(id)->aura.caster==caster)auras.erase(id);}
 bool IsInWorld(){return world;} bool IsAlive(){return alive;} bool IsInCombat(){return combat;}
 bool InBattleground(){return bg;} bool InArena(){return arena;}
};
namespace AscensionMysticEnchant {
 struct Grant {uint32 Enchant,Spell;}; std::vector<Grant> SpellGrants;
 bool GrantsActive(Player*,uint32){return true;}
}
struct Result {const char* Status="OK"; const char* Learn="";};
struct KnownEntry {};
bool Applies(Player*) {return true;}
"""
    for signature in ('void ExpandGrantedSpells(', 'void ExpandBuffSources(', 'void RemoveSelectionBuffs('):
        code += method(source, signature) + '\n'
    code += guards + r"""
int main() {
 Player p;
 for(uint32 id=10;id<=14;++id)manager.spells[id]={id,{}};
 manager.spells[10].effects={{11,2}};
 manager.spells[11].effects={{12,2}};
 manager.spells[12].effects={{10,2}};
 manager.links[SPELL_LINK_AURA+12]={13,-99};
 manager.next[10]=14;manager.first[14]=10;
 manager.spells[20]={20,{{12,2}}};
 Application a,b,c,d,e,other,shared;
 p.auras={{10,&a},{11,&b},{12,&c},{13,&d},{14,&e},{99,&other}};
 other.aura.caster=2;
 RemoveSelectionBuffs(&p,{10},{});
 assert(p.auras.size()==1 && p.auras.contains(99));
 p.auras={{10,&a},{11,&b},{12,&c},{13,&d}};
 RemoveSelectionBuffs(&p,{10},{20});
 assert(p.auras.contains(12)&&p.auras.contains(13));
 manager.spells[30]={30,{}};manager.spells[31]={31,{}};
 a.aura.parent=&manager.spells[10]; b.aura.parent=&manager.spells[30];
 p.auras={{30,&a},{31,&b}};
 RemoveSelectionBuffs(&p,{10},{});assert(p.auras.empty());
 a.aura.parent=nullptr;a.aura.caster=2;p.auras={{10,&a}};
 RemoveSelectionBuffs(&p,{10},{});assert(p.auras.contains(10));
 AscensionMysticEnchant::SpellGrants={{1,10}};
 a.aura.caster=1;p.auras={{10,&a},{12,&c}};
 RemoveSelectionBuffs(&p,{10},{});assert(p.auras.size()==2);
 assert(std::string_view(Apply(&p,{},false).Status)=="OK");
 p.combat=true;assert(std::string_view(Apply(&p,{},false).Learn)=="CA_LEARN_NOT_IN_COMBAT");p.combat=false;
 p.bg=true;assert(std::string_view(Apply(&p,{},false).Learn)=="CA_LEARN_NOT_IN_BATTLEGROUNDS");p.bg=false;
 p.arena=true;assert(std::string_view(Apply(&p,{},false).Learn)=="CA_LEARN_NOT_IN_BATTLEGROUNDS");
}
"""
    code = '#include <string_view>\n' + code
    with tempfile.TemporaryDirectory() as directory:
        path=Path(directory); (path/'test.cpp').write_text(code)
        compiler=os.environ.get('CXX') or shutil.which('g++') or shutil.which('clang++')
        subprocess.run([compiler,'-std=c++20',str(path/'test.cpp'),'-o',str(path/'test')],check=True)
        subprocess.run([str(path/'test')],check=True)
    print('PASS: free individual and bulk removal, combat/BG/arena gates, ranked/nested/linked/provenance buffs, retained and ME ownership, other-caster protection')

if __name__ == '__main__':
    main()
