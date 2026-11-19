from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parent
s=(Path(__file__).resolve().parents[3] / 'src/server/game/Entities/Item/Item.cpp').read_text()
methods=s[s.index('void Item::SetItemRandomProperties('):s.index('\nvoid Item::UpdateItemSuffixFactor(')]
methods+=s[s.index('void Item::SetState('):s.index('\nuint8 Item::GetBagSlot(')]
stub=r'''
#include <cassert>
#include <cstdint>
#include <vector>
#include <memory>
using uint32=std::uint32_t;using int32=std::int32_t;
#define ASSERT(x) assert(x)
#define LOG_DEBUG(...) do{}while(false)
enum ItemUpdateState{ITEM_NEW,ITEM_CHANGED,ITEM_UNCHANGED,ITEM_REMOVED};
enum EnchantmentSlot{PROP_ENCHANTMENT_SLOT_0,PROP_ENCHANTMENT_SLOT_1,PROP_ENCHANTMENT_SLOT_2,PROP_ENCHANTMENT_SLOT_3,MAX_ENCHANTMENT_SLOT};
constexpr int ITEM_FIELD_RANDOM_PROPERTIES_ID=1;
struct ItemRandomPropertiesEntry{uint32 ID=7;uint32 Enchantment[5]={1,2,3,4,5};};
using ItemRandomSuffixEntry=ItemRandomPropertiesEntry;
struct Store{ItemRandomPropertiesEntry entry;ItemRandomPropertiesEntry const*LookupEntry(int32 id){return id==7?&entry:nullptr;}}sItemRandomPropertiesStore,sItemRandomSuffixStore;
struct Item;
struct Player{std::vector<Item*>m_itemUpdateQueue;bool m_itemUpdateQueueBlocked=false;
uint32 GetGUID(){return 1;}void DeleteRefundReference(uint32){};};
struct Item{Player*owner;ItemUpdateState uState=ITEM_NEW;int uQueuePos=-1;int32 property=0;uint32 suffix=0;
uint32 GetGUID(){return 2;}uint32 GetOwnerGUID(){return owner?1:0;}Player*GetOwner(){return owner;}
int32 GetInt32Value(int){return property;}void SetInt32Value(int,int32 v){property=v;}
uint32 GetItemSuffixFactor(){return suffix;}void UpdateItemSuffixFactor(){suffix=123;}
bool IsInUpdateQueue(){return uQueuePos>=0;}
void SetEnchantment(EnchantmentSlot,uint32,uint32,uint32){SetState(ITEM_CHANGED,owner);}
void SetState(ItemUpdateState,Player*);void AddToUpdateQueueOf(Player*);void RemoveFromUpdateQueueOf(Player*);
void SetItemRandomProperties(int32);
};
'''
test=r'''
int main(){for(int property:{7,-7}){Player p;
auto item=std::make_unique<Item>();item->owner=&p;item->SetItemRandomProperties(property);
assert(p.m_itemUpdateQueue.size()==1&&p.m_itemUpdateQueue[0]==item.get());assert(item->uState==ITEM_NEW);
item->RemoveFromUpdateQueueOf(&p);assert(!item->IsInUpdateQueue()&&p.m_itemUpdateQueue[0]==nullptr);
assert(item->property==property);if(property<0)assert(item->suffix==123);
item->SetState(ITEM_CHANGED,&p);assert(item->IsInUpdateQueue()&&p.m_itemUpdateQueue.back()==item.get());
item->RemoveFromUpdateQueueOf(&p);item.reset();for(auto*q:p.m_itemUpdateQueue)assert(q==nullptr);
Player blocked;blocked.m_itemUpdateQueueBlocked=true;auto temp=std::make_unique<Item>();temp->owner=&blocked;
temp->SetItemRandomProperties(property);temp->RemoveFromUpdateQueueOf(&blocked);assert(blocked.m_itemUpdateQueue.empty());
}}
'''
with tempfile.TemporaryDirectory() as folder:
    p=Path(folder);(p/'test.cpp').write_text(stub+methods+test)
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
print('PASS extracted native random-property, SetState, queue add/remove: signed suffixes register temporary items, detachment preserves property, normal later ownership re-registers, blocked queues remain safe.')
