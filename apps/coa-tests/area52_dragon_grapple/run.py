from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[3];s=(r/'src/server/coa/AscensionSummonedEffects.cpp').read_text();a=s.index('case Behaviour::PullOwner:');b=s.index('case Behaviour::Heal:',a);pull=s[a:b]
w=(r/'src/server/coa/AscensionHeroWarrior.cpp').read_text();a=w.index('    SpellCastResult Check()',w.index('class spell_area52_grappling_followup'));b=w.index('    void Register()',a);methods=w[a:b]
code=r"""
#include "AscensionSummonedEffects.h"
#include <cassert>
using uint32=std::uint32_t;using SpellCastResult=int;using namespace AscensionSummonedEffects;
constexpr int TRIGGERED_FULL_MASK=1,SPELL_CAST_OK=0,SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW=1;
struct Unit{bool aura=true,known=true;float x=0,y=0,z=0;Unit*ToPlayer(){return this;}bool HasAura(uint32 id){assert(id==760096);return aura;}bool HasSpell(uint32 id){assert(id==760056);return known;}void RemoveAurasDueToSpell(uint32 id){assert(id==760096);aura=false;}};
struct SpellCastTargets{Unit*unit=nullptr;bool dest=false;float x=0,y=0,z=0;void SetUnitTarget(Unit*u){unit=u;}void SetDst(Unit const&u){dest=true;x=u.x;y=u.y;z=u.z;}};
struct SpellInfo{};
struct Manager{SpellInfo info;SpellInfo const*GetSpellInfo(uint32 id){assert(id==760057);return &info;}}manager,*sSpellMgr=&manager;
struct Creature:Unit{SpellCastTargets captured;unsigned calls=0;void CastSpell(SpellCastTargets const&t,SpellInfo const*,void*,int flags){assert(flags==TRIGGERED_FULL_MASK);captured=t;++calls;}};
struct AI{Creature*me;void summon(Summon const*row,Unit*owner){switch(row->Kind){@@PULL@@default:assert(false);}}};
struct Script{Unit caster;Unit*GetCaster(){return &caster;}@@METHODS@@};
int main(){
 for(unsigned id:{760056u,760094u}){auto*row=Find(id,840057,false);assert(row&&row->Kind==Behaviour::PullOwner&&row->Payload==760057);Creature c;c.x=12;c.y=-13;c.z=7;Unit owner;AI ai{&c};ai.summon(row,&owner);assert(c.calls==1&&c.captured.unit==&owner&&c.captured.dest&&c.captured.x==12&&c.captured.y==-13&&c.captured.z==7);}
 Script s;assert(s.Check()==0);s.Consume();assert(s.Check()!=0);s.caster.aura=true;s.caster.known=false;assert(s.Check()!=0);
}
""".replace('@@PULL@@',pull).replace('@@METHODS@@',methods).replace('#include <cassert>','#include <cassert>\n#include <initializer_list>')
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-I'+str(r/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS actual summon rows, native pull destination/unit target, recast authorization and single consumption. Movement/pathfinding and client rendering not emulated.')
