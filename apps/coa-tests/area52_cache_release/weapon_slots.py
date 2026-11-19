from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent
source = Path(__file__).resolve().parents[3] / 'src/server/coa'
header = (source / 'Area52CacheItemLevel.h').read_text().replace('#include "Item.h"', '').replace('#include "Player.h"', '')
constants = sorted(set(re.findall(r'\bINVTYPE_[A-Z0-9_]+', header)) | {'INVTYPE_FINGER', 'INVTYPE_TRINKET', 'INVTYPE_HEAD'})
stub = '''#include <cstdint>
#include <algorithm>
#include <array>
#include <cassert>
#include "Area52CacheSelection.h"
using uint32=std::uint32_t;using uint8=std::uint8_t;
constexpr uint32 EQUIPMENT_SLOT_START=0,EQUIPMENT_SLOT_END=19,EQUIPMENT_SLOT_BODY=3,
EQUIPMENT_SLOT_MAINHAND=15,EQUIPMENT_SLOT_OFFHAND=16,EQUIPMENT_SLOT_RANGED=17,EQUIPMENT_SLOT_TABARD=18,
INVENTORY_SLOT_BAG_0=255,ITEM_QUALITY_HEIRLOOM=7;
constexpr uint32 ITEM_SUBCLASS_WEAPON_POLEARM=101,ITEM_SUBCLASS_WEAPON_STAFF=102,ITEM_SUBCLASS_WEAPON_FISHING_POLE=103;
struct ItemTemplate{uint32 Quality=3,ItemLevel=200,InventoryType=0,SubClass=0;};
struct Item{ItemTemplate proto;ItemTemplate const*GetTemplate()const{return &proto;}};
struct Player{std::array<Item const*,19> slots{};bool dual=true,titan=false,shieldException=false;
Item const*GetItemByPos(uint8,uint8 slot)const{return slots[slot];}
bool CanDualWield()const{return dual;}
bool CanTitanGrip(ItemTemplate const* item)const{return titan&&item->SubClass!=ITEM_SUBCLASS_WEAPON_STAFF&&item->SubClass!=ITEM_SUBCLASS_WEAPON_FISHING_POLE&&item->SubClass!=ITEM_SUBCLASS_WEAPON_POLEARM;}
bool CanUseTwoHandWithShield(ItemTemplate const* main,ItemTemplate const* off)const;
};
'''
test = '''
bool Player::CanUseTwoHandWithShield(ItemTemplate const* main,ItemTemplate const* off)const{return shieldException&&main->InventoryType==INVTYPE_2HWEAPON&&main->SubClass==ITEM_SUBCLASS_WEAPON_POLEARM&&off->InventoryType==INVTYPE_SHIELD;}

int main(){using namespace Area52CacheItemLevel; Player p;
Item head{{3,200,INVTYPE_HEAD}},ringLow{{3,187,INVTYPE_FINGER}},ringHigh{{3,213,INVTYPE_FINGER}};
p.slots[0]=&head;p.slots[10]=&ringLow;p.slots[11]=&ringHigh;
ItemTemplate ring{3,200,INVTYPE_FINGER},hat{3,213,INVTYPE_HEAD};
assert(ReplacementWeight(&p,&ring,187)==3);assert(ReplacementWeight(&p,&hat,187)==1);assert(ImprovesSlot(&p,&ring,187));assert(ImprovesSlot(&p,&hat,187));
ring.ItemLevel=187;assert(ReplacementWeight(&p,&ring,187)==1);
ring.ItemLevel=180;assert(ReplacementWeight(&p,&ring,187)==1);assert(!ImprovesSlot(&p,&ring,187));
ringLow.proto.Quality=ITEM_QUALITY_HEIRLOOM;ringLow.proto.ItemLevel=1;ring.ItemLevel=187;
assert(ReplacementWeight(&p,&ring,187)==3);
Item irrelevant{{3,1,INVTYPE_HEAD}};p.slots[EQUIPMENT_SLOT_BODY]=&irrelevant;p.slots[EQUIPMENT_SLOT_TABARD]=&irrelevant;
assert(ReplacementWeight(&p,&ring,187)==3);
ringLow.proto={3,213,INVTYPE_FINGER};assert(ReplacementWeight(&p,&ring,187)==1);
assert(ReplacementWeight(&p,&hat,187)==3);
Item main{{3,213,INVTYPE_WEAPON}},off{{3,180,INVTYPE_WEAPON}};
p.slots[15]=&main;p.slots[16]=&off;
ItemTemplate weapon{3,200,INVTYPE_WEAPON},twohand{3,226,INVTYPE_2HWEAPON};
assert(ReplacementWeight(&p,&weapon,187)==3);assert(ReplacementWeight(&p,&twohand,187)==1);
p.slots[16]=nullptr;assert(ReplacementWeight(&p,&hat,187)==3);
Item chest{{3,180,INVTYPE_ROBE}};p.slots[4]=&chest;ItemTemplate robe{3,200,INVTYPE_CHEST};
assert(ReplacementWeight(&p,&robe,187)==3);
robe.ItemLevel=179;assert(!ImprovesSlot(&p,&robe,187));robe.ItemLevel=181;assert(ImprovesSlot(&p,&robe,187));Player empty;assert(ImprovesSlot(&empty,&hat,187));assert(ReplacementWeight(&empty,&hat,187)==1);
Player setup; Item primary{{3,226,INVTYPE_WEAPON}},shield{{3,182,INVTYPE_SHIELD}},held{{3,182,INVTYPE_HOLDABLE}},secondary{{3,182,INVTYPE_WEAPON}};
setup.slots[15]=&primary;setup.slots[16]=&shield;
ItemTemplate blade{3,213,INVTYPE_WEAPON},newShield{3,200,INVTYPE_SHIELD},newHeld{3,200,INVTYPE_HOLDABLE},offBlade{3,200,INVTYPE_WEAPONOFFHAND},mainBlade{3,200,INVTYPE_WEAPONMAINHAND};
assert(ReplacementWeight(&setup,&blade,182)==1&&!ImprovesSlot(&setup,&blade,182));
assert(ReplacementWeight(&setup,&newShield,182)==3&&ImprovesSlot(&setup,&newShield,182));
assert(!ImprovesSlot(&setup,&newHeld,182)&&!ImprovesSlot(&setup,&offBlade,182));
blade.ItemLevel=232;assert(ImprovesSlot(&setup,&blade,182)&&ReplacementWeight(&setup,&blade,182)==1);blade.ItemLevel=213;
setup.slots[16]=&held;assert(ImprovesSlot(&setup,&newHeld,182)&&ReplacementWeight(&setup,&newHeld,182)==3);assert(!ImprovesSlot(&setup,&newShield,182));
setup.slots[16]=&secondary;assert(ImprovesSlot(&setup,&blade,182)&&ReplacementWeight(&setup,&blade,182)==3);assert(ImprovesSlot(&setup,&offBlade,182));assert(!ImprovesSlot(&setup,&mainBlade,182));assert(!ImprovesSlot(&setup,&newShield,182));
setup.dual=false;assert(!ImprovesSlot(&setup,&blade,182)&&!ImprovesSlot(&setup,&offBlade,182));setup.dual=true;
offBlade.SubClass=ITEM_SUBCLASS_WEAPON_FISHING_POLE;assert(!ImprovesSlot(&setup,&offBlade,182));offBlade.SubClass=0;
Item great{{3,200,INVTYPE_2HWEAPON}},otherGreat{{3,182,INVTYPE_2HWEAPON}};ItemTemplate betterGreat{3,213,INVTYPE_2HWEAPON};
setup.slots[15]=&great;setup.slots[16]=nullptr;
assert(ImprovesSlot(&setup,&betterGreat,182)&&ReplacementWeight(&setup,&betterGreat,182)==3);assert(!ImprovesSlot(&setup,&blade,182)&&!ImprovesSlot(&setup,&newShield,182)&&!ImprovesSlot(&setup,&newHeld,182)&&!ImprovesSlot(&setup,&offBlade,182));
setup.slots[16]=&otherGreat;setup.titan=true;betterGreat.ItemLevel=195;assert(ImprovesSlot(&setup,&betterGreat,182)&&ReplacementWeight(&setup,&betterGreat,182)==3);setup.titan=false;assert(!ImprovesSlot(&setup,&betterGreat,182));
setup.slots[16]=&shield;great.proto.SubClass=ITEM_SUBCLASS_WEAPON_POLEARM;setup.shieldException=true;betterGreat.ItemLevel=213;betterGreat.SubClass=ITEM_SUBCLASS_WEAPON_POLEARM;
assert(ImprovesSlot(&setup,&newShield,182)&&ReplacementWeight(&setup,&newShield,182)==3);assert(ImprovesSlot(&setup,&betterGreat,182));assert(!ImprovesSlot(&setup,&blade,182)&&!ImprovesSlot(&setup,&newHeld,182));
setup.shieldException=false;assert(!ImprovesSlot(&setup,&newShield,182)&&!ImprovesSlot(&setup,&betterGreat,182));
Player unarmed;assert(ImprovesSlot(&unarmed,&blade,182)&&ImprovesSlot(&unarmed,&betterGreat,182));assert(!ImprovesSlot(&unarmed,&newShield,182)&&!ImprovesSlot(&unarmed,&offBlade,182));
using namespace Area52CacheSelection;
std::vector<Candidate> progression;
for(uint32 level:{182u,213u,226u,232u,239u}){Candidate entry;entry.item=level;entry.itemLevel=level;entry.requiredLevel=80;entry.contentUnlocked=entry.usable=entry.bonusesQualified=true;entry.improvesSlot=level>226;progression.push_back(entry);}
auto band=OfferRange(progression,226,80,6,1);assert(band.minimum==226&&band.maximum==232);
auto eligible=Select(progression,band.minimum,band.maximum,{},80);assert(eligible.choices.size()==1&&eligible.choices[0].item==232);
for(auto& entry:progression)entry.improvesSlot=false;
assert(Select(progression,0,999,{},80).choices.empty());assert(Select(progression,0,999,{},80).outcome==Outcome::NoEligibleReward);
Candidate a,b;a.item=1;b.item=2;a.replacementWeight=3;
auto choices=WeightedChoices({&a,&b});assert(choices.size()==2&&choices[0].weight==3&&choices[1].weight==1);
a.replacementWeight=100;choices=WeightedChoices({&a,&b});assert(choices[0].weight==3);
a.replacementWeight=0;choices=WeightedChoices({&a,&b});assert(choices[0].weight==1);
}
'''
with tempfile.TemporaryDirectory() as folder:
    path = Path(folder)
    (path / 'test.cpp').write_text(stub + '\nenum {' + ','.join(constants) + '};\n' + header + test)
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-I',str(source),str(path/'test.cpp'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True)
print('PASS native lowest-equipped-slot weighting: pairs, upgrades, heirloom floor, cosmetic/empty slots, weapon hands, chest/robe and bounded threefold weight. Existing eligibility/range tests remain separate.')
