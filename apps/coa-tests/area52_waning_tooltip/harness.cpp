#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <vector>
#include <type_traits>
#include <iostream>
using uint32=uint32_t;using uint16=uint16_t;using uint8=uint8_t;using int32=int32_t;
constexpr int MAX_SPELLMOD=32,MAX_CLIENT_SPELLMOD=31,SPELLMOD_CASTING_TIME=10,SPELLFAMILY_DRUID=7;
constexpr int SPELLMOD_FLAT=107,SPELLMOD_PCT=108,SMSG_SET_FLAT_SPELL_MODIFIER=1,SMSG_SET_PCT_SPELL_MODIFIER=2;
#define LOG_DEBUG(...)
#define LOG_ERROR(...)
struct flag96 {uint32 x[3];flag96(uint32 a=0,uint32 b=0,uint32 c=0):x{a,b,c}{}uint32& operator[](int i){return x[i];}flag96 operator&(flag96 const& b)const{return {x[0]&b.x[0],x[1]&b.x[1],x[2]&b.x[2]};}operator bool()const{return x[0]||x[1]||x[2];}};
struct SpellInfo{uint32 SpellFamilyName;};
struct SpellModifier{int op=10,type=108;uint32 spellId;flag96 mask;int32 value;void* ownerAura=this;};
struct Manager{std::map<uint32,SpellInfo> rows;SpellInfo const* GetSpellInfo(uint32 id)const{auto i=rows.find(id);return i==rows.end()?nullptr:&i->second;}} manager;
auto* sSpellMgr=&manager;
struct WorldPacket{uint16 opcode;std::vector<uint8> bytes;WorldPacket(uint16 o,int):opcode(o){}template<class T>WorldPacket& operator<<(T x){using U=std::make_unsigned_t<T>;for(unsigned i=0;i<sizeof(T);++i)bytes.push_back(U(x)>>(i*8));return *this;}};
struct Player;
namespace AscensionFreePick{bool Applies(Player const*);}
using SpellModContainer=std::set<SpellModifier*>;
struct Player{
 bool area=true,compat=true;SpellModContainer m_spellMods[MAX_SPELLMOD];mutable std::vector<WorldPacket> packets;
 bool UsesAscensionSpellModifierLayout()const{return compat;}uint32 GetClientSpellModCount()const{return compat?MAX_SPELLMOD:MAX_CLIENT_SPELLMOD;}
 void SendDirectMessage(WorldPacket const* p)const{packets.push_back(*p);}
 void SendSpellModifier(uint16,uint8,uint8,int32,uint32)const;
 void AddSpellMod(SpellModifier*,bool);
};
bool AscensionFreePick::Applies(Player const* p){return p&&p->area;}
SEND
ADD
int32 read32(std::vector<uint8> const& b,size_t i){return int32(uint32(b[i])|uint32(b[i+1])<<8|uint32(b[i+2])<<16|uint32(b[i+3])<<24);}
int32 latest(Player const& p,int family,int effect,int type=2){for(auto i=p.packets.rbegin();i!=p.packets.rend();++i)if(i->opcode==type&&i->bytes.size()==11&&read32(i->bytes,7)==family&&i->bytes[1]==effect){assert(i->bytes[0]==0&&i->bytes[2]==10);return read32(i->bytes,3);}assert(false);return 999;}
int main(){
 manager.rows={{760198,{3}},{100,{7}},{101,{3}},{102,{7}}};
 Player p;SpellModifier waning{10,108,760198,{1u<<29,0,0},-5},druid{10,108,100,{4,0,0},-10},mage{10,108,101,{4,0,0},-50},flat{10,107,102,{4,0,0},-500};
 p.AddSpellMod(&druid,true);p.AddSpellMod(&mage,true);p.AddSpellMod(&flat,true);
 for(int stack=1;stack<=5;++stack){
  if(stack>1)p.AddSpellMod(&waning,false);
  waning.value=-5*stack;p.AddSpellMod(&waning,true);
  assert(latest(p,3,29)==-5*stack&&latest(p,7,2)==-10-5*stack);
  assert(latest(p,3,2)==-50&&latest(p,7,2,1)==-500);
 }
 p.AddSpellMod(&druid,false);assert(latest(p,7,2)==-25);
 druid.value=-20;p.AddSpellMod(&druid,true);assert(latest(p,7,2)==-45);
 p.AddSpellMod(&waning,false);assert(latest(p,7,2)==-20&&latest(p,3,29)==0);
 p.AddSpellMod(&waning,true);p.AddSpellMod(&waning,false);assert(latest(p,7,2)==-20);
 Player other;other.area=false;other.AddSpellMod(&waning,true);assert(other.packets.size()==1&&latest(other,3,29)==-25);
 Player legacy;legacy.compat=false;legacy.AddSpellMod(&waning,true);assert(legacy.packets.size()==1&&legacy.packets[0].bytes.size()==6);
 Player clean;waning.value=-5;for(int stack=1;stack<=5;++stack){if(stack>1)clean.AddSpellMod(&waning,false);waning.value=-5*stack;clean.AddSpellMod(&waning,true);assert(latest(clean,7,2)==-5*stack);}
 clean.AddSpellMod(&waning,false);assert(latest(clean,7,2)==0);
 std::cout<<"PASS actual AddSpellMod and SendSpellModifier: 1-5 stacks project -5/-10/-15/-20/-25 to Druid Starfire bit and Mage Arcane Blast bit; removal resets; concurrent modifiers preserved in both update orders; unrelated family/opcode and non-A52/legacy isolation\n";
}
