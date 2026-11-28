from pathlib import Path
import subprocess
import tempfile
import os
ROOT=Path(__file__).resolve().parents[3]
def fixture(root):
    external=os.environ.get('CRUSADER_FIXTURE_ROOT')
    if external:
        path=Path(external)/'saved-order'
        return (path/'before-wire.txt').read_text().strip(),(path/'after-wire.txt').read_text().strip()
    here=Path(__file__).parent
    with tempfile.TemporaryDirectory() as directory:
        binary=Path(directory)/'synthetic'
        subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-Werror','-I',str(root/'src/server/coa'),str(here/'synthetic.cpp'),'-o',str(binary)],check=True)
        before=subprocess.run([str(binary)],check=True,capture_output=True,text=True).stdout.strip()
    raw=bytes.fromhex(before)
    for spell,old,new in [(64127,46,45),(64129,45,46),(31879,48,47),(31880,49,48),(31881,47,49)]:
        needle=spell.to_bytes(4,'little')+old.to_bytes(4,'little')
        assert raw.count(needle)==1
        raw=raw.replace(needle,spell.to_bytes(4,'little')+new.to_bytes(4,'little'))
    return before,raw.hex()
def main():
    here=Path(__file__).parent
    with tempfile.TemporaryDirectory() as directory:
        binary=Path(directory)/"saved-order"
        subprocess.run(["c++","-std=c++20","-Wall","-Wextra","-Werror","-I",str(ROOT/"src/server/coa"),str(here/"harness.cpp"),"-o",str(binary)],check=True)
        before,expected=fixture(ROOT)
        input_path=Path(directory)/"before-wire.txt";input_path.write_text(before)
        result=subprocess.run([str(binary),str(input_path)],check=True,capture_output=True,text=True)
        assert result.stdout.strip()==expected
    print("PASS native saved-build decode/encode; five rank-level changes only; exact full-document reversal")
if __name__=="__main__": main()
