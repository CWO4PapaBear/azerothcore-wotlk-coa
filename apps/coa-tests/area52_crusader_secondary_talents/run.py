from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]

def block(source, marker):
    start = source.index(marker)
    brace = source.index("{", start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]

source = (ROOT / "src/server/coa/AscensionHeroPaladin.cpp").read_text()
vindication = block(source[source.index("class aura_area52_vindication"):], "bool CheckProc(")
swift = source[source.index("class aura_area52_swift_retribution_party"):]
helper = block(swift, "uint32 Helper()")
apply = block(swift, "void Apply(")
remove = block(swift, "void Remove(")
manager = (ROOT / "src/server/game/Spells/SpellMgr.cpp").read_text()
proc = block(manager[manager.index("SpellProcEntry const* SpellMgr::GetSpellProcEntry"):], "if ((spellId == 9452")
assert "RegisterSpellScript(aura_area52_vindication)" in source
assert "RegisterSpellScript(aura_area52_swift_retribution_party)" in source
cleanup = (ROOT / "src/server/coa/AscensionFreePick.cpp").read_text()
for parent, child in ((53379,853379),(53484,853484),(53648,853648),(53379,965715),(53484,965716),(53648,965717)):
    assert "{" + str(parent) + ", " + str(child) + "}" in cleanup
code = r"""
#include <cassert>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <set>
using uint32=uint32_t;
using AuraEffectHandleModes=unsigned;
struct AuraEffect {};
constexpr unsigned SPELL_SCHOOL_MASK_HOLY=2,SPELLFAMILY_PALADIN=10,SPELL_DAMAGE_CLASS_MELEE=2;
constexpr unsigned PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS=16,PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS=256,PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG=65536,PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG=4096;
constexpr unsigned PROC_SPELL_TYPE_DAMAGE=1,PROC_SPELL_PHASE_HIT=2,PROC_HIT_NORMAL=1,PROC_HIT_CRITICAL=2,PROC_ATTR_TRIGGERED_CAN_PROC=2,EFFECT_1=1,EFFECT_2=2;
struct SpellProcEntry {unsigned ProcFlags=0,SpellTypeMask=0,SpellPhaseMask=0,HitMask=0,AttributesMask=0,DisableEffectsMask=0;float Chance=0,ProcsPerMinute=0;};
struct Unit {bool area=true;std::set<unsigned>known;unsigned cast=0,removed=0;Unit* ToPlayer(){return this;}bool HasSpell(unsigned id){return known.contains(id);}unsigned GetGUID(){return 1;}void CastSpell(Unit* target,unsigned id,bool triggered){assert(target==this && triggered);cast=id;}void RemoveAurasDueToSpell(unsigned id,unsigned guid){assert(guid==1);removed=id;}};
namespace AscensionFreePick {bool enabled=true;bool Enabled(){return enabled;}bool Applies(Unit* u){return u && u->area;}}
struct SpellInfo {unsigned School=2,SpellFamilyName=6,DmgClass=1;unsigned GetSchoolMask()const{return School;}};
struct DamageInfo {unsigned damage=1;unsigned GetDamage()const{return damage;}};
struct ProcEventInfo {Unit*actor=nullptr;Unit*target=nullptr;SpellInfo*spell=nullptr;DamageInfo*damage=nullptr;Unit*GetActor(){return actor;}Unit*GetActionTarget(){return target;}SpellInfo const*GetSpellInfo(){return spell;}DamageInfo const*GetDamageInfo(){return damage;}};
struct Probe {Unit owner;unsigned id=53379;Unit*GetTarget(){return &owner;}unsigned GetId()const{return id;}
@@CHECK@@
@@HELPER@@
@@APPLY@@
@@REMOVE@@
};
SpellProcEntry const* GetProc(unsigned spellId){@@PROC@@ return nullptr;}
int main(){
 Probe p;Unit enemy;SpellInfo spell;DamageInfo damage;ProcEventInfo event{&p.owner,&enemy,&spell,&damage};
 assert(p.CheckProc(event));spell.School=1;assert(!p.CheckProc(event));spell.SpellFamilyName=10;spell.DmgClass=2;assert(p.CheckProc(event));spell.SpellFamilyName=6;assert(!p.CheckProc(event));spell.School=2;event.actor=&enemy;assert(!p.CheckProc(event));event.actor=&p.owner;damage.damage=0;assert(!p.CheckProc(event));damage.damage=1;event.damage=nullptr;assert(!p.CheckProc(event));event.damage=&damage;event.spell=nullptr;assert(!p.CheckProc(event));event.spell=&spell;event.target=nullptr;assert(!p.CheckProc(event));event.target=&enemy;
 p.owner.area=false;assert(p.CheckProc(event));p.owner.area=true;
 for(unsigned id:{9452u,26016u,209452u}){auto*row=GetProc(id);assert(row);assert(row->Chance==(id==9452?17:id==26016?34:65));assert(row->ProcsPerMinute==0);assert(row->DisableEffectsMask==6);assert(row->ProcFlags==69904);assert(row->SpellTypeMask==1 && row->SpellPhaseMask==2 && row->HitMask==3);}
 AscensionFreePick::enabled=false;assert(!GetProc(9452));AscensionFreePick::enabled=true;assert(!GetProc(123));
 for(unsigned id:{53379u,53484u,53648u}){p.id=id;p.owner.cast=0;p.owner.removed=0;unsigned helper=id==53379?853379:id==53484?853484:853648;p.Apply(nullptr,0);assert(p.owner.cast==helper);p.Remove(nullptr,0);assert(p.owner.removed==helper);p.owner.removed=0;p.owner.known.insert(helper);p.Remove(nullptr,0);assert(!p.owner.removed);p.owner.known.clear();p.owner.area=false;p.owner.cast=0;p.Apply(nullptr,0);p.Remove(nullptr,0);assert(!p.owner.cast&&!p.owner.removed);p.owner.area=true;}
}
"""
for key, value in (("CHECK",vindication),("HELPER",helper),("APPLY",apply),("REMOVE",remove),("PROC",proc)):
    code = code.replace("@@"+key+"@@",value)
with tempfile.TemporaryDirectory() as directory:
    path=Path(directory)
    (path/"test.cpp").write_text(code)
    subprocess.run(["c++","-std=c++20","-Wall","-Wextra",str(path/"test.cpp"),"-o",str(path/"test")],check=True)
    subprocess.run([str(path/"test")],check=True)
print("PASS: production Vindication event filter and fixed 17/34/65% proc rows; Swift party-helper rank apply/remove and independent grants. SQL, strongest-haste aggregation and saved login remain separate.")
