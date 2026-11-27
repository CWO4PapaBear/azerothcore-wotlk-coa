import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]


def main():
    source = (ROOT / 'src/server/coa/AscensionMysticEnchant.cpp').read_text()
    start = source.index('void SetGrantedSpell(')
    opening = source.index('{', start)
    end, depth = opening + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    code = Path(__file__).with_name('harness.cpp').read_text().replace('GRANT_FUNCTION', source[start:end])
    compiler = shutil.which(os.environ.get('CXX', 'c++'))
    assert compiler
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        (path / 'test.cpp').write_text(code)
        subprocess.run([compiler, '-std=c++20', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
        subprocess.run([str(path / 'test')], check=True)
    print('PASS production Dragon Roar/Dragon Charge durable grants, repeated relog, saved action slots, shared and independent ownership, abandonment cleanup and logout guards; database adapters used.')


if __name__ == '__main__':
    main()
