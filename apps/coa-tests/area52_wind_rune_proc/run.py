from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
def main():
 source=(ROOT/'src/server/scripts/Spells/spell_dk.cpp').read_text().split('class spell_dk_dancing_rune_weapon :',1)[1]
 code=Path(__file__).with_name('harness.cpp').read_text().replace('@CHECK@',extract(source,'bool CheckProc('))
 start=source.index('        if (AscensionFreePick::Applies(player->ToPlayer()) && eventInfo.GetSpellInfo()')
 code=code.replace('@MARK@',extract(source[start:],'if (AscensionFreePick::Applies'))
 with tempfile.TemporaryDirectory() as tmp:
  cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
  old=subprocess.check_output(['git','show','HEAD:src/server/scripts/Spells/spell_dk.cpp'],cwd=ROOT,text=True).split('class spell_dk_dancing_rune_weapon :',1)[1]
  baseline=code.replace(extract(source,'bool CheckProc('),extract(old,'bool CheckProc('))
  cpp.write_text(baseline);subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
  result=subprocess.run([str(exe)],capture_output=True);assert result.returncode!=0
  print('PASS unchanged baseline rejects the unrelated-family attack scenario')
 print('PASS native rune weapon proc eligibility: class-family isolation, known spells, positive/summon rejection, AoE once per source cast, repeat cast, other realms')
if __name__=='__main__':main()
