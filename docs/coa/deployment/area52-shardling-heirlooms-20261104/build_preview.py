from pathlib import Path
import hashlib,json,struct,sys
from collections import Counter
T=Path(__file__).resolve().parent
O=T/'preview';O.mkdir(exist_ok=True)
C=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live')
R=Path('C:/Users/danie/Documents/Codex/area52-featured-catalog-runtime')
sys.path.insert(0,'C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools')
from lib.mpq import MPQArchive,write_archive
def parts(raw,fields):
 magic,n,f,z,s=struct.unpack_from('<4s4I',raw)
 assert (magic,f,z)==(b'WDBC',fields,fields*4) and len(raw)==20+n*z+s
 return list(struct.iter_unpack('<'+'I'*fields,raw[20:20+n*z])),raw[20+n*z:]
def details(raw):
 rows,_=parts(raw,28);out=bytearray(raw);hits=0
 for i,row in enumerate(rows):
  if row[1]==13:
   assert row[13]==0
   struct.pack_into('<I',out,20+i*112+52,11783);hits+=1
 assert hits==20
 return bytes(out)
def placements(raw):
 rows,pool=parts(raw,12);assert all(r[1]!=11783 for r in rows)
 donor=[r for r in rows if r[1]==510];assert len({r[2] for r in donor})==len(donor)==10
 start=max(r[0] for r in rows)+1
 extra=[(start+i,11783,*r[2:]) for i,r in enumerate(donor)]
 return struct.pack('<4s4I',b'WDBC',len(rows)+10,12,48,len(pool))+b''.join(struct.pack('<12I',*r) for r in rows+extra)+pool
keys=['DBFilesClient\\CharacterCreationArchetypeDetails.dbc','DBFilesClient\\CharacterCreationPetDetails.dbc']
base={}
for p in sorted((C/'Data').glob('*.MPQ'))+sorted((C/'Data/area-52').glob('*.MPQ')):
 with MPQArchive(p) as a:
  for k in keys:
   if a.has_file(k):base[k]=a.read_file(k)
src=C/'Data/area-52/patch-E.MPQ'
with MPQArchive(src) as a:files={k:a.read_file(k) for k in a.read_file('(listfile)').decode().splitlines() if k and k.lower()!='(listfile)'}
for k,func in zip(keys,[details,placements]):
 (O/(k.split('\\')[-1]+'.before')).write_bytes(base[k]);files[k]=func(base[k])
write_archive(O/'patch-E.MPQ',files)
with MPQArchive(O/'patch-E.MPQ') as a:
 for k,v in files.items():assert a.read_file(k)==v
worker=(R/'inputs/details').read_bytes();(O/'worker-details').write_bytes(details(worker))
(O/'hashes.json').write_text(json.dumps({'source':hashlib.sha256(src.read_bytes()).hexdigest(),'candidate':hashlib.sha256((O/'patch-E.MPQ').read_bytes()).hexdigest(),'worker_before':hashlib.sha256(worker).hexdigest(),'archetype':13,'creature':11783,'placement_donor':510},indent=2))
print('Staged 20 Cryotherapist preview variants and 10 race-specific placements in the client overlay and staged the worker default')
