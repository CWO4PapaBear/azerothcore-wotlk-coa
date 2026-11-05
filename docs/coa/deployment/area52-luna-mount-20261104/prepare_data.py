from pathlib import Path
import struct,json,sys,hashlib
T=Path(__file__).resolve().parent;W=T.parent;NAME="Luna, The Chronicler's Carapace";ARMOR={1,3,4,5,6,7,8,9,10,16,19,20}
def rows(raw):
 _,n,f,z,s=struct.unpack_from('<4s4I',raw);return [list(struct.unpack_from('<'+'I'*f,raw,20+i*z)) for i in range(n)],z
items={r[0]:r[6] for r in rows((T/'client/Item.dbc').read_bytes())[0]}
original=rows((W/'celestial-release-20261104/client/CharacterCreationArchetypeDetails.dbc').read_bytes())[0]
donors={(r[1],r[2],r[3]):r for r in original if r[1] in (33,140)}
def repair(raw):
 rr,z=rows(raw);out=bytearray(raw)
 for n,r in enumerate(rr):
  if r[1] not in (33,140):continue
  own=donors[(r[1],r[2],r[3])];other=donors[(140 if r[1]==33 else 33,r[2],r[3])]
  values=[v for v in own[16:28] if v and items[v] not in ARMOR]+[v for v in other[16:28] if v and items[v] in ARMOR]
  assert len(values)<=12
  struct.pack_into('<12I',out,20+n*z+64,*(values+[0]*(12-len(values))))
 return bytes(out)
def rename_spell(raw):
 magic,n,f,z,s=struct.unpack_from('<4s4I',raw);body=bytearray(raw[20:20+n*z]);pool=bytearray(raw[20+n*z:]);rr,_=rows(raw)
 index=next(i for i,r in enumerate(rr) if r[0]==1999401)
 for col,text in [(136,NAME),(170,'Summons and dismisses '+NAME+'. This is a ground mount.')]:
  struct.pack_into('<I',body,index*z+col*4,len(pool));pool.extend(text.encode()+b'\0')
 return struct.pack('<4s4I',magic,n,f,z,len(pool))+body+pool
for side in ('server','client'):
 out=T/'candidate'/side;out.mkdir(parents=True,exist_ok=True)
 root=T/'baseline/server' if side=='server' else T/'client'
 (out/'CharacterCreationArchetypeDetails.dbc').write_bytes(repair((root/'CharacterCreationArchetypeDetails.dbc').read_bytes()))
 if side=='server':(out/'Spell.dbc').write_bytes(rename_spell((root/'Spell.dbc').read_bytes()))
sys.path.insert(0,'C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools');from lib.mpq import MPQArchive,write_archive
D=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data/area-52/patch-E.MPQ');before=hashlib.sha256(D.read_bytes()).hexdigest()
with MPQArchive(D) as a:files={n:a.read_file(n) for n in a.read_file('(listfile)').decode().splitlines() if n and n.lower()!='(listfile)'}
(T/'client/Spell.dbc').write_bytes(files['DBFilesClient\\Spell.dbc'])
files['DBFilesClient\\Spell.dbc']=rename_spell(files['DBFilesClient\\Spell.dbc']);files['DBFilesClient\\CharacterCreationArchetypeDetails.dbc']=(T/'candidate/client/CharacterCreationArchetypeDetails.dbc').read_bytes()
(T/'candidate/client/Spell.dbc').write_bytes(files['DBFilesClient\\Spell.dbc'])
out=T/'client-preview/patch-E.MPQ';out.parent.mkdir(exist_ok=True);write_archive(out,files)
with MPQArchive(out) as a:
 for n,v in files.items():assert a.read_file(n)==v
assert hashlib.sha256(D.read_bytes()).hexdigest()==before
(T/'client-preview.json').write_text(json.dumps({'before_sha256':before,'after_sha256':hashlib.sha256(out.read_bytes()).hexdigest()},indent=2))
worker=T/'worker';(worker/'inputs').mkdir(parents=True,exist_ok=True)
(worker/'inputs/details').write_bytes(repair((T/'baseline/runtime/inputs/details').read_bytes()))
(worker/'inputs/archive').write_bytes(out.read_bytes());cfg=json.loads((T/'baseline/runtime/config.json').read_text())
for key in ('details','archive'):cfg[key]['sha256']=hashlib.sha256((worker/'inputs'/key).read_bytes()).hexdigest()
cfg['release']['archive_sha256']=cfg['archive']['sha256'];(worker/'config.json').write_text(json.dumps(cfg,indent=2))
print('Repaired all preview item lists by inventory type; renamed mount spell and preserved other assets')
