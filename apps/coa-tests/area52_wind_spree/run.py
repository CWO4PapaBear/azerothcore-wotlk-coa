from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def main():
    source=(ROOT/'src/server/game/Spells/SpellEffects.cpp').read_text()
    start=source.index('                bool const area52KillingSpree =')
    end=source.index('                // Fan of Knives, Hemorrhage, Ghostly Strike',start)
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@BLOCK@',source[start:end])
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual Killing Spree weapon modifier block: 130% ordinary, 195% dagger, both hands, Titan Grip 25% reduction and other-mode isolation')
if __name__=='__main__':main()
