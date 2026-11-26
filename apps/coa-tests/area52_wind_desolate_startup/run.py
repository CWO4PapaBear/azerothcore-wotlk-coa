from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/coa/Area52Desolate.inc').read_text()
    shaman=(ROOT/'src/server/scripts/Spells/spell_shaman.cpp').read_text()
    functions=[extract(source,x) for x in ('bool Area52DesolateEnabled(', 'void ConfigureArea52Desolate(', 'void RegisterArea52WindfuryReadiness(', 'void ApplyArea52DesolateWindfury(', 'bool HasArea52DesolateHandlers(')]
    functions.insert(0,extract(shaman,'bool HasArea52WindfuryHandler('))
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@FUNCTIONS@','\n'.join(functions))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS Desolate readiness rejects missing config, registry, native binding and charged proc; grant emits one modifier and one Dust Devil, honors Elemental Destruction and other realms')
if __name__=='__main__':main()
