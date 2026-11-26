#include <cassert>
#include <cmath>
#include <vector>
using uint32=unsigned;using SpellEffIndex=int;
constexpr int SPELL_EFFECT_HANDLE_LAUNCH_TARGET=1,SPELLFAMILY_HUNTER=9,SMSG_MOVE_KNOCK_BACK=2;
struct WorldPacket {std::vector<float>values;WorldPacket(int,int){}template<class T>WorldPacket&operator<<(T x){values.push_back(float(x));return *this;}};
struct Motion {void MoveJumpTo(float,float,float){assert(false);}};
namespace GameTime{struct Time{unsigned count(){return 100;}};Time GetGameTime(){return {};}}
struct Unit{void SetFallInformation(unsigned t,float z){assert(t==100&&z==5);}float GetPositionZ(){return 5;}float angle=0;WorldPacket packet{0,0};bool IsCreature(){return false;}bool IsPlayer(){return true;}Unit*ToPlayer(){return this;}Motion*GetMotionMaster(){static Motion m;return &m;}float GetOrientation(){return angle;}unsigned GetPackGUID(){return 7;}void SendDirectMessage(WorldPacket*p){packet=*p;}void JumpTo(float,float,bool);};
struct ScriptMgr{unsigned ack=0;void AnticheatSetUnderACKmount(Unit*){++ack;}}scripts;ScriptMgr*sScriptMgr=&scripts;
struct SpellInfo{struct Effect{int MiscValue=@MISC@;}Effects[3];int SpellFamilyName=@FAMILY@;};
struct Spell{int effectHandleMode=1,damage=@DAMAGE@;SpellInfo*m_spellInfo;Unit*unitTarget,*m_caster;void EffectLeapBack(SpellEffIndex);};
@JUMP@
@EFFECT@
int main(){Unit u;SpellInfo info;Spell cast{1,@DAMAGE@,&info,&u,&u};for(float a:{0.f,1.5707963f,3.1415926f,4.712389f}){u.angle=a;cast.EffectLeapBack(0);auto const&v=u.packet.values;assert(v.size()==6);assert(std::fabs(v[2]+std::cos(a))<0.00001);assert(std::fabs(v[3]+std::sin(a))<0.00001);assert(v[4]==20.f&&v[5]==-7.5f);}assert(scripts.ack==4);cast.effectHandleMode=0;cast.EffectLeapBack(0);assert(scripts.ack==4);}
