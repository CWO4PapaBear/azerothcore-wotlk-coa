#include "Area52BotWakeSchedule.h"
#include <cassert>
#include <map>
using Schedule=Area52BotWakeSchedule;
using Time=Schedule::Clock::time_point;
Time At(int seconds){return Time{}+std::chrono::seconds(seconds);}
std::vector<Schedule::Bot> Roster(unsigned size){
 std::vector<Schedule::Bot> out;
 for(unsigned f=0;f<2;++f)for(unsigned party=0;party<size/10;++party)
  for(unsigned role:{1u,2u,3u,3u,3u})out.push_back({unsigned(out.size()+1),f,role,false,true});
 return out;
}
void Mark(std::vector<Schedule::Bot>& bots,std::vector<unsigned> const& ids){
 for(auto& b:bots)if(std::find(ids.begin(),ids.end(),b.id)!=ids.end())b.online=true;
}
void CheckParty(std::vector<Schedule::Bot> const& bots,std::vector<unsigned> const& ids,unsigned faction){
 assert(ids.size()==5);std::map<unsigned,unsigned> roles;
 for(auto id:ids){auto it=std::find_if(bots.begin(),bots.end(),[id](auto& b){return b.id==id;});assert(it!=bots.end()&&it->faction==faction);++roles[it->role];}
 assert(roles[1]==1&&roles[2]==1&&roles[3]==3);
}
int main(){
 for(unsigned total:{20u,100u}){
  auto bots=Roster(total);Schedule s;s.Observe(true,true,At(0));
  for(unsigned batch=0;batch<total/5;++batch){
   unsigned due=60*(batch+1);s.Observe(true,true,At(due-1));
   assert(s.Take(At(due-1),total,bots).empty());auto ids=s.Take(At(due),total-5*batch,bots);
   CheckParty(bots,ids,batch%2);Mark(bots,ids);assert(s.Take(At(due),total,bots).empty());
  }
  assert(std::all_of(bots.begin(),bots.end(),[](auto& b){return b.online;}));
  assert(s.Take(At(3000),0,bots).empty());bots[0].online=false;
  auto refill=s.Take(At(3060),1,bots);assert(refill==std::vector<unsigned>{1});
 }
 auto bots=Roster(20);Schedule s;s.Observe(true,true,At(0));
 auto first=s.Take(At(600),20,bots);CheckParty(bots,first,0);assert(s.Take(At(601),20,bots).empty());
 std::vector<unsigned> partial(first.begin()+1,first.end());Mark(bots,partial);
 auto retry=s.Take(At(660),16,bots);assert(retry==std::vector<unsigned>{first.front()});
 Mark(bots,retry);auto second=s.Take(At(720),15,bots);CheckParty(bots,second,1);
 s.Observe(true,false,At(721));assert(s.Take(At(800),20,bots).empty());
 s.Observe(true,true,At(900));assert(s.Take(At(959),20,bots).empty());
 assert(!s.Take(At(960),20,bots).empty());s.Observe(false,true,At(1000));assert(s.Take(At(2000),20,bots).empty());
 Schedule absent;absent.Observe(true,false,At(0));assert(absent.Take(At(600),20,bots).empty());
 Schedule cap;cap.Observe(true,true,At(0));assert(cap.Take(At(60),4,Roster(20)).empty());
 Schedule corrupt;corrupt.Observe(true,true,At(0));auto invalid=Roster(20);invalid.push_back(invalid.front());assert(corrupt.Take(At(60),20,invalid).empty());
 Schedule roles;roles.Observe(true,true,At(0));auto missing=Roster(20);for(auto& b:missing)if(b.role==2)b.eligible=false;assert(roles.Take(At(60),20,missing).empty());
}
