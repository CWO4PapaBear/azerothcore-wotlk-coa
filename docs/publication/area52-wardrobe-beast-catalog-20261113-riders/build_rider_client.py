from pathlib import Path
import sys,hashlib,json,struct,shutil
r=Path(__file__).resolve().parent
w=r.parent; old=w/"beast-catalog-20261113"
sys.path.insert(0,str(w))
from validate_white_walker_tooltips import archive_files
tools=Path("C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools")
files=archive_files(old/"pending-payload/client/patch-E.MPQ",tools)
from lib.mpq import MPQArchive,write_archive
data=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data')
folder=w/"rootbound-beast-review/rider-candidate"
for p in folder.rglob("*"):
 if p.is_file():files[str(p.relative_to(folder))]=p.read_bytes()
key=r'DBFilesClient\CreatureDisplayInfo.dbc'
original=archive_files(old/"pending-payload/client/patch-E.MPQ",tools)[key]
b=bytearray(original);_,n,k,z,_=struct.unpack_from("<4s4I",b)
models={};targets={142603,142604,142605}
for i in range(n):
 o=20+i*z;entry,model=struct.unpack_from("<II",b,o)
 if model==112956:
  assert entry in targets
  models[entry]=struct.unpack_from("<I",files[key],o+4)[0]
  struct.pack_into("<I",b,o+4,1999413)
assert set(models)==targets and models[142604]==1999413
files[key]=bytes(b)
with MPQArchive(data/'patch-CC.MPQ') as a:
 for color in ('cadaverousmount','cadaverousmount_white','cadaverousmount_black'):
  path='Creature\\cadaverousmount\\'+color+'.blp'
  texture=a.read_file(path);assert texture.startswith(b'BLP')
  files['Creature\\Area52RootboundMount\\'+color+'.blp']=texture
with MPQArchive(data/'patch-CG.MPQ') as a:
 m=a.read_file(r'Creature\grovecrawlermount\grovecrawlermount.m2')
 _,off=struct.unpack_from('<II',m,240);_,bone=struct.unpack_from('<IH',m,off)
 _,bo=struct.unpack_from('<II',m,44);sn,so=struct.unpack_from('<II',m,28)
 assert bone==123
 for movement in (4,5):
  idx=next(i for i in range(sn) if struct.unpack_from('<H',m,so+64*i)[0]==movement)
  bone=123;moving=False
  while bone>=0:
   for track,width in ((16,12),(36,8)):
    length,offset=struct.unpack_from('<II',m,bo+bone*88+track+12)
    if idx<length:
     count,pos=struct.unpack_from('<II',m,offset+8*idx)
     moving |= len({m[pos+j*width:pos+(j+1)*width] for j in range(count)})>1
   bone=struct.unpack_from('<h',m,bo+88*bone+8)[0]
  assert moving
out=r/'pending-payload/client/patch-E.MPQ';out.parent.mkdir(parents=True,exist_ok=True)
write_archive(out,files);assert archive_files(out,tools)==files
print('Three mount variants and skins verified; Grove Crawler already has animated walk and run seat ancestry.')
