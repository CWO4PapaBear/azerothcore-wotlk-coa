from pathlib import Path
import tempfile, subprocess
r = Path(__file__).resolve().parents[3]
def block(s, token):
    a = s.index(token)
    b = s.index('{', a) + 1
    depth = 1
    while depth:
        depth += (s[b] == '{') - (s[b] == '}')
        b += 1
    return s[a:b]
h = (r / 'src/server/game/Spells/SpellMgr.h').read_text()
enums = '\n'.join(block(h, 'enum ' + n) + ';' for n in
    ['ProcFlags', 'ProcFlagsSpellType', 'ProcFlagsSpellPhase', 'ProcFlagsHit', 'ProcAttributes'])
s = (r / 'src/server/game/Spells/SpellMgr.cpp').read_text()
proc = block(s, '    if ((spellId == 29723')
a = (r / 'src/server/game/Spells/Auras/SpellAuras.cpp').read_text()
guard = block(a, '    if ((GetId() == 46913')
p = (r / 'src/server/game/Entities/Player/Player.cpp').read_text()
x = p.index('                if ((mod->spellId == 20502')
y = p.index('                    percent *= 2;', x) + len('                    percent *= 2;')
modifier = p[x:y]
c=(r/'src/server/game/Spells/Spell.cpp').read_text()
x=c.index('            bool const area52ExecuteCapstone =')
y=c.index('            std::optional<uint32>',x)
capstone=c[x:y]
assert c.index('if (area52ExecuteCapstone && scriptDamageResult)',y)>c.index('caster->DealSpellDamage(',y)

code = r"""
#include <array>
#include <cassert>
#include <chrono>
#include <map>
#include <set>
using uint32=unsigned;
using Milliseconds=std::chrono::milliseconds;
@@ENUMS@@
enum { EFFECT_1=1, EFFECT_2=2, DOT=2, SPELLMOD_DAMAGE=0 };
struct Player;
namespace AscensionFreePick { bool enabled=true; bool Enabled(){return enabled;} bool Applies(Player*){return enabled;} }
struct SpellProcEntry { unsigned ProcFlags{},SpellTypeMask{},SpellPhaseMask{},HitMask{},AttributesMask{},DisableEffectsMask{}; float Chance{}; Milliseconds Cooldown{}; };
bool GetSpellInfo(unsigned){return true;}
SpellProcEntry const* lookup(unsigned spellId){@@PROC@@ return nullptr;}
struct Player { std::map<unsigned,unsigned> origins; bool HasActiveSpell(unsigned n){return origins.count(n);} bool MatchesSpellInteraction(unsigned n,unsigned root){return n==root || origins[n]==root;} Player* ToPlayer(){return this;} };
struct SpellInfo { unsigned Id; };
struct DamageInfo { unsigned amount=10,type=0; unsigned GetDamage() const{return amount;} unsigned GetDamageType() const{return type;} };
struct Event { Player* actor; DamageInfo* damage; SpellInfo* spell; Player* GetActor(){return actor;} DamageInfo* GetDamageInfo(){return damage;} SpellInfo* GetSpellInfo(){return spell;} };
struct App { Player* p; Player* GetTarget(){return p;} };
struct Aura { unsigned id; unsigned GetId(){return id;} unsigned test(App* aurApp, Event& eventInfo){@@GUARD@@ return 1;} };
struct Victim { unsigned hp=19; bool HealthBelowPct(unsigned n){return hp<n;} };
struct Caster:Player { bool talent=true; bool HasAura(unsigned n){return talent&&n==820503;} };
bool capstoneCheck(Caster* caster,Victim* unitTarget,SpellInfo* m_spellInfo){@@CAPSTONE@@ return area52ExecuteCapstone;}
struct Mod { unsigned spellId,op; int value; };
struct ModifierPlayer:Player { int percent(Mod* mod,Victim* victim,unsigned spellId){int percent=mod->value;@@MODIFIER@@ return percent;} };
int main(){
 unsigned ids[]={29723,29725,29724,46913,46914,46915};
 unsigned chances[]={4,8,12,33,66,100};
 for(unsigned i=0;i<6;++i){auto e=lookup(ids[i]);assert(e&&e->Chance==chances[i]);assert(e->Cooldown.count()==6000);assert(e->SpellPhaseMask==PROC_SPELL_PHASE_HIT);assert(e->DisableEffectsMask==6);assert(!(e->ProcFlags&PROC_FLAG_DONE_PERIODIC));assert(!(e->HitMask&PROC_HIT_MISS));}
 AscensionFreePick::enabled=false;for(auto id:ids)assert(!lookup(id));AscensionFreePick::enabled=true;
 Player p,foreign;App app{&p};DamageInfo damage;SpellInfo spell{12294};Event event{&p,&damage,&spell};
 for(unsigned id:{46913u,46914u,46915u}){Aura aura{id};for(unsigned root:{12294u,5308u,20647u,1464u,1680u,954836u,20252u,20253u}){spell.Id=root;assert(aura.test(&app,event));p.origins[999]=root;spell.Id=999;assert(aura.test(&app,event));}spell.Id=23881;assert(!aura.test(&app,event));spell.Id=954288;assert(!aura.test(&app,event));p.origins[954256]=20252;assert(aura.test(&app,event));p.origins.erase(954256);event.actor=&foreign;assert(!aura.test(&app,event));event.actor=&p;spell.Id=12294;damage.amount=0;assert(!aura.test(&app,event));damage.amount=10;damage.type=DOT;assert(!aura.test(&app,event));damage.type=0;}
 Caster caster;Victim victim;SpellInfo hit{20647};assert(capstoneCheck(&caster,&victim,&hit));victim.hp=20;assert(!capstoneCheck(&caster,&victim,&hit));victim.hp=19;caster.talent=false;assert(!capstoneCheck(&caster,&victim,&hit));caster.talent=true;hit.Id=23881;assert(!capstoneCheck(&caster,&victim,&hit));
 ModifierPlayer m;Victim v;unsigned talent[]={20502,20503,820503};for(unsigned i=0;i<3;++i){Mod mod{talent[i],SPELLMOD_DAMAGE,int(4*(i+1))};v.hp=19;assert(m.percent(&mod,&v,20647)==int(8*(i+1)));v.hp=20;assert(m.percent(&mod,&v,20647)==int(4*(i+1)));v.hp=19;assert(m.percent(&mod,&v,23881)==mod.value);AscensionFreePick::enabled=false;assert(m.percent(&mod,&v,20647)==mod.value);AscensionFreePick::enabled=true;}
}
"""
for key,value in [('ENUMS',enums),('PROC',proc),('GUARD',guard),('MODIFIER',modifier),('CAPSTONE',capstone)]:
    code=code.replace('@@'+key+'@@',value)
with tempfile.TemporaryDirectory() as d:
    path=Path(d);(path/'test.cpp').write_text(code)
    subprocess.run(['c++','-std=c++20',str(path/'test.cpp'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True)
print('PASS extracted native rank proc configuration, Area52 isolation, hit-helper admission, owner/damage exclusions and Execute modifier threshold. Cooldown execution and full dispatcher covered separately.')
