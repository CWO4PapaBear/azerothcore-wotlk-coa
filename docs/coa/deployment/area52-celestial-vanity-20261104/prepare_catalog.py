from pathlib import Path
import struct,re,json
T=Path(__file__).resolve().parent;ARMOR={1,3,4,5,6,7,8,9,10,16,19,20};IDS=(33,140)
def rows(raw):
 magic,n,f,z,s=struct.unpack_from('<4s4I',raw);assert magic==b'WDBC' and len(raw)==20+n*z+s
 return [list(struct.unpack_from('<'+'I'*f,raw,20+i*z)) for i in range(n)],z,f
def read(name):return rows((T/'baseline/server'/name).read_bytes())[0]
item={r[0]:r[6] for r in read('Item.dbc')};apps={}
for r in read('ItemAppearances.dbc'):apps.setdefault(r[1],set()).add(r[2])
header=T/'baseline/source/src/server/coa/Area52ArchetypeVanityData.h';text=header.read_text();blocks={}
for name in ['Items','Bundles']:
 part=text.split('const '+name+' =')[1].split('\n};')[0]
 blocks[name]={int(k):list(map(int,re.findall(r'\d+',v))) for k,v in re.findall(r'\{(\d+), \{([^}]+)\}\}',part)}
armor={i:[v for v in blocks['Items'][9901000+i] if item[v] in ARMOR] for i in IDS}
app={i:set().union(*(apps.get(v,set()) for v in armor[i])) for i in IDS}
changed={}
for i,j in [(33,140),(140,33)]:
 for kind in ('Items','Bundles'):
  before=blocks[kind][9901000+i]
  after=sorted(([v for v in before if v not in armor[i]]+armor[j]) if kind=='Items' else ([v for v in before if v not in app[i]]+list(app[j])))
  old='{'+str(9901000+i)+', {'+', '.join(map(str,before))+'}}';new='{'+str(9901000+i)+', {'+', '.join(map(str,after))+'}}'
  assert text.count(old)==1;text=text.replace(old,new);changed[f'{kind}/{i}']={'before':before,'after':after}
out=T/'source/src/server/coa/Area52ArchetypeVanityData.h';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(text)
def swap(raw,name):
 rr,z,f=rows(raw);b=bytearray(raw)
 if name=='CharacterCreationArchetypeDetails':
  donors={(r[1],r[2],r[3]):r[:] for r in rr if r[1] in IDS}
  for idx,r in enumerate(rr):
   if r[1] in IDS:
    other=140 if r[1]==33 else 33;donor=donors[(other,r[2],r[3])];r[16:25]=donor[16:25]
    struct.pack_into('<9I',b,20+idx*z+64,*r[16:25])
 else:
  start,end=(18,35) if name=='ItemSet' else (18,42)
  for idx,r in enumerate(rr):
   i=r[0]-90000 if name=='ItemSet' else r[1]-9901000
   if i not in IDS:continue
   other=140 if i==33 else 33
   original=[v for v in r[start:end] if v];donor=[v for q in rr if (q[0]-90000 if name=='ItemSet' else q[1]-9901000)==other for v in q[start:end] if v and item.get(v) in ARMOR]
   values=[v for v in original if item.get(v) not in ARMOR]+donor;assert len(values)<=end-start
   struct.pack_into('<'+'I'*(end-start),b,20+idx*z+start*4,*(values+[0]*(end-start-len(values))))
 return bytes(b)
for name in ('CharacterCreationArchetypeDetails','ItemSet','VanityCollection'):
 out=T/'candidate/server'/(name+'.dbc');out.parent.mkdir(parents=True,exist_ok=True);out.write_bytes(swap((T/'baseline/server'/out.name).read_bytes(),name))
(T/'armor-swap.json').write_text(json.dumps(changed,indent=2));print('Prepared armor-only header and DBC swap; nonarmor grants retained')

