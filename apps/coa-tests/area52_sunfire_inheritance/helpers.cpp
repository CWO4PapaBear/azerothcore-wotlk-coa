#include <algorithm>
#include <cassert>
#include <cstdint>
#include <list>
#include <set>
#include <string>
#include <type_traits>
#include <iostream>
using uint32=uint32_t;using uint8=uint8_t;
constexpr int EFFECT_0=0,TARGET_UNIT_CASTER=1,SPELL_ATTR0_CU_NEGATIVE_EFF0=0x1000;
constexpr int MAX_SPELLMOD=1,SPELLFAMILY_MAGE=3,AURA_REMOVE_BY_EXPIRE=1;
struct SpellImplicitTargetInfo {int target;SpellImplicitTargetInfo(int t=0):target(t){}};
struct Config {
 bool enable=true;std::string model="hero",realm="live";uint32 mode=0;
 template<class T>T GetOption(std::string key,T fallback)const{
  if constexpr(std::is_same_v<T,bool>)return enable;
  else if constexpr(std::is_same_v<T,std::string>)return key=="CoA.ClassModel"?model:realm;
  else return mode;
 }
} config;
auto* sConfigMgr=&config;
struct SpellInfo {
 uint32 Id=0,AttributesCu=0,ProcFlags=0,ProcCharges=0,SpellFamilyName=7,SpellIconID=0;
 struct Effect{SpellImplicitTargetInfo TargetA;}Effects[1];
};
struct Aura {
 SpellInfo info;int charges=1,drops=0;bool removed=false;
 bool IsUsingCharges()const{return charges>0;}bool DropCharge(int){++drops;if(!--charges)removed=true;return false;}
 SpellInfo const* GetSpellInfo()const{return &info;}
};
struct AuraEffect{int GetAmount()const{return 0;}};
bool roll_chance_i(int){return false;}
struct SpellModifier{uint32 spellId;Aura* ownerAura;};
struct Spell {using UsedSpellMods=std::set<Aura*>;UsedSpellMods m_appliedMods;SpellInfo* m_spellInfo;};
struct Manager{bool proc=false;bool GetSpellProcEntry(uint32)const{return proc;}} manager;
auto* sSpellMgr=&manager;
using SpellModContainer=std::list<SpellModifier*>;
struct Player{SpellModContainer m_spellMods[1];AuraEffect* GetAuraEffectDummy(int){return nullptr;}void RemoveSpellMods(Spell*);};
REMOVE_MODS
void correct(SpellInfo* info){ METADATA }
int main(){
 for(int mode=0;mode<6;++mode){
  config=Config{};
  if(mode==1)config.enable=false;if(mode==2)config.model="coa";if(mode==3)config.realm="seasonal";if(mode==4)config.mode=1;if(mode==5)config.mode=0x400;
  bool eligible=mode==0||mode==5;
  SpellInfo waxing;waxing.Id=983846;waxing.Effects[0].TargetA.target=6;waxing.AttributesCu=SPELL_ATTR0_CU_NEGATIVE_EFF0;
  SpellInfo waning;waning.Id=760198;correct(&waxing);correct(&waning);correct(&waxing);correct(&waning);
  assert(waxing.Effects[0].TargetA.target==(eligible?1:6));assert(waxing.ProcFlags==0);
  assert(bool(waxing.AttributesCu&SPELL_ATTR0_CU_NEGATIVE_EFF0)==!eligible);
  assert(waning.ProcCharges==(eligible?1:0)&&waning.ProcFlags==0);
 }
 for(int stacks=1;stacks<=5;++stacks){
  Player p;Aura buff;buff.info.Id=760198;SpellInfo starfire;starfire.Id=2912;
  SpellModifier a{760198,&buff},b{760198,&buff};p.m_spellMods[0]={&a,&b};
  Spell cast{{},&starfire};p.RemoveSpellMods(&cast);assert(buff.drops==0);
  cast.m_appliedMods.insert(&buff);p.RemoveSpellMods(&cast);assert(buff.removed&&buff.drops==1&&cast.m_appliedMods.empty());
  p.RemoveSpellMods(&cast);assert(buff.drops==1);
  assert(3000*(100-5*stacks)/100==3000-150*stacks);
 }
 std::cout<<"PASS native helper metadata blocks under cold config and repeat loading; Waxing self target/positive flag; Waning one-charge lifecycle; native RemoveSpellMods consumes once even for duplicate modifiers, preserves unused charge\n";
}
