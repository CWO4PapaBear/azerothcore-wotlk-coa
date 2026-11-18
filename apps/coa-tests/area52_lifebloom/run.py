import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--dbc-dir', required=True)
args = parser.parse_args()
from pathlib import Path
import subprocess,tempfile,struct
root=Path(__file__).resolve().parents[3]
def function(source,signature):
    start=source.index(signature);end=source.index("{",start)+1;depth=1
    while depth:
        depth+=(source[end]=="{")-(source[end]=="}");end+=1
    return source[start:end]
source=(root/"src/server/scripts/Spells/spell_druid.cpp").read_text()
code=Path(__file__).with_name("harness.cpp.in").read_text().replace("@CLASS@",function(source,"class spell_dru_lifebloom"))
with tempfile.TemporaryDirectory() as directory:
    path=Path(directory);(path/"check.cpp").write_text(code)
    subprocess.run(["g++","-std=c++20","-Wall","-Wextra",str(path/"check.cpp"),"-o",str(path/"check")],check=True)
    subprocess.run([str(path/"check")],check=True)
b=(Path(args.dbc_dir) / 'Spell.dbc').read_bytes()
_,count,fields,size,_=struct.unpack_from('<4s4I',b)
for index in range(count):
    row=struct.unpack_from('<234I',b,20+index*size)
    if row[0] in [33763,48450,48451]:
        assert row[95:97]==(8,4) and row[98]==1000 and row[49]==3 and row[204]==24 and row[40]==165
print('Native bloom callbacks: all three rank amounts, one-to-three stacks, expiration, partial dispel, non-bloom removals and caster attribution passed; healing engine uses explicit coefficient adapter')
