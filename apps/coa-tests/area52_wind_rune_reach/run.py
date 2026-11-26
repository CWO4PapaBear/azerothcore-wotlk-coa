from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def main():
    source=(ROOT/'src/server/scripts/Spells/spell_dk.cpp').read_text().split('class spell_dk_dancing_rune_weapon :',1)[1]
    start=source.index('            bool const area52 =')
    end=source.index('\n        }',start)
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@BODY@',source[start:end])
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual rune-weapon cast block restores Area 52 reach after direct and area spells; other realm behavior preserved')
if __name__=='__main__':main()
