import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--dbc-dir', required=True)
args = parser.parse_args()
from pathlib import Path
import struct,subprocess,tempfile
root=Path(__file__).resolve().parents[3]
s=(root/"src/server/game/Entities/Player/Player.cpp").read_text()
a=s.index("void Player::ApplyTargetedSpellModTrackingDamage(");b=s.index("\ntemplate AC_GAME_API",a)
code=Path(__file__).with_name("harness.cpp.in").read_text().replace("@NATIVE@",s[a:b])
b=(Path(args.dbc_dir) / 'Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b)
data=['manager.records[1]={1,6,{0,0,0}};']
for i in range(n):
    r=struct.unpack_from('<234I',b,20+i*z)
    if r[0] in [586,34433,954522,15274,15311]:data.append('manager.records[%d]={%d,%d,{%d,%d,%d}};'%(r[0],r[0],r[208],*r[209:212]))
code=code.replace("@DATA@","\n".join(data))
with tempfile.TemporaryDirectory() as directory:
    p=Path(directory);(p/'test.cpp').write_text(code)
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
print('Entire production modifier arithmetic passed both cooldown ranks with explicit family-mask and engine adapters')
