from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    mgr=(ROOT/'src/server/game/Spells/SpellMgr.cpp').read_text()
    eff=(ROOT/'src/server/game/Spells/SpellEffects.cpp').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text()
    code=code.replace('@PROC@',extract(mgr,'bool SpellMgr::CanSpellTriggerProcOnEvent('))
    code=code.replace('@RANK@',extract(eff,'void ModifyAscensionRankCooldown(')).replace('@CHAIN@',extract(eff,'void ModifyAscensionCooldown('))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual proc dispatcher and ranked cooldown mutation: Starsurge cast only, Earth/Moon hit masks, inheritance, Nature Reach school and Vengeance crit filtering; cooldown subtraction and exhaustion')
if __name__=='__main__':main()
