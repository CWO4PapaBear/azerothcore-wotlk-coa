import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--dbc-dir', required=True)
args = parser.parse_args()
from pathlib import Path
import struct,subprocess,tempfile
root=Path(__file__).resolve().parents[3]
b=(Path(args.dbc_dir) / 'Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b)
ids={919159,914983,19306,1495,2973,2974,75,3044};records={}
for i in range(n):
    row=struct.unpack_from('<234I',b,20+i*z)
    if row[0] in ids:records[row[0]]=row
parent=records[919159]
assert parent[95]==107 and parent[110]==7 and parent[80]+1==3 and parent[49]==3
for id in [19306,1495,2973,2974]:
    target=records[id]
    assert parent[208]==target[208] and any(a&b for a,b in zip(parent[122:125],target[209:212])),str(id)
for id in [75,3044]:assert not any(a&b for a,b in zip(parent[122:125],records[id][209:212]))
vigor=records[914983];assert vigor[95]==35 and vigor[110]==3 and vigor[80]+1==20 and vigor[49]==1
source=(root/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text()
a=source.index('void AuraEffect::HandleAuraModIncreaseEnergy(');b=source.index('\nvoid AuraEffect::HandleAuraModIncreaseEnergyPercent',a)
code="""#include <cassert>
using uint8=unsigned char;using uint16=unsigned short;using Powers=int;using UnitMods=int;
constexpr int AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK=1,AURA_EFFECT_HANDLE_STAT=2,UNIT_MOD_POWER_START=10,TOTAL_VALUE=0;
struct Unit{float energy=100;void HandleStatFlatModifier(int mod,int,float value,bool apply){assert(mod==13);energy+=apply?value:-value;}};
struct AuraApplication{Unit* target;Unit* GetTarget()const{return target;}};
struct AuraEffect{int GetMiscValue()const{return 3;}int GetAmount()const{return 20;}void HandleAuraModIncreaseEnergy(AuraApplication const*,uint8,bool)const;};
"""+source[a:b]+"""
int main(){Unit p;AuraApplication app{&p};AuraEffect effect;effect.HandleAuraModIncreaseEnergy(&app,1,true);assert(p.energy==120);effect.HandleAuraModIncreaseEnergy(&app,1,false);assert(p.energy==100);effect.HandleAuraModIncreaseEnergy(&app,0,true);assert(p.energy==100);}
"""
with tempfile.TemporaryDirectory() as directory:
    path=Path(directory);(path/'check.cpp').write_text(code)
    subprocess.run(['g++','-std=c++20',str(path/'check.cpp'),'-o',str(path/'check')],check=True)
    subprocess.run([str(path/'check')],check=True)
print('Vigor native energy-cap apply/remove passed; Savage Strikes masks match four advertised roots and exclude Auto Shot/Arcane Shot. Full copy, replacement and restoration qualification remains pending.')
