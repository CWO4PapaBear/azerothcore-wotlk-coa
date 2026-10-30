from pathlib import Path
import json
import re
import struct
import subprocess
import tempfile

root = Path.cwd()
module = root/'modules/mod-playerbots/src'
inventory = {int(row['entry']): row for row in json.loads(Path('/results/heirloom-inventory.json').read_text())}
header = (module/'Ai/Coa/Area52Heirloom.h').read_text()
ids = sorted(set(map(int, re.findall(r'case (\d+):', header))) | set(range(1339079,1339134)))
ids.remove(1339091)
native = (root/'src/server/shared/DataStores/DBCStructure.h').read_text()
native = native[native.index('struct ScalingStatDistributionEntry'):native.index('//struct SkillLineCategoryEntry')]
source = (module/'Mgr/Item/StatsWeightCalculator.cpp').read_text()
start = source.index('    if (CoaDungeonPool::IsPilot(player_)')
end = source.index('    if (randomPropertyIds',start)
scaling = source[start:end]

def dbc(name, fields):
    path = Path('/data/dbc')/(name+'.dbc')
    data = path.read_bytes()
    magic,count,columns,size,strings = struct.unpack_from('<4s4I', data)
    assert magic == b'WDBC' and columns == fields and size == fields*4
    return {values[0]: values for values in (struct.unpack_from('<'+'I'*fields,data,20+i*size) for i in range(count))}

distributions = dbc('ScalingStatDistribution',22)
values = {row[1]:row for row in dbc('ScalingStatValues',24).values()}
coverage = []
initialization = []
for entry in ids:
    row = inventory[entry]
    distribution = int(row['ScalingStatDistribution'])
    mask = int(row['ScalingStatValue'])
    assert row['Quality']=='7' and distribution in distributions and mask and row['class'] in ('2','4')
    assert int(row['RequiredLevel'])<=15 and int(row['AllowableClass']) & (1<<9)
    assert all(level in values for level in range(15,81))
    coverage.append({'id':entry,'name':row['name'],'slot':int(row['InventoryType']),'maximum_scaling_level':distributions[distribution][21]})
    initialization.append(f'items.push_back({{{entry},{distribution},{mask},{row["delay"]},{row["armor"]},{row["block"]}}});')
fixture = r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <vector>
#include "Area52Heirloom.h"
using uint32=std::uint32_t;using int32=std::int32_t;
constexpr unsigned MAX_ITEM_PROTO_STATS=10,STATS_TYPE_SPELL_POWER=60,STATS_TYPE_HEAL_POWER=61;
'''+native+r'''
template<class T>struct Store{std::map<unsigned,T> rows;T const* LookupEntry(unsigned id){auto i=rows.find(id);return i==rows.end()?nullptr:&i->second;}};
Store<ScalingStatDistributionEntry> sScalingStatDistributionStore;
Store<ScalingStatValuesEntry> sScalingStatValuesStore;
struct ItemTemplate{unsigned id,ScalingStatDistribution,ScalingStatValue,Delay,Armor,Block;unsigned StatsCount=1;struct Stat{unsigned ItemStatType=0;int ItemStatValue=99999;}ItemStat[10];struct Dmg{float DamageMin=1,DamageMax=3;}Damage[2];};
struct Player{unsigned level=15;bool pilot=true;std::uint8_t GetLevel(){return level;}}player;
namespace CoaDungeonPool{bool IsPilot(Player* p){return p->pilot;}}
struct Collector{float stats[64]{};ItemTemplate collected{};void CollectItemStats(ItemTemplate const* p){collected=*p;for(unsigned i=0;i<p->StatsCount;++i)stats[p->ItemStat[i].ItemStatType]+=p->ItemStat[i].ItemStatValue;}};
float Run(ItemTemplate const* proto,Player* player_,Collector* collector_){
'''+scaling+r'''
return 1.0f;
}
int main(){
std::vector<ItemTemplate> items;
'''
for ident, row in distributions.items():
    raw = ','.join(str(v)+'u' for v in row)
    fixture += f'{{uint32 raw[]={{ {raw} }};ScalingStatDistributionEntry entry;static_assert(sizeof(entry)==sizeof(raw));std::memcpy(&entry,raw,sizeof(entry));sScalingStatDistributionStore.rows[{ident}]=entry;}}\n'
for ident, row in values.items():
    raw = ','.join(str(v)+'u' for v in row)
    fixture += f'{{uint32 raw[]={{ {raw} }};ScalingStatValuesEntry entry;static_assert(sizeof(entry)==sizeof(raw));std::memcpy(&entry,raw,sizeof(entry));sScalingStatValuesStore.rows[{ident}]=entry;}}\n'
fixture += '\n'.join(initialization)+r'''
assert(items.size()==Area52Heirloom::Items().size());
for(auto item:items){assert(Area52Heirloom::Allowed(item.id));
for(unsigned level=15;level<=80;++level){player.level=level;Collector c;assert(Run(&item,&player,&c)==1);
auto d=sScalingStatDistributionStore.LookupEntry(item.ScalingStatDistribution);
auto v=sScalingStatValuesStore.LookupEntry(std::min(level,d->MaxLevel));
unsigned count=0;for(unsigned i=0;i<10;++i)if(d->StatMod[i]>=0){auto st=c.collected.ItemStat[count++];assert(st.ItemStatType==unsigned(d->StatMod[i]));assert(st.ItemStatValue==int(v->getssdMultiplier(item.ScalingStatValue)*d->Modifier[i]/10000));}
assert(c.collected.StatsCount==count);assert(c.stats[STATS_TYPE_SPELL_POWER]>=v->getSpellBonus(item.ScalingStatValue));
if(auto armor=v->getArmorMod(item.ScalingStatValue))assert(c.collected.Armor==armor);
if(auto dps=v->getDPSMod(item.ScalingStatValue)){assert(item.Delay);float actual=(c.collected.Damage[0].DamageMin+c.collected.Damage[0].DamageMax)*500/item.Delay;assert(actual>dps-0.01&&actual<dps+0.01);}
}}
Collector c;player.pilot=false;assert(Run(&items.front(),&player,&c)==1&&c.collected.ItemStat[0].ItemStatValue==99999);
player.pilot=true;auto invalid=items.front();invalid.ScalingStatDistribution=999999;assert(Run(&invalid,&player,&c)==0);
assert(!Area52Heirloom::Allowed(44090)&&!Area52Heirloom::Allowed(400000));
}
'''
with tempfile.TemporaryDirectory() as temporary:
    out=Path(temporary)
    (out/'test.cpp').write_text(fixture)
    subprocess.run(['clang++','-std=c++20','-I'+str(module/'Ai/Coa'),str(out/'test.cpp'),'-o',str(out/'test')],check=True)
    subprocess.run([str(out/'test')],check=True)
Path('/results/heirloom-coverage.json').write_text(json.dumps(coverage,indent=2))
print('PASS:',len(ids),'deployed heirlooms at each level15-80, actual DBC layouts/native scaling methods and production scoring block; missing data and nonpilot isolation. Equip legality remains native CanEquip validation, not gameplay.')
