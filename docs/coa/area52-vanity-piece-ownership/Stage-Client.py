from pathlib import Path
import sys,struct,json,hashlib
sys.path.insert(0,'work/github-upload/tools');from lib.mpq import MPQArchive,write_archive
p=Path('outputs/Area52_Vanity_Piece_Ownership');client=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data/area-52/patch-D.MPQ')
with MPQArchive(client) as a:files={n:a.read_file(n) for n in a.read_file('(listfile)').decode().splitlines() if n and n.lower() not in ('(listfile)','(attributes)','(signature)')}
def decode(b):
 _,n,f,z,_=struct.unpack_from('<4s4I',b);return [list(struct.unpack_from('<'+'I'*(z//4),b,20+i*z)) for i in range(n)],b[20+n*z:],f,z
ir,_,_,_=decode(files['DBFilesClient\\Item.dbc']);items={r[0]:r for r in ir}
key='DBFilesClient\\VanityCollection.dbc';rr,pool,f,z=decode(files[key]);existing={r[1] for r in rr};oldrows=len(rr)
m=json.loads(Path('outputs/Area52_Archetype_Vanity/physical-vanity/bundle-manifest.json').read_text());equipment={x['reward_item_id']:x for e in m['bundles'] for x in e['equipment']};added=[];nextid=max(r[0] for r in rr)+1
for entry,e in sorted(equipment.items()):
 if entry in existing:continue
 item=items[entry];candidates=[r for r in rr[:oldrows] if r[1] in items and items[r[1]][1:3]==item[1:3] and (item[1]==2 or items[r[1]][6]==item[6] or {items[r[1]][6],item[6]}=={5,20} or {items[r[1]][6],item[6]} <= {13,21,22}) and r[18]==r[1] and not any(r[19:42]) and not r[76]]
 if not candidates and item[1]==4 and item[2] in (1,2,3,4):
  candidates=[r for r in rr[:oldrows] if r[1] in items and items[r[1]][1]==4 and items[r[1]][6]==item[6] and r[18]==r[1] and not any(r[19:42]) and not r[76]]
 if not candidates:raise ValueError(('No equivalent single-item template',entry,item))
 template=candidates[0];r=template[:];
 if item[1]==4 and item[2] in (1,2,3,4):r[2]=(r[2]&~(512|1024|2048|4096))|(1<<(8+item[2]))
 r[0]=nextid;nextid+=1;r[1]=entry;r[12:18]=[0]*6;r[18:42]=[entry]+[0]*23;r[42:76]=[0]*34;r[42]=len(pool);pool+=e['name'].encode()+b'\0';r[76]=0;rr.append(r);added.append({'item':entry,'template':template[1],'category':r[2]})
files[key]=struct.pack('<4s4I',b'WDBC',len(rr),f,z,len(pool))+b''.join(struct.pack('<'+'I'*(z//4),*r) for r in rr)+pool
(p/'VanityCollection.dbc').write_bytes(files[key]);write_archive(p/'patch-D.MPQ',files)
(p/'catalog.json').write_text(json.dumps({'added':added,'equipment':sorted(equipment),'bundles':{str(9901000+e['archetype_id']):sorted({x['reward_item_id'] for x in e['equipment']}) for e in m['bundles']}},indent=2))
(p/'stage.json').write_text(json.dumps({'before_sha256':hashlib.sha256(client.read_bytes()).hexdigest(),'after_sha256':hashlib.sha256((p/'patch-D.MPQ').read_bytes()).hexdigest(),'status':'staged_not_installed'},indent=2));print('Added',len(added),'individual Vanity records')
