from pathlib import Path
import subprocess, tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s, signature):
    a=s.index(signature);b=s.index('{',a)+1;depth=1
    while depth:
        depth+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    s=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text().replace('CRITICAL',extract(s,'uint32 Unit::SpellCriticalDamageBonus('))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual critical calculation: all Vengeance/Shadow Power ranks, strongest bonus, extra modifiers and realm isolation')
if __name__=='__main__':main()
