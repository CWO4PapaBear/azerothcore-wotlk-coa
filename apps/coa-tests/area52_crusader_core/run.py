from pathlib import Path
import re,subprocess,tempfile
root=Path(__file__).resolve().parents[3]
s=(root/'src/server/game/Spells/SpellMgr.cpp').read_text()
def block(start):
 a=s.index(start);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
f=block('int32 GetDiminishingReturnsLimitDuration(')
constants=sorted(set(re.findall(r'\b(?:SPELLFAMILY_|DIMINISHING_)[A-Z_0-9]+',f)))
meta=block('        if (spellInfo->Id == 20424 &&')
code=r"""
#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
using uint32=uint32_t;using int32=int32_t;using DiminishingGroup=int;
constexpr int IN_MILLISECONDS=1000,EFFECT_0=0;
@@ENUM@@
bool limited=true;
bool IsDiminishingReturnsGroupDurationLimited(int){return limited;}
namespace AscensionFreePick {bool enabled=false;bool Enabled(){return enabled;}}
struct Effect{int BasePoints=34,DieSides=1;};
struct SpellInfo{uint32 Id=0,SpellFamilyName=0;std::array<uint32,3> SpellFamilyFlags{};Effect Effects[3];};
@@FUNCTION@@
struct Config{bool enabled=true;std::string model="hero",realm="live";uint32 mode=0x400;
template<class T>T GetOption(char const* k,T){if constexpr(std::is_same_v<T,bool>)return enabled;else if constexpr(std::is_same_v<T,uint32>)return mode;else return std::string(k)=="CoA.ClassModel"?model:realm;}} config;
auto sConfigMgr=&config;
void metadata(SpellInfo* spellInfo){@@METADATA@@}
int main(){
 for(uint32 id:{710u,18647u}){
  SpellInfo s;s.Id=id;s.SpellFamilyName=SPELLFAMILY_WARLOCK;s.SpellFamilyFlags[1]=0x8000000;
  AscensionFreePick::enabled=false;assert(GetDiminishingReturnsLimitDuration(0,&s)==6000);
  AscensionFreePick::enabled=true;assert(GetDiminishingReturnsLimitDuration(0,&s)==8000);
  limited=false;assert(GetDiminishingReturnsLimitDuration(0,&s)==0);limited=true;
 }
 for(int mode=0;mode<6;++mode){
  config={};SpellInfo s;s.Id=20424;
  if(mode==1)config.enabled=false;if(mode==2)config.model="coa";if(mode==3)config.realm="seasonal";
  if(mode==4)config.mode=0x800;if(mode==5)s.Id=1120424;
  AscensionFreePick::enabled=false;metadata(&s);metadata(&s);
  assert(s.Effects[0].BasePoints+1==(mode==0?18:35));assert(s.Effects[0].DieSides==1);
 }
}
"""
code=code.replace('@@ENUM@@','enum {'+','.join(constants)+'};').replace('@@FUNCTION@@',f).replace('@@METADATA@@',meta)
with tempfile.TemporaryDirectory() as directory:
 p=Path(directory);(p/'test.cpp').write_text(code)
 subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS native Banish duration function both ranks, duration-limited exclusion, other-realm isolation; actual Seal metadata cold-unready repeated load and realm/config exclusions. No combat simulation.')
