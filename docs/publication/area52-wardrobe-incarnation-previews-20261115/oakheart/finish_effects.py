"""Complete the single-texture particle and emissive passes for the pinned Nightmare Ancient."""
from pathlib import Path
import struct as s
root=Path(__file__).resolve().parent
src=(root/'vsnightmareancient.m2').read_bytes()[8:]
path=root/'candidate/creature/vsancient/vsnightmareancient.m2'
model=bytearray(path.read_bytes())
def pair(b,o):return s.unpack_from('<II',b,o)
count,offset=pair(src,296)
assert count==3 and pair(model,296)==(0,0)
emitters=bytearray()
for i in range(count):
 start=offset+492*i
 emitter=bytearray(src[start:start+476])
 flags=s.unpack_from('<I',emitter,4)[0]
 assert flags==0x60830229 and not any(src[start+476:start+492])
 assert s.unpack_from('<H',emitter,22)[0]==5
 # Only the vertical packed-gravity case has an exact scalar equivalent.
 n,outer=pair(emitter,144)
 for j in range(n):
  keys,inner=pair(src,outer+8*j)
  for k in range(keys):
   x,y,z=s.unpack_from('<bbh',src,inner+4*k)
   assert x==y==0
   s.pack_into('<f',model,inner+4*k,-z*0.04238648)
 s.pack_into('<I',emitter,4,flags & 0x7fffff)
 emitters.extend(emitter)
s.pack_into('<II',model,296,count,len(model))
model.extend(emitters)
path.write_bytes(model)
for view in range(2):
 skin=path.with_name(path.stem+f'{view:02}.skin')
 b=bytearray(skin.read_bytes())
 count,offset=pair(b,36)
 batches=[]
 for i in range(count):
  batch=list(s.unpack_from('<12H',b,offset+24*i))
  batches.append(tuple(batch))
  if batch[6]!=1:continue
  # The upstream split cloned the diffuse index into its second pass.
  # Keep its combined diffuse/reflection pass and bind the original emissive layer here.
  batch[5]+=1
  batch[8]+=2
  batches[-1]=tuple(batch)
 for i,batch in enumerate(batches):s.pack_into('<12H',b,offset+i*24,*batch)
 skin.write_bytes(b)
