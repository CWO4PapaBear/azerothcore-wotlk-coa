from pathlib import Path
import subprocess,tempfile,json,os

task=Path(os.environ['AREA52_BOT_TASK'])
repo=Path(__file__).resolve().parents[3]
source=(repo/'src/server/coa/AscensionCompat.cpp').read_text()
start=source.index('        auto const setInfo = _appearances.find(set);',source.index('    bool ApplyPilotVanity'))
end=source.index('        desired[55] = set;',start)
body=source[start:end]
data=json.loads((task/'equipment-inputs.json').read_text())
assert data['missing_hoplite_armor_appearances']==[]
stub='''#include <array>
#include <unordered_map>
#include <cassert>
#include <cstdint>
using uint32=std::uint32_t;
struct Appearance{uint32 SourceItem=0;};
struct Player{unsigned race,gender;unsigned getRace(){return race;}unsigned getGender(){return gender;}};
std::unordered_map<uint32,Appearance> _appearances;
std::unordered_map<uint32,uint32> _itemAppearances;
std::unordered_map<uint32,std::array<uint32,9>> _botArchetypeArmor;
void apply(Player* player,uint32 set,std::array<uint32,64>&desired){
'''
test='}\nint main(){_appearances[900573]={90073};\n'
for item,appearance in data['item_appearances'].items():
 if appearance:test+=f'_itemAppearances[{item}]={appearance};_appearances[{appearance}]={{}};\n'
for row in data['hoplite']:
 key=(row[1]<<16)|(row[2]<<8)|row[3]
 test+=f'_botArchetypeArmor[{key}]={{{",".join(map(str,row[16:25]))}}};\n'
test+='constexpr std::array<unsigned,9> cats{1,2,4,8,11,7,9,3,10};\n'
for row in data['hoplite']:
 race,gender=row[2],row[3]
 alternate=next(x for x in data['hoplite'] if x[2]==race and x[3]!=(gender))
 appearances=[data['item_appearances'][str(i)] for i in alternate[16:25]]
 target=[data['item_appearances'][str(i)] for i in row[16:25]]
 test+=f'{{Player p{{{race},{gender}}};std::array<uint32,64> desired{{}};std::array<uint32,9> old{{{",".join(map(str,appearances))}}},expected{{{",".join(map(str,target))}}};'
 test+='for(unsigned i=0;i<9;++i)desired[cats[i]]=old[i];desired[12]=123;desired[13]=456;desired[14]=789;apply(&p,900573,desired);for(unsigned i=0;i<9;++i)assert(desired[cats[i]]==expected[i]);assert(desired[12]==123&&desired[13]==456&&desired[14]==789);desired[cats[0]]=999999;desired[cats[1]]=0;apply(&p,900573,desired);assert(desired[cats[0]]==999999&&desired[cats[1]]==0);}\n'
test+='}\n'
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(stub+body+test)
 subprocess.run(['g++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS actual bot gender resolver for all 20 Hoplite race/gender rows; explicit custom armor, empty slots and all weapons preserved')
