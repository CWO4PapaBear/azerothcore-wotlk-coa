#include <cassert>
#include <cstdint>
#include <iostream>
using uint8=uint8_t; using uint32=uint32_t;
FORM_ENUM
enum WeaponAttackType {BASE_ATTACK,OFF_ATTACK,RANGED_ATTACK};
enum {INVENTORY_SLOT_BAG_0=0,EQUIPMENT_SLOT_MAINHAND=15,EQUIPMENT_SLOT_OFFHAND=16,EQUIPMENT_SLOT_RANGED=17,ITEM_CLASS_WEAPON=2,INVTYPE_WEAPON=13,INVTYPE_WEAPONMAINHAND=21};
struct ItemTemplate {uint32 Class=ITEM_CLASS_WEAPON,InventoryType=INVTYPE_WEAPON;};
struct Item {ItemTemplate data;bool broken=false;auto GetTemplate()const{return &data;}bool IsBroken()const{return broken;}};
struct Player {
 bool area=true,enchant=true,offhand=false,present=true;ShapeshiftForm form=FORM_NONE;mutable Item item;
 auto GetShapeshiftForm()const{return form;}bool HasAura(uint32)const{return enchant;}
 Item* GetItemByPos(int,int slot)const{return slot==EQUIPMENT_SLOT_OFFHAND?(offhand?&item:nullptr):(present?&item:nullptr);}
 Item* GetUseableItemByPos(int bag,int slot)const{return GetItemByPos(bag,slot);}
 Item* GetWeaponForAttack(WeaponAttackType,bool useable=false)const;
 FERAL_METHOD
};
namespace AscensionFreePick {bool Applies(Player* p){return p&&p->area;}}
WEAPON_METHOD
GATE
BASELINE_GATE
int main(){
 Player p;assert(!DefiasWeaponReady(nullptr));assert(DefiasWeaponReady(&p));
 for(auto form:{FORM_BATTLESTANCE,FORM_DEFENSIVESTANCE,FORM_BERSERKERSTANCE,FORM_STEALTH,FORM_SHADOW}){
  p.form=form;assert(!BaselineReady(&p));assert(DefiasWeaponReady(&p));
 }
 for(auto form:{FORM_CAT,FORM_BEAR,FORM_DIREBEAR,FORM_GHOSTWOLF,FORM_VENOMANCER_SPIDER,FORM_VENOMANCER_BEETLE}){
  p.form=form;assert(!DefiasWeaponReady(&p));
 }
 for(int value=0;value<256;++value){p.form=ShapeshiftForm(value);assert(DefiasWeaponReady(&p)==!p.IsInFeralForm());}
 p.form=FORM_SHADOW;
 p.offhand=true;assert(!DefiasWeaponReady(&p));p.offhand=false;
 p.enchant=false;assert(!DefiasWeaponReady(&p));p.enchant=true;
 p.area=false;assert(!DefiasWeaponReady(&p));p.area=true;
 p.item.broken=true;assert(!DefiasWeaponReady(&p));p.item.broken=false;
 p.present=false;assert(!DefiasWeaponReady(&p));p.present=true;
 p.item.data.InventoryType=17;assert(!DefiasWeaponReady(&p));
 p.item.data.InventoryType=INVTYPE_WEAPONMAINHAND;assert(DefiasWeaponReady(&p));
 p.item.data.Class=4;assert(!DefiasWeaponReady(&p));
 std::cout<<"Defias weapon-form checks passed\n";
}
