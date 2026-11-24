#include <cassert>
#include <cstdint>
#include <set>
using uint32=uint32_t;
constexpr int INVENTORY_SLOT_BAG_0=0,EQUIPMENT_SLOT_MAINHAND=15,INVTYPE_2HWEAPON=17,SPEC_MASK_ALL=255;
struct ItemTemplate{int InventoryType=17;int SubClass=8;};
struct Item{ItemTemplate info;ItemTemplate const* GetTemplate(){return &info;}};
struct Player;
namespace AscensionFreePick{struct SpellLearningScope{static inline std::set<uint32> const* current=nullptr;std::set<uint32> const* old;SpellLearningScope(Player*,std::set<uint32> const& s):old(current){current=&s;}~SpellLearningScope(){current=old;}};}
struct Player{
 Item item;bool equipped=true,titan=false,blocked=false;int learns=0,removes=0;std::set<uint32> spells;
 Item* GetItemByPos(int,int)const{return equipped?const_cast<Item*>(&item):nullptr;}
 bool CanTitanGrip()const{return titan;}
 PREDICATE
 bool HasActiveSpell(uint32 s){return spells.contains(s);}
 bool HasSpell(uint32 s){return spells.contains(s);}
 void learnSpell(uint32 s,bool temporary,bool fromSkill=false,bool announce=true){assert(temporary&&!fromSkill&&announce);assert(AscensionFreePick::SpellLearningScope::current->contains(s));if(!blocked){spells.insert(s);++learns;}}
 void removeSpell(uint32 s,int mask,bool temporary){assert(mask==255&&!temporary);spells.erase(s);++removes;}
};
bool Area52Enabled=true;
FUNCTION
int main(){
 for(int subclass:{1,5,6,8,10}){Player p;p.item.info.SubClass=subclass;RefreshTwoHandRequirement(&p);assert(p.HasSpell(997611));RefreshTwoHandRequirement(&p);assert(p.learns==1);p.equipped=false;RefreshTwoHandRequirement(&p);assert(!p.HasSpell(997611));RefreshTwoHandRequirement(&p);assert(p.removes==1);}
 for(int type:{0,13,14,15,21,22,23,26}){Player p;p.item.info.InventoryType=type;p.spells.insert(997611);RefreshTwoHandRequirement(&p);assert(!p.HasSpell(997611));}
 Player p;p.titan=true;RefreshTwoHandRequirement(&p);assert(!p.HasSpell(997611));p.titan=false;RefreshTwoHandRequirement(&p);assert(p.HasSpell(997611));
 p=Player{};p.blocked=true;RefreshTwoHandRequirement(&p);assert(!p.HasSpell(997611));p.blocked=false;RefreshTwoHandRequirement(&p);assert(p.HasSpell(997611));assert(!AscensionFreePick::SpellLearningScope::current);
 for(bool equipmentFirst:{false,true}){p=Player{};p.equipped=equipmentFirst;RefreshTwoHandRequirement(&p);assert(p.HasSpell(997611)==equipmentFirst);p.equipped=true;RefreshTwoHandRequirement(&p);assert(p.HasSpell(997611));p.spells.clear();RefreshTwoHandRequirement(&p);assert(p.HasSpell(997611));}
 Area52Enabled=false;p=Player{};RefreshTwoHandRequirement(&p);assert(p.spells.empty());p.spells.insert(997611);p.equipped=false;RefreshTwoHandRequirement(&p);assert(p.HasSpell(997611));
}
