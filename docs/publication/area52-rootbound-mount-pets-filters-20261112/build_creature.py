from pathlib import Path
import json,struct,sys
sys.path.insert(0,'C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools')
from lib.mpq import MPQArchive
r=Path(__file__).resolve().parent
d=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data')
with MPQArchive(d/'patch-M.MPQ') as a:b=a.read_file('DBFilesClient\\Creature.dbc')
magic,n,k,z,s=struct.unpack_from('<4s4I',b);assert (magic,k,z)==(b'WDBC',23,92)
end=20+n*z;rows=[b[i:i+z] for i in range(20,end,z)]
get=lambda x:struct.unpack_from('<I',x)[0]
assert not any(get(row)==1999412 for row in rows)
row=bytearray(next(v for v in rows if get(v)==79010))
assert struct.unpack_from('<I',row,4)[0]==struct.unpack_from('<I',row,76)[0]
label=b'Dark Rootbound Beast\0'
for col,val in [(0,1999412),(1,1999412),(2,s),(19,1999412)]:struct.pack_into('<I',row,col*4,val)
i=next((j for j,v in enumerate(rows) if get(v)>1999412),n)
rows.insert(i,row);out=struct.pack('<4s4I',magic,n+1,k,z,s+len(label))+b''.join(rows)+b[end:]+label
assert out[20:20+i*z]+out[20+(i+1)*z:-len(label)]==b[20:]
(r/'candidate/Creature.dbc').write_bytes(out)
print('Added missing creature preview record; preserved every existing record and string')
