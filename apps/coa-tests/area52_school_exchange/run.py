from pathlib import Path
import subprocess,tempfile,struct
r=Path(__file__).resolve().parents[3]
s=(r/'src/server/coa/Area52SchoolExchange.inc').read_text().replace(' override','')
holy=(r/'src/server/coa/Area52HolyContracts.h').read_text();a=holy.index('inline bool DamagingSpell');b=holy.index('inline constexpr',a)
code=Path(__file__).with_name('harness.cpp').read_text().replace('@PRODUCTION@',s).replace('@DAMAGE@',holy[a:b])
blob=Path('/data/dbc/Spell.dbc').read_bytes();magic,n,f,size,strings=struct.unpack_from('<4s4I',blob);assert magic==b'WDBC' and f==234 and size==936
wanted={84579,84581,84583,84587,84589,286100,286101,286102,286103,286105,286106};records={}
for i in range(n):
 row=struct.unpack_from('<234I',blob,20+i*size)
 records[row[0]]=row
assert wanted.issubset(records)
for id in wanted:
 row=records[id]
 if id>200000:assert row[49]==4 and row[40]==32 and row[80]+row[74]==2
 else:assert row[49]==1
fixtures=[]
for id in [5143,7268,5740,42223,740,44203,755]:
 row=records[id];fixtures.append('{SpellInfo v;v.Id='+str(id)+';v.school='+str(row[225])+';')
 for j in range(3):fixtures.append('v.Effects['+str(j)+']={'+str(row[71+j])+','+str(row[116+j])+','+str(row[95+j])+'};v.auras.insert('+str(row[95+j])+');')
 fixtures.append('manager.records[v.Id]=v;}')
fixtures.append('p.owned={84579,84581,84583,84587,84589};')
for id,expected in [(5143,1),(5740,1),(740,0),(755,0)]:fixtures.append('{Spell channel;channel.school=manager.records['+str(id)+'].school;int before=p.casts;casts.OnSpellCast(&channel,&p,&manager.records['+str(id)+'],false);assert(p.casts==before+'+str(expected)+');}')
code=code.replace('@CHANNEL_RECORDS@',''.join(fixtures))
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS actual school-exchange cast callbacks: five parents, hybrid and multi-owner casts, wrappers, exclusions, dedup and readiness-aware shared-helper cleanup; native helper stack/duration metadata')

subprocess.run([__import__("sys").executable,str(Path(__file__).with_name("stacks.py"))],check=True)
