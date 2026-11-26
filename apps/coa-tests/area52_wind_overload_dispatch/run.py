from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/scripts/Spells/spell_shaman.cpp').read_text().split('class spell_sha_lightning_overload',1)[1]
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@HANDLE@',extract(source,'void HandleProc('))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS native Overload dispatch copies one half pre-target damage for the matching target, all critical states, LB/CL and replacements; missing/zero events and other realms retain isolation')
if __name__=='__main__':main()
