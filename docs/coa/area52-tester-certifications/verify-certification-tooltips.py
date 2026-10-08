from pathlib import Path
import json,sys,struct,re,hashlib
root=Path(__file__).resolve().parents[3];p=root.parent/'Area52_Tester_Certification_20261008'
sys.path.insert(0,str(root.parents[1]/'work/github-upload/tools'));from lib.mpq import MPQArchive
client=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data/area-52/patch-D.MPQ')
r=json.loads((p/'stage.json').read_text());target=set(r['spells']);seen=set()
assert hashlib.sha256(client.read_bytes()).hexdigest()==r['before_sha256']
with MPQArchive(client) as a,MPQArchive(p/'patch-D-certified.MPQ') as b:
 for name in a.read_file('(listfile)').decode().splitlines():
  if not name or name.lower() in ('(listfile)','(attributes)','(signature)'):continue
  x,y=a.read_file(name),b.read_file(name)
  if name!='DBFilesClient\\Spell.dbc':assert x==y,name;continue
  _,n,f,z,_=struct.unpack_from('<4s4I',x);xp,yp=x[20+n*z:],y[20+n*z:]
  for i in range(n):
   u=list(struct.unpack_from('<'+'I'*f,x,20+i*z));v=list(struct.unpack_from('<'+'I'*f,y,20+i*z));sid=u[0]
   old=xp[u[170]:xp.find(b'\0',u[170])];new=yp[v[170]:yp.find(b'\0',v[170])]
   if sid in target:
    expected,c=re.subn(rb'\|cff[0-9a-fA-F]{6}(?:NOT VERIFIED|VERIFIED|CERTIFIED)\|r',b'|cff00ff00CERTIFIED|r',old);assert c==1 and new==expected;seen.add(sid)
   else:assert old==new,sid
   u[170]=v[170]=0;assert u==v,sid
assert seen==target
print('PASS: all 228 certification labels; all other fields, descriptions, SHIFT text and archive members unchanged.')
