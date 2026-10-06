import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]


def block(text, marker):
    start = text.index(marker)
    opening = text.index('{', start)
    end, depth = opening + 1, 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def main():
    source = (ROOT / 'src/server/coa/AscensionHoplite.cpp').read_text()
    code = (Path(__file__).parent / 'harness.cpp').read_text()
    code = code.replace('DAMAGE_FUNCTION', block(source, 'int32 ShieldStrikeDamage('))
    code = code.replace('HIT_FUNCTION', block(source, 'void Hit('))
    compiler = shutil.which(os.environ.get('CXX', 'c++'))
    assert compiler
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        (path / 'test.cpp').write_text(code)
        subprocess.run([compiler, '-std=c++20', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
        subprocess.run([str(path / 'test')], check=True)
    print('PASS: production Shield Strike AP scaling, point cap, hit gating, single roll, cooldown and proc outcomes')


if __name__ == '__main__':
    main()
