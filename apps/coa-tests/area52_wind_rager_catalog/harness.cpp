#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>
struct ClientDBC{std::vector<char> data;uint32_t count=0,size=0;struct Record{char const* row;char const* pool;uint32_t GetUInt32(uint32_t c)const{uint32_t v;std::memcpy(&v,row+c*4,4);return v;}uint8_t GetUInt8(uint32_t c)const{return uint8_t(row[c]);}char const* GetString(uint32_t c)const{return pool+GetUInt32(c);}};bool Load(std::string const& path,uint32_t){std::ifstream f(path,std::ios::binary);if(!f)return false;data={std::istreambuf_iterator<char>(f),{}};if(data.size()<20)return false;std::memcpy(&count,data.data()+4,4);std::memcpy(&size,data.data()+12,4);return data.size()>=20+count*size;}uint32_t GetRecordCount()const{return count;}uint32_t GetRecordSize()const{return size;}Record GetRecord(uint32_t i)const{return {data.data()+20+i*size,data.data()+20+count*size};}};
std::string GetClientDBCPath(char const* file){return std::string("/data/dbc/")+file;}
struct KnownEntry{uint32_t EntryId,Rank;};
TYPES
constexpr std::array<uint32_t,4> QualityLimits={10,12,11,6};
FUNCTIONS
int main(){Catalog c;assert(LoadCatalog(c));std::vector<std::pair<uint32_t,uint32_t>> selections={SELECTIONS};unsigned passed=0;for(unsigned level=1;level<=80;++level){std::map<uint32_t,uint32_t> chosen;for(auto [spell,at]:selections){if(at>level)continue;bool found=false;for(auto const& [id,e]:c.Entries){auto pos=std::find(e.Spells.begin(),e.Spells.end(),spell);if(pos==e.Spells.end())continue;chosen[id]=std::max(chosen[id],uint32_t(pos-e.Spells.begin()+1));found=true;break;}assert(found);}std::vector<KnownEntry> picks;for(auto [id,rank]:chosen)picks.push_back({id,rank});auto result=Validate(c,level,picks,{});if(!result){std::cerr<<"level "<<level<<" failed "<<result.Learn<<" entry "<<result.EntryId<<" rank "<<result.Rank<<'\n';return 1;}++passed;}
std::vector<std::pair<uint32_t,uint32_t>> members;for(auto [mastery,spell]:std::vector<std::pair<uint32_t,uint32_t>>{{1318,8184},{1318,5675},{1318,5394},{1319,8075},{1319,8071},{1319,5730}}){unsigned found=0;for(auto const&[id,e]:c.Entries)if(std::find(e.Spells.begin(),e.Spells.end(),spell)!=e.Spells.end()){members.push_back({mastery,id});++found;}assert(found==1);}assert(members.size()==6);for(auto [mastery,member]:members){auto selected=IncludeMemberMasteries(c,{{member,1}});assert(std::count_if(selected.begin(),selected.end(),[&](auto e){return e.EntryId==mastery;})==1);auto twice=IncludeMemberMasteries(c,selected);assert(twice.size()==selected.size());assert(c.Entries.at(member).AE==1);Catalog costProbe=c;costProbe.Entries.at(member).Quality=1;costProbe.Entries.at(member).QualityCost=1000;assert(Validate(costProbe,80,selected,{}));costProbe.Entries.at(member).Masteries={};assert(!Validate(costProbe,80,{{member,1}},{}));assert(c.Entries.at(mastery).Flags&0x1000);std::vector<KnownEntry> removal;assert(PlanMasteryRemoval(c,selected,mastery,removal));assert(std::none_of(removal.begin(),removal.end(),[&](auto e){return e.EntryId==member||e.EntryId==mastery;}));}
std::cout<<"PASS "<<passed<<" full build levels through native LoadCatalog/Validate; all six documented totem members: mastery auto-inclusion, duplicate suppression, one AE, zero gem cost and removal planning\n";
}
