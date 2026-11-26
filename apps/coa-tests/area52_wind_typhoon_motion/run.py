from pathlib import Path
import subprocess,tempfile,struct,os
ROOT=Path(__file__).resolve().parents[3]
def extract(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
def main():
 data=Path(os.environ.get('COA_DBC_DIR','/data/dbc'),'Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',data)
 rows={struct.unpack_from('<I',data,20+i*z)[0]:struct.unpack_from('<'+str(f)+'I',data,20+i*z) for i in range(n)}
 r=rows[977726];assert r[71]==138 and r[208]==9
 code=Path(__file__).with_name('harness.cpp').read_text().replace('@EFFECT@',extract((ROOT/'src/server/game/Spells/SpellEffects.cpp').read_text(),'void Spell::EffectLeapBack(')).replace('@JUMP@',extract((ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text(),'void Unit::JumpTo(float'))
 code=code.replace('@FAMILY@',str(r[208])).replace('@MISC@',str(r[110])).replace('@DAMAGE@',str(r[80]+1))
 with tempfile.TemporaryDirectory() as tmp:
  cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
 print('PASS native effect 138 and JumpTo: self recoil backward at four headings, packet speeds, anticheat notification, correct execution phase')
if __name__=='__main__':main()
