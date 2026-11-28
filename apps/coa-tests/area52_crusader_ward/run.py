from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
production = (root / "src/server/coa/Area52WardOfLight.inc").read_text().replace(" override", "")
unit = (root / "src/server/game/Entities/Unit/Unit.cpp").read_text()
a = unit.index("                switch (effect->GetId())", unit.index("if (damagetype == DOT && AscensionFreePick::Applies(ToPlayer()))"))
b = unit.index("            });", a)
filter_body = unit[a:b]
stub = r"""
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <vector>
#include <initializer_list>
using uint32=uint32_t; using int32=int32_t; using int64=int64_t; using uint8=uint8_t;
constexpr int EFFECT_0=0,EFFECT_1=1,EFFECT_2=2,SPELL_AURA_DUMMY=4,SPELL_AURA_NONE=0;
constexpr int ALLSPELLHOOK_ON_CALCULATED_TARGET=1,ALLSPELLHOOK_ON_HIT_RESULT=2;
constexpr int UNITHOOK_ON_PERIODIC_DAMAGE_RESULT=3,UNITHOOK_ON_AURA_REMOVE=4;
constexpr int PLAYERSPELL_REMOVED=3,SPELL_MISS_NONE=0;using AuraRemoveMode=int;
struct Player;
struct Unit {Player* player=nullptr; bool IsPlayer()const{return player;} Player*ToPlayer(){return player;}};
struct Entry{int State=0;};
struct Player:Unit {
 bool area=true,owned=true;std::map<uint32,Entry*> spells;std::set<uint32>storms{53385,53386,999999};
 std::map<uint32,uint32>cooldowns;std::vector<uint32>removed;uint32 cast=0,cleanup=0;
 Player(){player=this;}auto const&GetSpellMap(){return spells;}
 bool MatchesSpellInteraction(uint32 id,uint32 root){return root==53385&&storms.contains(id);}
 uint32 GetSpellCooldownDelay(uint32 id){return cooldowns[id];}
 void RemoveSpellCooldown(uint32 id,bool){removed.push_back(id);cooldowns[id]=0;}
 void CastSpell(Unit*,uint32 id,bool){cast=id;}uint32 GetGUID(){return 7;}
 void RemoveAurasDueToSpell(uint32 id,uint32 guid){assert(guid==7);cleanup=id;}
};
bool Applies(Player*p){return p&&p->area;}
namespace AscensionMysticEnchant{bool GrantsActive(Player*p,uint32 id){return p&&p->owned&&id==977900;}}
struct EffectData{int Effect=6,ApplyAuraName=108,TriggerSpell=977901;};
struct SpellInfo {uint32 Id=53385,ProcFlags=664096,ProcCharges=1,StackAmount=3,school=2;EffectData Effects[3];uint32 GetSchoolMask()const{return school;}};
struct Spell{Unit*caster;SpellInfo info;Unit*GetCaster(){return caster;}SpellInfo const*GetSpellInfo(){return &info;}};
struct TargetInfo{int32 damage=1000,damageBeforeTakenMods=1000;};
struct Aura{uint32 id=977900;uint32 GetId(){return id;}};
struct AuraApplication{Aura aura;Aura*GetBase(){return &aura;}};
struct AllSpellScript{AllSpellScript(char const*,std::initializer_list<int>){}};
struct UnitScript{UnitScript(char const*,bool,std::initializer_list<int>){}};
bool roll=true;int calls=0;bool roll_chance_i(int chance){assert(chance==20);++calls;return roll;}
struct AuraEffect {uint32 id=977902,mask=126;uint32 GetId()const{return id;}uint32 GetMiscValue()const{return mask;}};
"""
tests = r"""
bool PeriodicFilter(AuraEffect const* effect,SpellInfo const* spellProto){
@@FILTER@@
}
int main(){
 SpellInfo metadata;metadata.Id=977900;ConfigureWardOfLight(&metadata);
 assert(!metadata.ProcFlags&&!metadata.ProcCharges&&metadata.StackAmount==1);
 assert(metadata.Effects[0].ApplyAuraName==4&&!metadata.Effects[0].TriggerSpell);
 assert(!metadata.Effects[1].Effect&&!metadata.Effects[2].Effect);
 metadata.Id=42;metadata.ProcFlags=7;ConfigureWardOfLight(&metadata);assert(metadata.ProcFlags==7);
 Player caster,victim;Unit creature;Spell spell{&caster};WardOfLightCasts casts;WardOfLightAuras auras;
 for(uint32 id:{53385u,53386u,999999u,42u})for(bool pvp:{false,true})for(bool owns:{false,true}){
  spell.info.Id=id;caster.owned=owns;TargetInfo hit;
  casts.OnSpellCalculatedTarget(&spell,pvp?static_cast<Unit*>(&victim):&creature,hit);
  int expected=owns&&id!=42?(pvp?1050:1200):1000;assert(hit.damage==expected&&hit.damageBeforeTakenMods==expected);
 }
 caster.owned=true;spell.info.Id=53385;TargetInfo hit;caster.area=false;casts.OnSpellCalculatedTarget(&spell,&creature,hit);assert(hit.damage==1000);caster.area=true;
 casts.OnSpellCalculatedTarget(nullptr,&creature,hit);assert(hit.damage==1000);hit.damage=0;casts.OnSpellCalculatedTarget(&spell,&creature,hit);assert(!hit.damage);
 casts.OnSpellHitResult(&spell,&creature,0,100,0,false);assert(caster.cast==977902);
 caster.cast=0;casts.OnSpellHitResult(&spell,&creature,1,100,0,false);assert(!caster.cast);
 casts.OnSpellHitResult(&spell,&creature,0,0,100,false);assert(!caster.cast);
 Entry active,forgotten;forgotten.State=PLAYERSPELL_REMOVED;
 victim.spells={{53385,&active},{53386,&active},{999999,&forgotten},{42,&active}};
 victim.cooldowns={{53385,5000},{53386,60000},{999999,7000},{42,8000}};
 spell.info.Id=42;casts.OnSpellHitResult(&spell,&victim,0,1,0,false);
 assert(!victim.cooldowns[53385]&&!victim.cooldowns[53386]&&victim.cooldowns[999999]==7000&&victim.cooldowns[42]==8000);
 victim.cooldowns[53385]=5000;roll=false;auras.OnPeriodicDamageResult(&victim,&caster,1,&spell.info);assert(victim.cooldowns[53385]==5000);
 roll=true;auras.OnPeriodicDamageResult(&victim,&caster,1,&spell.info);assert(!victim.cooldowns[53385]);
 int old=calls;WardOfLightReset(&victim,&victim,1,&spell.info);WardOfLightReset(&victim,&caster,0,&spell.info);WardOfLightReset(&victim,&caster,1,nullptr);WardOfLightReset(&creature,&caster,1,&spell.info);assert(calls==old);
 victim.owned=false;WardOfLightReset(&victim,&caster,1,&spell.info);assert(calls==old);victim.owned=true;victim.area=false;WardOfLightReset(&victim,&caster,1,&spell.info);assert(calls==old);victim.area=true;
 AuraApplication app;auras.OnAuraRemove(&victim,&app,0);assert(victim.cleanup==977902);victim.cleanup=0;app.aura.id=42;auras.OnAuraRemove(&victim,&app,0);assert(!victim.cleanup);auras.OnAuraRemove(nullptr,nullptr,0);
 AuraEffect protection;SpellInfo incoming;
 for(uint32 school:{1u,2u,4u,8u,16u,32u,64u}){incoming.school=school;assert(PeriodicFilter(&protection,&incoming)==(school!=1));}
 protection.id=42;assert(!PeriodicFilter(&protection,&incoming));
}
""".replace("@@FILTER@@", filter_body)
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)
    (path / "ward.cpp").write_text(stub + production + tests)
    subprocess.run(["c++", "-std=c++20", "-Wall", "-Wextra", str(path / "ward.cpp"), "-o", str(path / "ward")], check=True)
    subprocess.run([str(path / "ward")], check=True)
print("PASS: production Ward metadata, damage hooks, direct/periodic reset admission, parent cleanup and native school filter. Adapter does not simulate complete spell execution or login.")

subprocess.run([__import__("sys").executable, str(Path(__file__).with_name("cold.py"))], check=True)
