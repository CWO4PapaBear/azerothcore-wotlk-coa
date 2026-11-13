"""Stage an isolated mounted model and point only the dark mount display at it."""
from pathlib import Path
import sys,struct,json
sys.path.insert(0,'C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools')
from lib.mpq import MPQArchive
root=Path(__file__).resolve().parent/'rider-candidate'
data=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data')
def put(name,blob):
 p=root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(blob)
old=r'Creature\cadaverousmount\cadaverousmount'
new=r'Creature\Area52RootboundMount\Area52RootboundMount'
with MPQArchive(data/'patch-CC.MPQ') as a:
 b=bytearray(a.read_file(old+'.m2'))
 _,off=struct.unpack_from('<II',b,240)
 assert struct.unpack_from('<IH',b,off)==(0,0)
 bones,bo=struct.unpack_from('<II',b,44)
 seq,so=struct.unpack_from('<II',b,28)
 for movement in (4,5):
  i=next(i for i in range(seq) if struct.unpack_from('<H',b,so+i*64)[0]==movement)
  varying=[]
  bone=9
  while bone>=0:
   for track,fmt,width in [(16,'<3f',12),(36,'<4h',8)]:
    length,t=struct.unpack_from('<II',b,bo+bone*88+track+12)
    if i<length:
     n,keys=struct.unpack_from('<II',b,t+8*i)
     if len(set(struct.iter_unpack(fmt,b[keys:keys+n*width])))>1:varying.append((bone,track))
   bone=struct.unpack_from('<h',b,bo+bone*88+8)[0]
  assert varying
  print('movement',movement,'animated ancestors/tracks',varying)
 before=bytes(b);struct.pack_into('<H',b,off+4,9)
 assert sum(x!=y for x,y in zip(before,b))==1
 put(new+'.m2',b)
 for name in a.read_file('(listfile)').decode().splitlines():
  if name.lower().startswith(old.lower()) and name.lower().endswith(('.anim','.skin')):
   put(new+name[len(old):],a.read_file(name))
 texture=r'Creature\cadaverousmount\cadaverousmount_black.blp'
 put(new.rsplit('\\',1)[0]+'\\cadaverousmount_black.blp',a.read_file(texture))
with MPQArchive(data/'area-52/patch-E.MPQ') as a:
 for table in ['CreatureModelData','CreatureDisplayInfo']:
  path='DBFilesClient\\'+table+'.dbc';b=bytearray(a.read_file(path))
  magic,n,k,z,ss=struct.unpack_from('<4s4I',b);end=20+n*z
  ids=[struct.unpack_from('<I',b,20+i*z)[0] for i in range(n)]
  if table=='CreatureModelData':
   key=max(ids)+1
   assert key<2**31
   base=20+ids.index(112956)*z;row=bytearray(b[base:base+z])
   value=(new+'.m2').encode()+b'\0'
   struct.pack_into('<I',row,0,key);struct.pack_into('<I',row,8,ss)
   b=b[:end]+row+b[end:]+value
   struct.pack_into('<I',b,4,n+1);struct.pack_into('<I',b,16,ss+len(value))
  else:
   base=20+ids.index(142604)*z
   assert struct.unpack_from('<I',b,base+4)[0]==112956
   struct.pack_into('<I',b,base+4,key)
  put(path,b)
print('Staged display 142604, model',key,'and verified walk/run animation on saddle bone chain')
