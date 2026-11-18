import os
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[3]
source = (root / 'src/server/coa/Area52MysticScrolls.inc').read_text()
code = source[source.index('Entry const* DrawScroll()'):source.index('void HandleScrollRequest(')]
prefix = r"""
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
using uint32=std::uint32_t;using int32=std::int32_t;
struct Entry { uint32 Spell=0,Item=0; };
struct {std::map<uint32,Entry> BySpell;std::map<uint32,uint32> ByItem;} Data;
uint32 pick=0;uint32 urand(uint32 lo,uint32 hi){assert(lo==0&&pick<=hi);return pick;}
struct State {std::set<uint32> Known;};
struct Player;
struct Item {uint32 Entry=97866,Count=2;bool trade=false;uint32 GetEntry()const{return Entry;}uint32 GetCount()const{return Count;}bool IsInTrade()const{return trade;}int GetBagSlot()const{return 0;}int GetSlot()const{return 3;}static Item* CreateItem(uint32 entry,uint32 count,Player*){return new Item{entry,count};}};
struct Manager {bool exists=true;bool GetItemTemplate(uint32){return exists;}} manager;auto* sObjectMgr=&manager;
using ItemPosCountVec=std::vector<int>;using InventoryResult=int;
constexpr int EQUIP_ERR_OK=0,NULL_BAG=0,NULL_SLOT=255,MysticRuneItem=375250,MysticExtractItem=98463,CHAR_INS_FREEPICK_MYSTIC_COLLECTION=0;
struct Statement {void SetData(int,uint32){}};
struct Transaction {void Append(Statement*){}};
struct Database {int commits=0;Statement stmt;auto BeginTransaction(){return std::make_shared<Transaction>();}Statement* GetPreparedStatement(int){return &stmt;}void CommitTransaction(std::shared_ptr<Transaction>){++commits;}} CharacterDatabase;
struct Session {uint32 GetAccountId(){return 55;}};
struct WorldPacket {WorldPacket(int,int){}WorldPacket& operator<<(uint32){return *this;}};
struct Player {State state;Session session;uint32 money=25000,runes=5000,extracts=1,consumed=0,newItem=0,notifications=0;bool space=true;int saves=0;Item* held=nullptr;
~Player(){delete held;}
uint32 GetItemCount(uint32 id){return id==MysticRuneItem?runes:extracts;}bool HasItemCount(uint32 id,uint32 n){return GetItemCount(id)>=n;}uint32 GetMoney(){return money;}
int CanStoreItem(int,int,ItemPosCountVec&,Item*,bool swap=false){return (space||swap)?0:1;}
void SendEquipError(int,void*,void*,uint32){}void DestroyItemCount(Item* i,uint32& n,bool){consumed+=n;i->Count-=n;n=0;}void DestroyItemCount(uint32 id,uint32 n,bool){auto& v=id==MysticRuneItem?runes:extracts;assert(v>=n);v-=n;}
void ModifyMoney(int n){money+=n;}Item* StoreItem(ItemPosCountVec&,Item* i,bool){held=i;newItem=i->Entry;return i;}void SendNewItem(Item*,int,bool,bool){++notifications;}
void SaveInventoryAndGoldToDB(std::shared_ptr<Transaction>){++saves;}Session* GetSession(){return &session;}void SendDirectMessage(WorldPacket*){++notifications;}};
bool change=true,altar=true;bool CanChange(Player*,State*){return change;}bool NearAltar(Player*){return altar;}uint32 ScrollExtractPrice(Player*){return 1200;}void SendResult(Player*,int,char const*){}
"""
suffix = r"""
int main(){
 Data.BySpell={{10,{10,100}},{20,{20,200}},{30,{30,300}}};Data.ByItem={{100,10},{200,20},{300,30}};
 for(uint32 i=0;i<3;++i){pick=i;auto* e=DrawScroll();assert(e&&e->Item==100+i*100);Player p;Item item;assert(ReplaceMysticScroll(&p,&item,e->Item,0,0));assert(p.newItem==e->Item&&p.consumed==1&&p.state.Known.empty());}
 {Player p;Item item;p.space=false;assert(!ReplaceMysticScroll(&p,&item,100,250,0));assert(p.consumed==0&&p.runes==5000&&item.Count==2);}
 {Player p;Item item;item.Count=1;p.space=false;assert(ReplaceMysticScroll(&p,&item,100,250,0));assert(p.consumed==1&&p.runes==4750);}
 {Player p;Item item;item.trade=true;assert(!ReplaceMysticScroll(&p,&item,100,0,0));assert(!p.consumed);}
 {Player p;Item item;p.runes=0;assert(!ReplaceMysticScroll(&p,&item,100,250,0));assert(!p.consumed);manager.exists=false;assert(!ReplaceMysticScroll(&p,&item,100,0,0));manager.exists=true;}
 for(bool nearby : {false,true}) for(bool buy : {false,true}) for(bool funded : {false,true}) {
     Player p; Item item{100,2}; altar=nearby;
     p.extracts=funded?1:0; p.runes=funded?5000:0;
     int commits=CharacterDatabase.commits;
     assert(!CollectMysticScroll(&p,&p.state,&item,buy));
     assert(p.state.Known.contains(10)&&item.Count==1&&p.consumed==1);
     assert(p.extracts==(funded?1u:0u)&&p.runes==(funded?5000u:0u)&&p.money==25000);
     assert(p.saves==1&&CharacterDatabase.commits==commits+1);
     assert(CollectMysticScroll(&p,&p.state,&item,buy));
     assert(item.Count==1&&CharacterDatabase.commits==commits+1);
 }
 for(int reason=0;reason<4;++reason){
     Player p;Item item{100,1};change=reason!=0;item.trade=reason==1;
     if(reason==2)item.Entry=999;
     int commits=CharacterDatabase.commits;
     assert(CollectMysticScroll(&p,&p.state,reason==3?nullptr:&item,true));
     assert(!p.consumed&&p.state.Known.empty()&&CharacterDatabase.commits==commits);
 }
 Data.BySpell.clear();assert(!DrawScroll());
}
"""
with tempfile.TemporaryDirectory() as directory:
    cpp = Path(directory) / 'test.cpp'
    binary = Path(directory) / 'test'
    cpp.write_text(prefix + code + suffix)
    subprocess.run([os.environ.get('CXX', 'c++'), '-std=c++20', str(cpp), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print('PASS: full-pool draw creates items without learning; bag capacity; payment; trade protection; free collection near or away from altar; duplicate and invalid item rejection')

