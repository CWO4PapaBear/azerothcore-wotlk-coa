from pathlib import Path
import sys,struct,json,hashlib,shutil
T=Path(__file__).resolve().parent
sys.path.insert(0,'C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools')
from lib.mpq import MPQArchive,write_archive
def quiet(raw,client=False):
 magic,n,fields,size,strings=struct.unpack_from('<4s4I',raw)
 assert (magic,fields,size)==(b'WDBC',234,936)
 result=bytearray(raw);changed=[]
 for i in range(n):
  start=20+i*size;entry=struct.unpack_from('<I',raw,start)[0]
  if entry in (59906,59915):
   offset=start+131*4
   assert struct.unpack_from('<II',raw,offset)==((0,0) if client and entry==59906 else (7578,0))
   struct.pack_into('<I',result,offset,0);changed.append(entry)
 assert sorted(changed)==[59906,59915]
 return bytes(result)
archive=T/'preview/patch-E.MPQ'
with MPQArchive(archive) as a:files={n:a.read_file(n) for n in a.read_file('(listfile)').decode().splitlines() if n and n.lower()!='(listfile)'}
key='DBFilesClient\\Spell.dbc';(T/'baseline/client-spell.dbc').write_bytes(files[key]);files[key]=quiet(files[key],client=True)
write_archive(archive,files)
with MPQArchive(archive) as a:
 for n,v in files.items():assert a.read_file(n)==v
dest=T/'candidate/server/Spell.dbc';dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(quiet((T/'baseline/server/Spell.dbc').read_bytes()))
hashfile=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
h=json.loads((T/'preview/hashes.json').read_text())
(T/'client-preview.json').write_text(json.dumps({'before_sha256':h['source']}))
worker=T/'worker';(worker/'inputs').mkdir(parents=True,exist_ok=True)
cfg=json.loads((T/'baseline/runtime/config.json').read_text())
for name,path in [('details',T/'preview/worker-details'),('archive',archive)]:
 out=worker/'inputs'/name;shutil.copyfile(path,out);cfg[name]['sha256']=hashfile(out)
cfg['release']['archive_sha256']=cfg['archive']['sha256'];(worker/'config.json').write_text(json.dumps(cfg,indent=2))

