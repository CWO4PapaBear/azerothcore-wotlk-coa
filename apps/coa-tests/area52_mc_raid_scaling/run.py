from pathlib import Path
import subprocess
import tempfile

root=Path(__file__).resolve().parents[3]
src=root/'modules/mod-coa-raid-difficulty/src'
gear=(src/'FlexItems.cpp').read_text()
tokens=(src/'FlexLoot.cpp').read_text()
def body(text,name):
    p=text.index(name)
    start=text.index('{',p)
    depth=0
    for end in range(start,len(text)):
        depth+=(text[end]=='{')-(text[end]=='}')
        if not depth:return text[p:end+1]
    raise ValueError(name)
cpp=r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <random>
#include <set>
#include <vector>
#include <unordered_map>
using uint32=uint32_t; using uint8=uint8_t; using uint16=uint16_t;
const unsigned MAX_NR_LOOT_ITEMS=18,LOOT_MODE_DEFAULT=1;
const uint32 FLEX_MIN_PLAYERS=10,FLEX_MAX_PLAYERS=25,FLEX_LOOT_BONUS_THRESHOLD=20;
const uint32 FLEX_ITEM_MAJORDOMO_ENTRY=12018,GO_CACHE_OF_THE_FIRELORD=179703;
std::mt19937 rng(6);
float frand(float a,float b){return std::uniform_real_distribution<float>(a,b)(rng);}
int urand(int a,int b){return std::uniform_int_distribution<int>(a,b)(rng);}
struct Creature{uint32 id;uint32 GetEntry()const{return id;}};
using GameObject=Creature;
struct Map;
struct Player{Map* m;bool gm;Map* GetMap(){return m;}Map* FindMap(){return m;}bool IsGameMaster(){return gm;}};
struct Map{unsigned mapid=409,mode=0;Creature npc{12118};GameObject go{179703};
std::vector<Player> members;
unsigned GetId(){return mapid;}unsigned GetSpawnMode(){return mode;}
Creature* GetCreature(int g){return g==1?&npc:nullptr;}GameObject* GetGameObject(int g){return g==2?&go:nullptr;}
template<class F>void DoForAllPlayers(F f){for(auto& p:members)f(&p);}};
struct LootStore{}; LootStore LootTemplates_Creature,LootTemplates_Gameobject,other;
struct LootTemplate{};
struct LootItem{uint32 itemid,count;};
struct LootStoreItem{uint32 id;LootStoreItem(uint32 i,int,float,bool,int,int,int,int):id(i){}};
struct Loot{int sourceWorldObjectGUID=1;std::vector<LootItem> items;void AddItem(LootStoreItem i){items.push_back({i.id,1});}};
struct WeightedItem{uint32 item;float weight;};
std::map<uint32,uint32> g_itemCountByMinPlayers;
std::unordered_map<uint32,std::unordered_map<uint32,std::vector<WeightedItem>>> g_poolByDiffAndBoss;
std::unordered_map<uint32,uint32> g_tokenRefByEntry;
std::unordered_map<uint32,std::vector<uint32>> g_tokenItemsByRef;
'''
cpp+='namespace coa_flex {\n'+body((src/'FlexHealth.cpp').read_text(),'uint32 CountPlayers(Map* map)')+'\n'+body(tokens,'uint32 BonusTokenRolls(Map* map)')+'}\n'
cpp+=body(gear,'uint32 ItemCountForPlayers')+'\n'+body(gear,'std::vector<uint32> DrawWithoutReplacement')+'\n'
for name,text in [('Gear',gear),('Token',tokens)]:
    cpp+='struct '+name+' { '+body(text,'void OnAfterLootTemplateProcess').replace(' override','')+' };\n'
cpp+=r'''
int main(){
    Map map;Player owner{&map,false};Gear gear;Token token;
    g_itemCountByMinPlayers={{10,2},{15,3},{20,4}};
    for(unsigned d=0;d<4;++d){
        for(unsigned base:{12118u,12018u})
            for(unsigned i=0;i<7;++i)g_poolByDiffAndBoss[d][base].push_back({100*d+i+10,float(i+1)});
        g_tokenRefByEntry[12118+d*100000]=d+1;g_tokenItemsByRef[d+1]={900+d};
    }
    for(unsigned d=0;d<4;++d)for(unsigned n:{1u,9u,10u,14u,15u,19u,20u,25u,30u}){
        map.mode=d;map.members.assign(n,owner);map.members.push_back({&map,true});
        for(unsigned variant:{0u,d}){
            map.npc.id=12118+100000*variant;
            Loot out;out.items.push_back({111111,1});
            gear.OnAfterLootTemplateProcess(&out,nullptr,LootTemplates_Creature,&owner,false,false,1);
            const unsigned count=n<15?2:n<20?3:4;
            assert(out.items.size()==count+1);
            std::set<uint32> distinct;
            for(unsigned j=1;j<out.items.size();++j){auto id=out.items[j].itemid;assert(id>=100*d+10&&id<=100*d+16);distinct.insert(id);}
            assert(distinct.size()==count);
            if(variant)out.items.push_back({900+d,2});
            token.OnAfterLootTemplateProcess(&out,nullptr,LootTemplates_Creature,&owner,false,false,1);
            unsigned awarded=0;for(auto i:out.items)if(i.itemid==900+d)awarded+=i.count;
            assert(awarded==(n>=20?3u:2u)&&out.items[0].itemid==111111);
        }
    }
    map.npc.id=12018;Loot chest;
    gear.OnAfterLootTemplateProcess(&chest,nullptr,LootTemplates_Creature,&owner,false,false,1);
    assert(chest.items.empty());
    chest.sourceWorldObjectGUID=2;
    gear.OnAfterLootTemplateProcess(&chest,nullptr,LootTemplates_Gameobject,&owner,false,false,1);
    token.OnAfterLootTemplateProcess(&chest,nullptr,LootTemplates_Gameobject,&owner,false,false,1);
    assert(chest.items.size()==4);
    Loot out;map.npc.id=12118;map.mapid=1;
    gear.OnAfterLootTemplateProcess(&out,nullptr,LootTemplates_Creature,&owner,false,false,1);
    token.OnAfterLootTemplateProcess(&out,nullptr,LootTemplates_Creature,&owner,false,false,1);
    assert(out.items.empty());
    map.mapid=409;map.npc.id=512118;
    gear.OnAfterLootTemplateProcess(&out,nullptr,LootTemplates_Creature,&owner,false,false,1);
    token.OnAfterLootTemplateProcess(&out,nullptr,LootTemplates_Creature,&owner,false,false,1);
    assert(out.items.empty());
    map.npc.id=12118;out.items.resize(17);
    gear.OnAfterLootTemplateProcess(&out,nullptr,LootTemplates_Creature,&owner,false,false,1);
    token.OnAfterLootTemplateProcess(&out,nullptr,LootTemplates_Creature,&owner,false,false,1);
    assert(out.items.size()==18);
    assert(DrawWithoutReplacement({{1,1},{2,1}},4).size()==2);
}
'''
with tempfile.TemporaryDirectory() as temp:
    path=Path(temp)/'check.cpp'
    path.write_text(cpp)
    exe=path.with_suffix('')
    subprocess.run(['c++','-std=c++17',str(path),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
