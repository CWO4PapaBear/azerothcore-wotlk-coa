from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    mgr=(ROOT/'src/server/game/Spells/SpellMgr.cpp').read_text()
    method=extract(mgr,'bool SpellMgr::CanSpellTriggerProcOnEvent(')
    flags=method[method.index('    if (!(eventInfo.GetTypeMask() & procEntry.ProcFlags))'):method.index('    // check XP')]
    phase=method[method.index('    // check spell type mask'):method.rindex('}')]
    aura=extract((ROOT/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text(),'uint8 Aura::GetProcEffectMask(')
    ownership=aura[aura.index('    bool const area52Clearcasting'):aura.index('    // do checks using conditions table')]
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@FLAGS@',flags).replace('@PHASE@',phase).replace('@OWNERSHIP@',ownership)
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS native proc flag/type/phase/hit admission and applied-aura ownership: old melee flags fail, corrected CAST accepts once, HIT/heal/unapplied modifiers reject')
if __name__=='__main__':main()
