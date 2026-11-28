from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
s=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text()
def block(s,m):
 a=s.index(m);b=s.index('{',a);e=b+1;n=1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
blocks=[]
for name in ['uint32 Unit::SpellDamageBonusDone(', 'uint32 Unit::MeleeDamageBonusDone(']:
 a=s.index(name);a=s.index('    float DoneTotalMod =',a);b=s.index('    if (AscensionFreePick::Applies(ToPlayer()))',a);blocks.append(block(s[b:],'if (AscensionFreePick::Applies(ToPlayer()))'))
assert s.count('if (aurEff->GetId() == 84537 && AscensionFreePick::Applies(ToPlayer()))')==3
code=r"""
#include <cassert>
#include <cmath>
#include <map>
#include "Area52StrengthFaith.h"
constexpr unsigned EFFECT_0=0,EFFECT_1=1,SPELL_ATTR2_AUTO_REPEAT=32;
struct AuraEffect{int amount;int GetAmount()const{return amount;}};
struct SpellInfo{unsigned school=1;bool autorepeat=false;unsigned GetSchoolMask()const{return school;}bool HasAttribute(unsigned)const{return autorepeat;}};
struct Probe;namespace AscensionFreePick{bool Applies(Probe*);}
struct Probe{bool area=true;std::map<unsigned,AuraEffect>effects;Probe*ToPlayer(){return this;}AuraEffect const*GetAuraEffect(unsigned id,unsigned effect){assert(id==84537);auto i=effects.find(effect);return i==effects.end()?nullptr:&i->second;}
float Spell(SpellInfo const*spellProto){float DoneTotalMod=1.2f;@@SPELL@@return DoneTotalMod;}
float Weapon(SpellInfo const*spellProto,unsigned damageSchoolMask){float DoneTotalMod=1.2f;@@WEAPON@@return DoneTotalMod;}
};namespace AscensionFreePick{bool Applies(Probe*p){return p->area;}}
bool close(float a,float b){return std::abs(a-b)<0.000001f;}
int main(){Probe p;SpellInfo spell;p.effects={{0,{1}},{1,{1}}};for(unsigned school:{1u,2u,3u,4u,6u,127u}){spell.school=school;float factor=(school&1?1.01f:1)*(school&2?1.01f:1);assert(close(p.Spell(&spell),1.2f*factor));assert(close(p.Weapon(&spell,school),1.2f*factor));}
spell.school=3;spell.autorepeat=true;assert(close(p.Spell(&spell),1.2f));assert(close(p.Weapon(&spell,3),1.2f));assert(close(p.Weapon(nullptr,1),1.2f));spell.autorepeat=false;
for(int cycle=0;cycle<3;++cycle){p.effects.clear();assert(close(p.Spell(&spell),1.2f));p.effects[0]={1};assert(close(p.Spell(&spell),1.212f));p.effects[1]={1};assert(close(p.Spell(&spell),1.22412f));assert(close(p.Weapon(&spell,3),1.22412f));}
p.area=false;assert(close(p.Spell(&spell),1.2f));assert(close(p.Weapon(&spell,3),1.2f));}
""".replace('@@SPELL@@',blocks[0]).replace('@@WEAPON@@',blocks[1])
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-I',str(ROOT/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS: actual spell/weapon Strength in Faith callsite arithmetic, each school/hybrid, ordinary auto exclusions, live aura removal/relearning, repeated evaluation, other-realm gating and unrelated multiplier preservation. Saved login restoration remains separate native fixture.')
