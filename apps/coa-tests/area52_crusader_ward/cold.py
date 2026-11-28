from pathlib import Path
import subprocess,tempfile,struct
r=Path(__file__).resolve().parents[3]
def block(s,m):
 a=s.index(m);b=s.index('{',a);i=b+1;n=1
 while n:
  n+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]
u=(r/'src/server/game/Entities/Unit/Unit.cpp').read_text()
s=(r/'src/server/coa/AscensionMysticKnightEpics.cpp').read_text()
t=(r/'apps/coa-tests/area52_crusader_modifiers/stats.cpp').read_text()
t=t[:t.index('int main()')].replace('@MULTIPLIER@','int Area52ConsecratedGroundAmount(AuraEffect const* effect,Unit const*){assert(effect->GetId()!=965903);return 0;}\n'+block(u,'float Unit::GetTotalAuraMultiplier(AuraType auraType, std::function')).replace('@HANDLER@','')
t=t.replace('bool player=true;','bool player=true;bool area=true;uint32 GetGUID()const{return 7;}float Taken(int,SpellInfo const*,Unit const*)const;')
t=t.replace('bool HasAttribute(int)const{return false;}','uint32 school=2;uint32 GetSchoolMask()const{return school;}bool HasAttribute(int)const{return false;}')
t=t.replace('int amount=1,misc=0,type=137;','int amount=1,misc=0,type=137,miscB=0;uint32 guid=7;int GetMiscValueB()const{return miscB;}uint32 GetCasterGUID()const{return guid;}')
t=t.replace('bool Applies(Unit const*){return true;}','bool Applies(Unit const*p){return p&&p->area;}')
a=u.index('    if (damagetype == DOT && AscensionFreePick::Applies(ToPlayer()))');b=u.index('    if (caster && caster->IsPlayer()',a)
t+='\nconstexpr int DOT=1,SPELL_AURA_214=214;float Unit::Taken(int damagetype,SpellInfo const*spellProto,Unit const*caster)const{float TakenTotalMod=1;'+u[a:b]+'return TakenTotalMod;}\n'
t+=r"""
int main(){Unit victim,caster;AuraEffect ward;ward.id=977902;ward.amount=-10;ward.misc=126;ward.type=214;victim.auras[214]={&ward};SpellInfo incoming;
for(uint32 school:{1u,2u,4u,8u,16u,32u,64u}){incoming.school=school;assert(close(victim.Taken(DOT,&incoming,&caster),school==1?1:.9));assert(close(victim.Taken(0,&incoming,&caster),1));}
victim.area=false;assert(close(victim.Taken(DOT,&incoming,&caster),1));victim.area=true;
AuraEffect other;other.id=30299;other.amount=-6;other.misc=126;victim.auras[214].push_back(&other);assert(close(victim.Taken(DOT,&incoming,&caster),.9*.94));
victim.auras[214].erase(victim.auras[214].begin());assert(close(victim.Taken(DOT,&incoming,&caster),.94));victim.auras[214].clear();assert(close(victim.Taken(DOT,&incoming,&caster),1));}
"""
blob=Path('/data/dbc/Spell.dbc').read_bytes();magic,n,f,size,strings=struct.unpack_from('<4s4I',blob);assert magic==b'WDBC' and f==234 and size==936
row=next(struct.unpack_from('<234I',blob,20+i*size) for i in range(n) if struct.unpack_from('<I',blob,20+i*size)[0]==977902)
assert row[95]==214 and row[110]==126 and row[113]==0 and struct.unpack('<i',struct.pack('<I',row[80]))[0]+row[74]==-10
config=block(s,'void OnAfterConfigLoad(bool)').replace(' override','')
a=s.index('    if (spell == WardOfLight)',s.index('bool HasArea52EpicEquipmentHandlers'));b=s.index('    if (Area52Worldforged',a)
reg=s[s.index('    new WardOfLightCasts();'):s.index('    new WorldforgedAuras();')]
cold=r"""
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
using uint32=uint32_t;bool Enabled=false,WardOfLightRegistered=false;constexpr uint32 WardOfLight=977900,WardOfLightProtection=977902;
struct Config{bool enabled=true;std::string model="hero",realm="live";template<class T>T GetOption(char const*key,T fallback){std::string k=key;if constexpr(std::is_same_v<T,bool>)return enabled;else return k=="CoA.ClassModel"?model:realm;}}config,*sConfigMgr=&config;
struct Manager{bool loaded=false;void*GetSpellInfo(uint32 id){return loaded&&id==977902?this:nullptr;}}manager,*sSpellMgr=&manager;
int casts=0,auras=0;struct WardOfLightCasts{WardOfLightCasts(){++casts;}};struct WardOfLightAuras{WardOfLightAuras(){++auras;}};
"""+config+'\nbool Available(uint32 spell){'+s[a:b]+'return false;}\nvoid Register(){'+reg+'}\n'+r"""
int main(){assert(!Available(977900));Register();assert(casts==1&&auras==1&&!Available(977900));OnAfterConfigLoad(false);assert(!Available(977900));manager.loaded=true;assert(Available(977900));OnAfterConfigLoad(true);assert(Available(977900));manager.loaded=false;assert(!Available(977900));manager.loaded=true;assert(Available(977900));config.model="coa";OnAfterConfigLoad(true);assert(!Available(977900));config.model="hero";config.realm="seasonal";OnAfterConfigLoad(true);assert(!Available(977900));config.realm="live";config.enabled=false;OnAfterConfigLoad(true);assert(!Available(977900));config.enabled=true;OnAfterConfigLoad(true);assert(Available(977900)&&!Available(42));}
"""
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp)
 for name,code in [('aggregate',t),('cold',cold)]:
  src=p/(name+'.cpp');src.write_text(code);subprocess.run(['c++','-std=c++20',str(src),'-o',str(p/name)],check=True);subprocess.run([str(p/name)],check=True)
print('PASS extracted native Ward startup availability and DOT214 aggregation with native metadata, removal and other-realm isolation')
