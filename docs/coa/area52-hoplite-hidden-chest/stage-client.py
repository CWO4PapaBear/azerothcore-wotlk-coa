from pathlib import Path
import sys,struct,json,hashlib
sys.path.insert(0,'work/github-upload/tools')
from lib.mpq import MPQArchive,write_archive
p=Path('outputs/Area52_Hoplite_Chest');source=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data/area-52/patch-D.MPQ')
with MPQArchive(source) as a:files={n:a.read_file(n) for n in a.read_file('(listfile)').decode().splitlines() if n and n.lower() not in ('(listfile)','(attributes)','(signature)')}
changes=[]
for name,field,target,start,end,item in [('ItemSet',0,90073,18,35,168665),('VanityCollection',1,9901073,18,42,168665)]:
 key='DBFilesClient\\'+name+'.dbc';original=files[key];data=bytearray(original);_,count,f,z,ss=struct.unpack_from('<4s4I',data)
 matches=[]
 for i in range(count):
  pos=20+i*z;r=struct.unpack_from('<'+str(z//4)+'I',data,pos)
  if r[field]!=target:continue
  assert item not in r[start:end];slot=next(j for j in range(start,end) if r[j]==0);struct.pack_into('<I',data,pos+slot*4,item);matches.append(pos+slot*4)
 assert len(matches)==1
 expected=bytearray(original);struct.pack_into('<I',expected,matches[0],item);assert data==expected
 files[key]=bytes(data);(p/(name+'.candidate.dbc')).write_bytes(data);changes.append(dict(table=name,entry=target,offset=matches[0],item=item))
write_archive(str(p/'patch-D.MPQ'),files)
with MPQArchive(p/'patch-D.MPQ') as a:
 for n,b in files.items():assert a.read_file(n)==b,n
sha=lambda x:hashlib.sha256(x.read_bytes()).hexdigest()
(p/'client-stage.json').write_text(json.dumps(dict(status='staged_not_installed',source=str(source),before=sha(source),after=sha(p/'patch-D.MPQ'),changes=changes,appearance=12179,female_set_unchanged=True),indent=2))
print('Male set and shared bundle updated; female set unchanged; all archive members verified.')
