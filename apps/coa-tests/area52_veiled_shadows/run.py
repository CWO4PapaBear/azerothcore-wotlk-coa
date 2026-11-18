import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--dbc-dir', required=True)
args = parser.parse_args()
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[3]
def function(source, signature):
    start = source.index(signature)
    end = source.index("{", start) + 1
    depth = 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]
source = (root / "src/server/coa/AscensionFreePick.cpp").read_text()
functions = "\n".join(function(source, name) for name in
    ["float VeiledShadowsProcChance(", "bool SynchronizeTalentRanks("])
callback = (root / "src/server/coa/AscensionVeiledShadows.cpp").read_text()
callback = "\n".join(line for line in callback.splitlines() if not line.startswith("#include"))
fixture = Path(__file__).with_name("harness.cpp.in").read_text().replace("@FUNCTIONS@", functions).replace("@CALLBACK@", callback)
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)
    (path / "test.cpp").write_text(fixture)
    subprocess.run(["g++", "-std=c++20", "-Wall", "-Wextra", str(path / "test.cpp"), "-o", str(path / "test")], check=True)
    subprocess.run([str(path / "test")], check=True)
print("Production Fade callback and talent synchronization: forced chance outcomes, both ranks, combo 0-5 fixed2s, saved-rank/load order, stale rank, independent approval and realm isolation passed")

import struct
blob=(Path(args.dbc_dir) / 'Spell.dbc').read_bytes()
magic,count,fields,size,strings=struct.unpack_from('<4s4I',blob)
assert magic==b'WDBC' and fields==234 and size==936
wanted={15274,15311,954696,586,34433,954522}
records={}
for index in range(count):
    row=struct.unpack_from('<234I',blob,20+index*size)
    if row[0] in wanted:records[row[0]]=row
assert set(records)==wanted
signed=lambda value:value if value<2147483648 else value-4294967296
for talent,fade,other,chance in [(15274,-1000,-30000,50),(15311,-2000,-60000,100)]:
    row=records[talent]
    assert row[35]==chance
    assert row[95:97]==(107,107) and row[110:112]==(11,11)
    assert tuple(signed(v)+1 for v in row[80:82])==(fade,other)
    for spell,slot,base,expected in [(586,0,30000,30000+fade),(34433,1,150000,150000+other),(954522,1,180000,180000+other)]:
        target=records[spell]
        assert row[208]==target[208]
        assert any(a&b for a,b in zip(row[122+3*slot:125+3*slot],target[209:212]))
        assert max(target[29],target[30])==base
        assert base+signed(row[80+slot])+1==expected
helper=records[954696]
assert helper[95:97]==(77,77) and set(helper[110:112])=={7,11}
assert helper[86:88]==(21,21)
print('Current DBC: both cooldown masks/rank values and native root/snare immunity effects validated; cooldown arithmetic is a data check, not a full cast test')
