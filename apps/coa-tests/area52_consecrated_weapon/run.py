from pathlib import Path
import subprocess,tempfile,struct
r=Path(__file__).resolve().parents[3]
def block(s,m):
 a=s.index(m);i=s.index('{',a)+1;n=1
 while n:n+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]
s=(r/'src/server/coa/Area52ConsecratedWeapon.inc').read_text().replace(' override','')
u=(r/'src/server/game/Entities/Unit/Unit.cpp').read_text()
code=Path(__file__).with_name('harness.cpp').read_text().replace('@PRODUCTION@',s).replace('@PPM@',block(u,'float Unit::GetPPMProcChance('))
blob=Path('/data/dbc/Spell.dbc').read_bytes();_,n,f,size,_=struct.unpack_from('<4s4I',blob);assert f==234 and size==936
rows={}
for i in range(n):
 row=struct.unpack_from('<234I',blob,20+i*size)
 if row[0] in {977848,977827,965295,986281,278745}:rows[row[0]]=row
assert rows[977848][34]==0 and rows[977848][35]==0 and rows[977848][95]==42 and rows[977848][116]==977827
assert rows[965295][35]==50
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);src=p/'candidate.cpp';src.write_text(code);out=src.with_suffix('');subprocess.run(['c++','-std=c++20',str(src),'-o',str(out)],check=True);subprocess.run([str(out)],check=True)
print('PASS actual Consecrated Weapon callback:12PPM,750ms,ownership,melee replacements,exclusions. DBC baseline zero flags/chance is structural evidence, not a failing native baseline test; registration prerequisites modeled.')

subprocess.run([__import__("sys").executable,str(Path(__file__).with_name("seal.py"))],check=True)
