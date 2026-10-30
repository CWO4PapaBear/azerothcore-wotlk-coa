from pathlib import Path
import json
import struct
import subprocess
import tempfile

root = Path.cwd()
module = root/'modules/mod-playerbots/src'

def block(text, marker):
    start = text.index(marker)
    opening = text.index('{', start)
    depth, end = 1, opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]

def run(text):
    with tempfile.TemporaryDirectory() as temporary:
        path = Path(temporary)
        (path/'test.cpp').write_text(text)
        subprocess.run(['clang++', '-std=c++20', '-I'+str(module/'Ai/Coa'), str(path/'test.cpp'), '-o', str(path/'test')], check=True)
        subprocess.run([str(path/'test')], check=True)

pool = (module/'Ai/Coa/CoaDungeonPool.cpp').read_text()
factory = (module/'Bot/Factory/PlayerbotFactory.cpp').read_text()
ensure = block(pool, 'bool EnsureEquipment(Player* bot)')
destroy = block(factory, 'void PlayerbotFactory::DestroyEquippedGear(Player* bot)')
visitor = block(factory[factory.index('class DestroyItemsVisitor'):], 'bool Visit(Item* item) override')
expected = []
for role, name in enumerate(('mktest', 'naturalist', 'hopstrike'), 1):
    data = json.loads((Path('/results')/(name+'-gear-reference.json')).read_text())
    slots = [0]*19
    for item in data['equipment']:
        slots[int(item['slot'])] = int(item.get('item_id', item.get('itemEntry')))
    expected.append('assert((Area52FixedEquipment::Profiles['+str(role-1)+'] == std::array<uint32,19>{'+','.join(map(str,slots))+'}));')

fixture = r'''
#include "Area52FixedEquipment.h"
#include <cassert>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
using uint32=unsigned;using uint16=unsigned short;using uint8=unsigned char;
constexpr unsigned EQUIPMENT_SLOT_START=0,EQUIPMENT_SLOT_END=19,INVENTORY_SLOT_BAG_0=0,EQUIP_ERR_OK=0,EQUIPMENT_SLOT_TABARD=18,EQUIPMENT_SLOT_BODY=3;
constexpr unsigned INVENTORY_SLOT_ITEM_START=23,INVENTORY_SLOT_ITEM_END=39,INVENTORY_SLOT_BAG_START=19,INVENTORY_SLOT_BAG_END=23;
struct Item {unsigned entry,guid,slot=0;struct Template{unsigned ItemId;};Template temp{};Template* GetTemplate(){temp.ItemId=entry;return &temp;}unsigned GetEntry(){return entry;}unsigned GetBagSlot(){return 0;}unsigned GetSlot(){return slot;}};
struct Bag {unsigned GetBagSize(){return 0;}Item* GetItemByPos(unsigned){return nullptr;}};
struct Player {bool pilot=true,vanity=true;unsigned role=1,level=70,next=100,saves=0,deleted=0,blocked=99;
struct Setting{unsigned value;};std::vector<Setting> roles;auto* FindPlayerSettings(char const*){roles={{role}};return &roles;}
std::map<unsigned,Item> items;std::map<unsigned,Item>::node_type detached;
Bag* GetBagByPos(unsigned){return nullptr;}unsigned CanEquipItem(unsigned s,uint16& d,Item*,bool){d=s;return s==blocked?1:0;}
void RemoveItem(unsigned,unsigned s,bool){detached=items.extract(s);}Item* EquipItem(unsigned s,Item* i,bool){auto copy=*i;copy.slot=s;items[s]=copy;detached={};return &items.at(s);}
Item* GetItemByPos(unsigned,unsigned s){auto i=items.find(s);return i==items.end()?nullptr:&i->second;}
unsigned CanEquipNewItem(unsigned s,uint16& dest,unsigned,bool){dest=s;return s==blocked?1:0;}
void DestroyItem(unsigned,unsigned s,bool){items.erase(s);++deleted;}
Item* EquipNewItem(unsigned s,unsigned e,bool){items.insert_or_assign(s,Item{e,next++,s});return &items.at(s);}
void SaveToDB(bool,bool){++saves;}
};
struct ObjectMgr {unsigned missing=0;bool GetItemTemplate(unsigned e){return e!=missing;}} objects;auto* sObjectMgr=&objects;
bool ApplyArea52PilotVanity(Player* p){return p->vanity;}
namespace CoaDungeonPool {bool IsPilot(Player* p){return p&&p->pilot;}unsigned Role(Player* p){return p->role;}
'''
fixture += ensure + '\n}\nstruct PlayerbotFactory{static void DestroyEquippedGear(Player*);};\n' + destroy
fixture += '\nstruct IterateItemsVisitor{virtual bool Visit(Item*)=0;};struct Cleanup:IterateItemsVisitor{Player* bot;std::set<unsigned> keep;bool CanKeep(unsigned){return false;}'+visitor+'};\n'
fixture += '\nint main(){\n'+'\n'.join(expected)+r'''
for(unsigned role=1;role<=3;++role){
Player p;p.role=role;p.items.emplace(0,Item{123,1});
assert(CoaDungeonPool::EnsureEquipment(&p));
auto saved=p.items;auto saves=p.saves;auto next=p.next;Cleanup cleanup;cleanup.bot=&p;for(auto& [slot,item]:p.items)assert(cleanup.Visit(&item));assert(p.items.size()==saved.size());
for(unsigned level : {15,80,20,70,15}){p.level=level;PlayerbotFactory::DestroyEquippedGear(&p);assert(CoaDungeonPool::EnsureEquipment(&p));
assert(p.next==next&&p.saves==saves&&p.items.size()==saved.size());for(auto [slot,item]:saved)assert(p.items.at(slot).guid==item.guid);}
p.vanity=false;assert(!CoaDungeonPool::EnsureEquipment(&p));p.vanity=true;assert(CoaDungeonPool::EnsureEquipment(&p));assert(p.next==next);
}
Player partial;partial.blocked=16;assert(!CoaDungeonPool::EnsureEquipment(&partial));auto first=partial.items;
partial.blocked=99;assert(CoaDungeonPool::EnsureEquipment(&partial));for(auto [slot,item]:first)assert(partial.items.at(slot).guid==item.guid);
Player bagged;assert(CoaDungeonPool::EnsureEquipment(&bagged));auto shield=bagged.items.at(16);bagged.items.erase(16);shield.slot=23;bagged.items[23]=shield;auto before=bagged.next;assert(CoaDungeonPool::EnsureEquipment(&bagged));assert(bagged.items.at(16).guid==shield.guid&&bagged.next==before&&!bagged.items.contains(23));
Player missing;objects.missing=Area52FixedEquipment::Profiles[0][0];assert(!CoaDungeonPool::EnsureEquipment(&missing));assert(missing.items.empty()&&missing.next==100);objects.missing=0;
Player human;human.pilot=false;human.items.emplace(0,Item{111,2});assert(!CoaDungeonPool::EnsureEquipment(&human));assert(human.items.at(0).guid==2&&human.saves==0);
assert(!CoaDungeonPool::EnsureEquipment(nullptr));
}
'''
run(fixture)
prepare = block(pool, 'bool Prepare(Player* bot, uint32 level)')
assert 'DestroyEquippedGear' not in prepare
init = block(factory, 'void PlayerbotFactory::InitEquipment(')
assert init.index('CoaDungeonPool::EnsureEquipment(bot)') < init.index('if (level < 5)')
assert 'CanEquipWeapon' not in ensure
print('PASS: production fixed assignment, exact owner profiles, existing item GUID retention through level changes/regear, partial retry, missing catalog and human isolation. Native equip service is an adapter, not gameplay.')

def dbc(name, fields):
    data = (Path('/data/dbc')/(name+'.dbc')).read_bytes()
    magic,count,columns,size,strings = struct.unpack_from('<4s4I', data)
    assert magic == b'WDBC' and columns >= fields and size == columns*4
    return {row[0]:row for row in (struct.unpack_from('<'+'I'*columns,data,20+i*size) for i in range(count))}

appearances = dbc('Appearances',9)
sets = dbc('ItemSet',35)
mapping = {row[1]:row[2] for row in dbc('ItemAppearances',3).values()}
initializers, coverage = [], []
for role, appearance in enumerate((900527,900501,900573),1):
    record = appearances[appearance]
    set_id = record[3]
    assert set_id in sets
    items = [item for item in sets[set_id][18:35] if item]
    initializers.append('s._appearances['+str(appearance)+']={'+str(set_id)+',55,0,0};')
    initializers.append('s._itemSetItems['+str(set_id)+']={'+','.join(map(str,items))+'};')
    covered = []
    for item in items:
        aid = mapping[item]
        row = appearances[aid]
        initializers.append('s._itemAppearances['+str(item)+']='+str(aid)+';')
        initializers.append('s._appearances['+str(aid)+']={'+','.join(map(str,(row[3],row[5],row[6],row[7])))+'};')
        covered.append({'item':item,'appearance':aid,'categories':list(row[5:8])})
    coverage.append({'role':role,'set_appearance':appearance,'items':covered})
core = (root/'src/server/coa/AscensionCompat.cpp').read_text()
apply = block(core, 'bool ApplyPilotVanity(Player* player)')
expand = block(core, 'bool ExpandItemSetAppearance(')
vanity = r'''
#include <array>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <cassert>
using uint32=unsigned;constexpr unsigned APPEARANCE_CATEGORY_COUNT=64;
struct Setting{unsigned value=1;};struct Player {bool area=true,loaded=true;std::vector<Setting> roles{{1}};auto* FindPlayerSettings(char const*){return &roles;}};
namespace AscensionFreePick {bool Applies(Player* p){return p&&p->area;}}
struct State {std::array<uint32,APPEARANCE_CATEGORY_COUNT> ActiveAppearances{};};
struct AppearanceInfo{unsigned SourceItem,PrimaryCategory,SecondaryCategory,TertiaryCategory;};
struct Service {bool _clientDataLoaded=false;unsigned saves=0,refreshes=0;
std::shared_ptr<State> state=std::make_shared<State>();std::map<unsigned,AppearanceInfo> _appearances;
std::map<unsigned,std::vector<unsigned>> _itemSetItems;std::map<unsigned,unsigned> _itemAppearances;
auto GetState(Player* p){return p->loaded?state:nullptr;}void SaveActiveAppearances(Player*,State const&){++saves;}
void RefreshVisibleItems(Player*){++refreshes;}
'''+apply+'\n'+expand+'\n};\nint main(){Service s;Player p;\n'+'\n'.join(initializers)+r'''
assert(!s.ApplyPilotVanity(&p));assert(!s.saves);
s._clientDataLoaded=true;p.loaded=false;assert(!s.ApplyPilotVanity(&p));p.loaded=true;
for(unsigned role=1;role<=3;++role){p.roles[0].value=role;assert(s.ApplyPilotVanity(&p));auto saved=s.state->ActiveAppearances;auto n=s.saves;assert(saved[55]);
assert(s.ApplyPilotVanity(&p));assert(s.saves==n&&s.state->ActiveAppearances==saved);
s._clientDataLoaded=false;assert(!s.ApplyPilotVanity(&p));s._clientDataLoaded=true;assert(s.ApplyPilotVanity(&p));assert(s.saves==n);
s.state=std::make_shared<State>();s.state->ActiveAppearances=saved;assert(s.ApplyPilotVanity(&p));assert(s.saves==n);
}
p.area=false;assert(!s.ApplyPilotVanity(&p));p.area=true;p.roles[0].value=0;assert(!s.ApplyPilotVanity(&p));p.roles.clear();assert(!s.ApplyPilotVanity(&p));
}
'''
run(vanity)
Path('/results/fixed-vanity-coverage.json').write_text(json.dumps(coverage,indent=2))
print('PASS: production Vanity expansion with deployed DBC, startup unready-to-ready, saved-state restoration, repeated calls and other-realm isolation. No equipment/stat mutation in appearance path; persistence/session services bounded offline.')
