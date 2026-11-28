#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "SpellChargeState.h"
using uint32=uint32_t;using uint64=uint64_t;using int32=int32_t;using Milliseconds=std::chrono::milliseconds;
constexpr uint32 DAY=86400,IN_MILLISECONDS=1000;constexpr int CHEAT_COOLDOWN=1,CHEAT_SPELLCHARGES=2,SPELLMOD_COOLDOWN=1;
uint64 now=100000;namespace GameTime{std::chrono::system_clock::time_point GetSystemTime(){return std::chrono::system_clock::time_point(Milliseconds(now));}}
struct SpellInfo{uint32 Id=278745,MaxCharges=3,ChargeRecoveryKey=278745,ChargeRecoveryTime=8000;};struct Spell{};struct Setting{uint32 value=0;};
struct Player{std::map<std::string,std::vector<Setting>>m_charSettingsMap;int updates=0,sends=0;bool cheat=false;bool HasAura(uint32)const{return false;}bool GetCommandStatus(int)const{return cheat;}void ApplySpellMod(uint32,int,int32&,Spell*){}void UpdatePlayerSetting(std::string const&s,uint32 index,uint32 value){auto&row=m_charSettingsMap[s];row.resize(std::max(row.size(),size_t(index+1)));row[index].value=value;++updates;}void SendSpellChargeState(uint32){++sends;}SpellChargeState GetSpellCharges(SpellInfo const*)const;void StoreSpellCharges(SpellInfo const*,SpellChargeState const&);void ConsumeSpellCharge(SpellInfo const*,Spell*);};
namespace AscensionFreePick{bool Applies(Player const*){return true;}}
struct Scripts{int events=0;void OnPlayerSpellChargeConsumed(Player*,SpellInfo const*,Spell*,uint32 recovery,uint64 timestamp){assert(recovery==8000&&timestamp==now);++events;}}scripts,*sScriptMgr=&scripts;
@METHODS@
int main(){Player p;SpellInfo info;assert(p.GetSpellCharges(&info).Available==3);for(int i=2;i>=0;--i){p.ConsumeSpellCharge(&info,nullptr);auto s=p.GetSpellCharges(&info);assert(s.Available==uint32(i)&&s.NextRecovery==108000&&s.RecoveryTime==8000);}int before=p.updates;int events=scripts.events;p.ConsumeSpellCharge(&info,nullptr);assert(p.updates==before&&scripts.events==events);Player relog;relog.m_charSettingsMap=p.m_charSettingsMap;now=107999;assert(relog.GetSpellCharges(&info).Available==0);now=108000;assert(relog.GetSpellCharges(&info).Available==1);relog.ConsumeSpellCharge(&info,nullptr);assert(relog.GetSpellCharges(&info).Available==0&&relog.GetSpellCharges(&info).NextRecovery==116000);now=132000;auto full=relog.GetSpellCharges(&info);assert(full.Available==3&&full.NextRecovery==0);relog.StoreSpellCharges(&info,full);Player restart;restart.m_charSettingsMap=relog.m_charSettingsMap;assert(restart.GetSpellCharges(&info).Available==3);restart.cheat=true;before=restart.updates;restart.ConsumeSpellCharge(&info,nullptr);assert(restart.updates==before);}
