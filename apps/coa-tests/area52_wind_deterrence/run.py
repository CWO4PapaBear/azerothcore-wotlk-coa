from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/scripts/Spells/spell_hunter.cpp').read_text().split('class spell_hun_area52_deterrence :',1)[1]
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@APPLY@',extract(source,'void HandleApply('))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual Deterrence apply handler removes stuns, preserves unrelated mechanics and other realms, tolerates missing target')
if __name__=='__main__':main()
