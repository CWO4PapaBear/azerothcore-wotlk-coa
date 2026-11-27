from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[3];s=(r/'src/server/game/Spells/SpellEffects.cpp').read_text();a=s.index('void Spell::EffectSelfResurrect(');b=s.index('void Spell::EffectSkinning(',a);method=s[a:b].replace('Spell::EffectSelfResurrect','EffectSelfResurrect')
code=r"""
#include <cassert>
#include <cstdint>
#include <array>
using uint32=uint32_t;using SpellEffIndex=int;
constexpr int SPELL_EFFECT_HANDLE_HIT=1,POWER_MANA=0,POWER_RAGE=1,POWER_ENERGY=2;
uint32 CalculatePct(uint32 x,int p){return uint64_t(x)*p/100;}
struct Player{bool alive=false,world=true,player=true;uint32 health=0,resurrections=0,bones=0;std::array<uint32,3>power{0,50,10},maximum{2000,100,100};bool IsAlive(){return alive;}bool IsPlayer(){return player;}bool IsInWorld(){return world;}uint32 CountPctFromMaxHealth(int p){return CalculatePct(10000,p);}uint32 GetMaxPower(int i){return maximum[i];}Player*ToPlayer(){return this;}void ResurrectPlayer(float){++resurrections;alive=true;}void SetHealth(uint32 n){health=n;}void SetPower(int i,uint32 n){power[i]=n;}void SpawnCorpseBones(){++bones;}};
struct Effect{uint32 MiscValue=0;};struct Info{Effect Effects[3];};
struct Spell{int effectHandleMode=1,damage=20;Player*m_caster=nullptr;Info info,*m_spellInfo=&info;
@@METHOD@@
};
int main(){
 for(int rank=0;rank<3;++rank)for(int copies=0;copies<=3;++copies){Player p;Spell s;s.m_caster=&p;s.damage=20+rank*20+copies*5;s.EffectSelfResurrect(0);assert(p.resurrections==1&&p.bones==1);assert(p.health==uint32(s.damage*100));assert(p.power[0]==uint32(s.damage*20)&&p.power[1]==0&&p.power[2]==100);s.EffectSelfResurrect(0);assert(p.resurrections==1);}
 for(int mode=0;mode<4;++mode){Player p;Spell s;s.m_caster=&p;if(mode==0)s.effectHandleMode=0;if(mode==1)p.alive=true;if(mode==2)p.world=false;if(mode==3)p.player=false;s.EffectSelfResurrect(0);assert(!p.resurrections&&!p.bones);}
 Player p;Spell s;s.m_caster=&p;p.maximum[0]=0;s.EffectSelfResurrect(0);assert(p.power[0]==0);
 s.m_caster=nullptr;s.EffectSelfResurrect(0);
}
""".replace('@@METHOD@@',method)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS native self-resurrection restoration for base/talent/enchant percentage combinations and rejection guards. Percentages injected at effect boundary; modifier and cooldown paths require separate checks.')
