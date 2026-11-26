from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def main():
    source=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text()
    start=source.index('uint32 Unit::SpellDamageBonusDone(')
    source=source[start:]
    start=source.index('else if (GetEntry() == 27893)')
    source=source[start:]
    start=source.index('return owner->SpellDamageBonusDone(');end=source.index(';',start)+1
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@FORWARD@',source[start:end])
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual Rune Weapon owner forwarding preserves effect index, modifier, stacks, damage/type/target and halves the owner result')
if __name__=='__main__':main()
