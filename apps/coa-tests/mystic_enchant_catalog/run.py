import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE.parent))
from coa_talent_catalog import STUBS
from client_data import dbc_dir


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dbc-dir', type=Path)
    args = parser.parse_args()
    data = args.dbc_dir or dbc_dir()
    compiler = shutil.which(os.environ.get('CXX', 'cl.exe' if os.name == 'nt' else 'c++'))
    if not compiler:
        raise SystemExit('A C++20 compiler is required')
    with tempfile.TemporaryDirectory(prefix='coa-mystic-') as directory:
        out = Path(directory)
        for name, content in STUBS.items():
            (out / name).write_text(content, encoding='utf-8')
        includes = [out, ROOT / 'src/server/coa', ROOT / 'src/server/shared/DataStores', ROOT / 'src/common']
        sources = [HERE / 'harness.cpp', ROOT / 'src/server/coa/AscensionMysticEnchantCatalog.cpp',
                   ROOT / 'src/server/shared/DataStores/ClientDBC.cpp']
        executable = out / ('freepick.exe' if os.name == 'nt' else 'freepick')
        if Path(compiler).stem.lower() == 'cl':
            flags = ['/nologo', '/std:c++20', '/EHsc', '/utf-8',
                     *['/I' + str(p) for p in includes], *map(str, sources), '/Fe' + str(executable)]
        else:
            flags = ['-std=c++20', '-Wall', '-Wextra', *['-I' + str(p) for p in includes],
                     *map(str, sources), '-o', str(executable)]
        subprocess.run([compiler, *flags], cwd=out, check=True)
        subprocess.run([str(executable), str(data.resolve())], check=True)


if __name__ == '__main__':
    main()
