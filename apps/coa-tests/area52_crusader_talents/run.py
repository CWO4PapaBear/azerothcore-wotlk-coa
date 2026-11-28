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

paladin = (ROOT / "src/server/coa/AscensionHeroPaladin.cpp").read_text()
priest = (ROOT / "src/server/scripts/Spells/spell_priest.cpp").read_text()
unit = (ROOT / "src/server/game/Entities/Unit/Unit.cpp").read_text()
stats = (ROOT / "src/server/game/Entities/Unit/StatSystem.cpp").read_text()
power = block(paladin[paladin.index("class aura_area52_power_of_light"):], "bool CheckProc(")
body = block(priest[priest.index("class spell_pri_body_and_soul"):], "bool CheckProcTriggerSpell(")
metadata = block(paladin, "void ApplyAscensionHeroPaladinContracts(")
healing = block(unit[unit.index("int32 Unit::SpellBaseHealingBonusDone"):], "[this, schoolMask](AuraEffect const* effect)")
crit = block(stats[stats.index("void Player::UpdateCritPercentage"):], "[](AuraEffect const* effect)")
assert "attType == RANGED_ATTACK || !(HasAura(84864) || HasAura(84865))" in stats
assert "value -= GetTotalAuraModifier(SPELL_AURA_MOD_WEAPON_CRIT_PERCENT" in stats
assert "aura_area52_power_of_light" in (ROOT / "data/sql/updates/pending_db_world/rev_20261126_41_area52_crusader_talents.sql").read_text()
source = r"""
#include <cassert>
#include <cstdint>
#include <initializer_list>
using uint32=uint32_t;
using int32=int32_t;
constexpr unsigned EFFECT_0=0,EFFECT_1=1,EFFECT_2=2;
constexpr unsigned SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER=238,SPELL_AURA_MOD_WEAPON_CRIT_PERCENT=52,SPELL_AURA_PROC_TRIGGER_SPELL=42,SPELL_SCHOOL_MASK_ALL=127;
constexpr unsigned FanaticismDebuff=31882,SPELL_AURA_NONE=0;
struct flag96 {unsigned a=0,b=0,c=0;flag96()=default;flag96(unsigned x,unsigned y,unsigned z):a(x),b(y),c(z){};};
flag96 FANATICISM_MASK;
constexpr unsigned SPELL_EFFECT_APPLY_AREA_AURA_FRIEND=128,SPELL_ATTR3_DOT_STACKING_RULE=128;
struct Store {unsigned radius=14,duration=31;unsigned const* LookupEntry(unsigned id){assert(id==14||id==31);return id==14?&radius:&duration;}} sSpellRadiusStore,sSpellDurationStore;
struct TestEffect {unsigned const* RadiusEntry=nullptr;unsigned Effect=6,ApplyAuraName=4,MiscValue=0,TriggerSpell=0;flag96 SpellClassMask;};
struct SpellInfo {unsigned const* DurationEntry=nullptr;unsigned AttributesEx3=0;unsigned Id=0;unsigned SpellFamilyFlags[3]={0,0,0};TestEffect Effects[3];};
namespace Area52MysticRealm {bool active=true;bool UsesFreePickEngine(){return active;}}
struct Unit {bool area=true,str=false,agi=false;Unit* ToPlayer(){return this;}bool HasAura(unsigned id){return id==84864?str:id==84865?agi:false;}};
namespace AscensionFreePick {bool Applies(Unit* u){return u && u->area;}}
struct Spell {bool triggered=false;int time=1000;bool IsTriggered()const{return triggered;}int GetCastTime()const{return time;}};
struct HealInfo {unsigned amount=1;unsigned GetHeal()const{return amount;}};
struct AuraEffect {unsigned id=853510;int misc=127;unsigned GetId()const{return id;}int GetMiscValue()const{return misc;}};
struct ProcEventInfo {Unit* actor=nullptr;Unit* target=nullptr;Spell* cast=nullptr;HealInfo* heal=nullptr;SpellInfo* info=nullptr;Spell const* GetProcSpell(){return cast;}HealInfo const* GetHealInfo(){return heal;}Unit* GetActor(){return actor;}Unit* GetActionTarget(){return target;}SpellInfo const* GetSpellInfo(){return info;}};
@@METADATA@@
struct Probe {Unit owner;Unit* GetTarget(){return &owner;}Unit* ToPlayer(){return &owner;}bool HasAura(unsigned id){return owner.HasAura(id);}
@@POWER@@
@@BODY@@
bool Healing(AuraEffect* effect,unsigned schoolMask){return (@@HEALING@@)(effect);}
};
int main(){
 SpellInfo ground;ground.Id=965903;ApplyAscensionHeroPaladinContracts(&ground);assert(ground.Effects[0].Effect==128&&*ground.Effects[0].RadiusEntry==14&&*ground.DurationEntry==31&&ground.AttributesEx3==128);
 Probe p;Spell cast;HealInfo heal;SpellInfo spell;Unit other;ProcEventInfo event{&p.owner,&p.owner,&cast,&heal,&spell};
 for(bool str:{false,true})for(bool agi:{false,true}){p.owner.str=str;p.owner.agi=agi;assert(p.CheckProc(event)==(str||agi));}
 p.owner.str=true;p.owner.agi=false;assert(p.CheckProc(event));
 event.target=&other;assert(!p.CheckProc(event));event.target=&p.owner;
 event.actor=&other;assert(!p.CheckProc(event));event.actor=&p.owner;
 cast.time=0;assert(!p.CheckProc(event));cast.time=1000;
 cast.triggered=true;assert(!p.CheckProc(event));cast.triggered=false;
 heal.amount=0;assert(!p.CheckProc(event));heal.amount=1;
 event.cast=nullptr;assert(!p.CheckProc(event));event.cast=&cast;
 event.heal=nullptr;assert(!p.CheckProc(event));event.heal=&heal;
 p.owner.area=false;assert(p.CheckProc(event));p.owner.area=true;
 for(unsigned id:{853510u,853511u,853512u}){SpellInfo info;info.Id=id;ApplyAscensionHeroPaladinContracts(&info);assert(info.Effects[0].ApplyAuraName==238 && info.Effects[0].MiscValue==127);assert(info.Effects[1].ApplyAuraName==52);assert(info.Effects[2].TriggerSpell==(id==853512?853513u:0));}
 Area52MysticRealm::active=false;SpellInfo untouched;untouched.Id=853510;ApplyAscensionHeroPaladinContracts(&untouched);assert(untouched.Effects[0].ApplyAuraName==4);Area52MysticRealm::active=true;
 for(unsigned id:{64127u,64129u}){SpellInfo info;info.Id=id;ApplyAscensionHeroPaladinContracts(&info);assert(info.Effects[0].SpellClassMask.a==1 && info.Effects[0].SpellClassMask.c==0x800000);assert(info.Effects[1].SpellClassMask.b==1);}
 spell.Id=86381;assert(p.CheckProcTriggerSpell(nullptr,event));p.owner.area=false;assert(!p.CheckProcTriggerSpell(nullptr,event));p.owner.area=true;spell.Id=17;spell.SpellFamilyFlags[0]=1;assert(p.CheckProcTriggerSpell(nullptr,event));spell.Id=999;spell.SpellFamilyFlags[0]=0;assert(!p.CheckProcTriggerSpell(nullptr,event));event.info=nullptr;assert(!p.CheckProcTriggerSpell(nullptr,event));
 AuraEffect effect;
 for(unsigned id:{853510u,853511u,853512u})for(bool str:{false,true})for(bool agi:{false,true}){effect.id=id;p.owner.str=str;p.owner.agi=agi;assert(p.Healing(&effect,2)==(str||agi));assert((@@CRIT@@)(&effect));}
 effect.id=123;p.owner.str=false;p.owner.agi=false;assert(p.Healing(&effect,2));assert(!(@@CRIT@@)(&effect));effect.misc=4;assert(!p.Healing(&effect,2));
}
"""
for key, value in (("POWER", power), ("BODY", body), ("METADATA", metadata), ("HEALING", healing), ("CRIT", crit)):
    source = source.replace("@@" + key + "@@", value)
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)
    (path / "test.cpp").write_text(source)
    subprocess.run(["c++", "-std=c++20", "-Wall", "-Wextra", str(path / "test.cpp"), "-o", str(path / "test")], check=True)
    subprocess.run([str(path / "test")], check=True)
print("PASS: production talent metadata, Power of Light primary/heal/cast filters and Body and Soul Leap admission. Native dispatcher, lifecycle and database application remain separate checks.")
