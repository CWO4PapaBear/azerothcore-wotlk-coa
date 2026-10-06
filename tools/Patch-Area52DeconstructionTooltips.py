import argparse
from pathlib import Path
import struct

ids={954081,954082,954083,*range(954443,954451)}
def patch(b):
 n,f,z,length=struct.unpack_from('<4I',b,4);assert f==234
 data=bytearray(b[:20+n*z]);strings=bytearray(b[20+n*z:]);changed=[]
 for i in range(n):
  offset=20+i*z;id=struct.unpack_from('<I',data,offset)[0]
  if id not in ids:continue
  for field in (170,187):
   index=struct.unpack_from('<I',data,offset+field*4)[0]
   text=strings[index:strings.find(b'\0',index)].decode()
   new=text.replace('for 1 second per stack','for 3 seconds per stack')
   if field==170 and id==954081:new=text+'\nDeconstruction lasts 3 seconds per stack of Sunder Armor, up to 15 seconds.'
   if id==954083 and text:new=text+'\nLasts 3 seconds per stack of Sunder Armor, up to 15 seconds.'
   if new!=text:
    struct.pack_into('<I',data,offset+field*4,len(strings));strings.extend(new.encode()+b'\0');changed.append([id,field])
 struct.pack_into('<I',data,16,len(strings))
 assert {x[0] for x in changed}==ids
 return bytes(data+strings),changed

parser = argparse.ArgumentParser()
parser.add_argument('input', type=Path)
parser.add_argument('output', type=Path)
args = parser.parse_args()
assert args.input.resolve() != args.output.resolve(), 'Use a separate output file'
updated, changed = patch(args.input.read_bytes())
args.output.write_bytes(updated)
print('Updated Deconstruction text fields:', changed)
