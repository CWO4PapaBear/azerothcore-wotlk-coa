from pathlib import Path
import json,struct
p=Path(__file__).parent
changes=json.loads((p/'changes.json').read_text())
assert {x['id'] for x in changes}=={9452,26016,209452}
b=Path('/data/dbc/Spell.dbc').read_bytes()
_,n,f,z,_=struct.unpack_from('<4s4I',b)
assert f==234 and z==936
pool=b[20+n*z:]
rows={r[0]:r for i in range(n) if (r:=struct.unpack_from('<234I',b,20+i*z))[0] in {9452,26016,209452,200067}}
for x in changes:
 r=rows[x['id']];before=pool[r[170]:pool.find(b'\0',r[170])].decode()
 assert before==x['before']
 assert x['field']==170
 assert x['after'].count('$200067s1')==1 and x['after'].count('$200067d')==1
 assert '$200067s2' not in x['after']
 assert before.split('Capstone Bonus:')[0]==x['after'].split('Capstone Bonus:')[0]
 assert before.count('|cFFAAAAAA')==x['after'].count('|cFFAAAAAA')
 assert before.count('|r')==x['after'].count('|r')
r=rows[200067]
assert r[71:74]==(6,0,0) and r[95]==99 and r[40]==1
print('PASS3 guarded tooltip changes: dynamic helper amount/duration, AP-only, lower-rank gray text and existing descriptions preserved; client archive packaging remains pending')
