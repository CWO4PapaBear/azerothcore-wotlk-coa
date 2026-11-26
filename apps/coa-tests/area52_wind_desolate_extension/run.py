from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/coa/Area52Desolate.inc').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text()
    code=code.replace('@ENABLED@',extract(source,'bool Area52DesolateEnabled('))
    code=code.replace('@HIT@',extract(source,'void OnSpellHitResult(').replace(' override',''))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS extracted Desolate hit handler: rank chain, ownership, positive hit, once per cast, 2s extension, 12s remaining cap, no DoT creation or timer reset; native aura storage is separately qualified')
if __name__=='__main__':main()
