from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/coa/Area52WindRager.inc').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text()
    for tag,sig in (('@KIND@','uint32 Area52WindStrikeKind('),('@CONFIG@','void ConfigureArea52WindRager('),('@RISING@','void AddArea52RisingWinds('),('@READY@','bool HasArea52WindRagerHandlers('),('@CHECK@','bool CheckProc('),('@PROC@','void HandleProc('),('@CAST@','void OnSpellBeforeEffects('),('@DAMAGE@','void OnSpellCalculatedTarget('),('@HIT@','void OnSpellHitResult('),('@AMOUNT@','void OnAfterAuraEffectCalculateAmount('),('@REMOVE@','void OnAuraRemove(')):
        code=code.replace(tag,extract(source,sig).replace(' override',''))
    b=Path('/data/dbc/Spell.dbc').read_bytes();_,n,_,size,_=struct.unpack_from('<4s4I',b)
    wanted={978939,983986,983974,978232};rows={}
    for i in range(n):
        row=struct.unpack_from('<234I',b,20+i*size)
        if row[0] in wanted:rows[row[0]]=row
    assert rows[978939][98]==500 and rows[978939][116]==878939
    assert rows[983974][80]==19 and rows[983974][110]==18
    assert rows[978232][82]==0xfffffff7 and rows[978232][112]==23
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20','-I'+str(ROOT/'src/server/coa'),str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual Wind Rager callbacks: cold readiness, native bindings/proc dependency, ten-stack rollover, all26 strike helpers, crit cooldowns, PvE/PvP, Tornado multiplier, Calm consumption, Doom/Titan Grip amount and cleanup')
if __name__=='__main__':main()
