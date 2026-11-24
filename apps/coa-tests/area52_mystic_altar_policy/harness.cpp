#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
using uint32=uint32_t;
struct Config {
 bool enabled=true;std::string model="hero",realm="live",mode="";uint32 mask=0;
 template<class T>T GetOption(std::string key,T fallback){
  if constexpr(std::is_same_v<T,bool>)return enabled;
  else if constexpr(std::is_same_v<T,std::string>){if(key=="CoA.ClassModel")return model;if(key=="CoA.RealmType")return realm;return mode;}
  else return mask;
 }
} config;auto sConfigMgr=&config;
namespace AscensionWildcard{bool PlaysWildcard(std::string mode){return mode=="wildcard";}}
namespace AscensionFreePick{constexpr uint32 QualityMode=0x400;}
namespace Area52MysticRealm {ENGINE}
namespace AscensionFreepick {bool MysticAltars=false;POLICY}
VENDOR
struct Service {
 ITEMS
 bool IsBrokenCosmeticPetToken(uint32 id)const{return id==999;}
 WITHHELD
};
int main(){
 Service service;
 assert(AscensionFreepick::RealmOffersMysticAltars());assert(SellsAltars());
 for(auto id:Service::MysticAltarVanityItems)assert(!service.IsWithheldVanityItem(id));
 assert(service.IsWithheldVanityItem(999));assert(!service.IsWithheldVanityItem(3648545));
 for(auto model:{"coa","wcr","hero"})for(bool upstream:{false,true}){
  config.model=model;config.realm="seasonal";AscensionFreepick::MysticAltars=upstream;
  assert(AscensionFreepick::RealmOffersMysticAltars()==upstream);
 }
 config.model="hero";config.realm="live";AscensionFreepick::MysticAltars=false;
 config.mode="wildcard";assert(!SellsAltars());config.mode="";
 config.enabled=false;assert(!SellsAltars());config.enabled=true;
 config.mask=2;assert(!SellsAltars());config.mask=0;
 assert(SellsAltars());
 config.model="coa";for(auto id:Service::MysticAltarVanityItems)assert(service.IsWithheldVanityItem(id));
 assert(!service.IsWithheldVanityItem(3648545));assert(!service.IsWithheldVanityItem(123));
}
