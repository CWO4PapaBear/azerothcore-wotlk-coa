from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
def main():
 source=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text()
 a=source.index('    // cast by caster in front of victim');b=source.index('    return SPELL_MISS_NONE;',a)
 code=Path(__file__).with_name('harness.cpp').read_text().replace('@MISS@',extract(source,'float Unit::GetUnitMissChance(')).replace('@DEFLECT@',source[a:b])
 b=Path('/data/dbc/Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b);r={struct.unpack_from('<I',b,20+i*z)[0]:struct.unpack_from('<234I',b,20+i*z) for i in range(n)}
 main=r[19263];helper=r[67801];assert main[71:74]==(6,6,6);assert main[95:98]==(184,287,25)
 assert main[80]+1==4294967096 and main[81]+1==100
 assert helper[95:98]==(288,4,27) and helper[71:74]==(6,6,6)
 with tempfile.TemporaryDirectory() as tmp:
  cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
 print('PASS native melee miss and magic deflection calculation, behind/front directions, stun gate; real pacify/silence/helper metadata')
if __name__=='__main__':main()
