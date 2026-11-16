from pathlib import Path
import re,struct
root=Path(__file__).resolve().parents[3]
s=(root/'src/server/coa/Area52ArchetypeVanityData.h').read_text()
m=re.findall(r'\{9901073, \{([^}]+)\}\}',s)
assert len(m)==2
assert 12179 in [int(v) for v in m[0].split(',')]
assert 168665 in [int(v) for v in m[1].split(',')]
assert all(len([int(v) for v in row.split(',')])==len(set(int(v) for v in row.split(','))) for row in m)
p=Path('/block-results')
for name,field,entry,start,end in [('ItemSet',0,90073,18,35),('VanityCollection',1,9901073,18,42)]:
 old=(p/(name+'.dbc')).read_bytes();new=(p/(name+'.candidate.dbc')).read_bytes()
 _,n,f,z,ss=struct.unpack_from('<4s4I',old);expected=bytearray(old);found=0
 for i in range(n):
  pos=20+i*z;r=struct.unpack_from('<'+str(z//4)+'I',old,pos)
  if r[field]==entry:
   assert 168665 not in r[start:end]
   slot=next(j for j in range(start,end) if r[j]==0)
   struct.pack_into('<I',expected,pos+slot*4,168665);found+=1
 assert found==1 and bytes(expected)==new
print('PASS: Hidden Chest reward and appearance; male set only; other client bytes unchanged.')
