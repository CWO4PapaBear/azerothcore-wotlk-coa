from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def block(s,signature):
 a=s.index(signature);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
def main():
 source=(ROOT/'src/server/game/Entities/Player/Player.cpp').read_text().split('void Player::AddSpellAndCategoryCooldowns(',1)[1]
 policy=source[source.index('    bool const area52SharedDefensive'):source.index('    time_t catrecTime;')]
 dispatch=block(source,'    if (cat && catrec > 0)')
 b=Path('/data/dbc/Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b);rows={struct.unpack_from('<I',b,20+i*z)[0]:struct.unpack_from('<234I',b,20+i*z) for i in range(n)}
 records=[]
 for id in [19263,31224,642,1022,45438,645438]:
  r=rows[id];records.append('{'+','.join(map(str,[id,r[208],r[1],r[29],r[30]]))+'}')
 code=Path(__file__).with_name('harness.cpp').read_text().replace('@POLICY@',policy).replace('@DISPATCH@',dispatch).replace('@RECORDS@',','.join(records))
 with tempfile.TemporaryDirectory() as tmp:
  cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20','-I',str(ROOT/'src/server/coa'),str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
  old=subprocess.check_output(['git','show','HEAD:src/server/game/Entities/Player/Player.cpp'],cwd=ROOT,text=True).split('void Player::AddSpellAndCategoryCooldowns(',1)[1]
  baseline=code.replace(policy,'bool const area52SharedDefensive=false;').replace(dispatch,block(old,'    if (cat && catrec > 0)'))
  cpp.write_text(baseline);subprocess.run(['c++','-std=c++20','-I',str(ROOT/'src/server/coa'),str(cpp),'-o',str(exe)],check=True)
  result=subprocess.run([str(exe)],capture_output=True);assert result.returncode!=0
  print('PASS unchanged baseline fails the same cross-family cooldown scenario')
 print('PASS native shared-category policy/dispatch: all five defenses cross-family, 30 seconds, own recovery retained, zero-category Ice Block, NPC variant exclusion and other-realm isolation')
if __name__=='__main__':main()
