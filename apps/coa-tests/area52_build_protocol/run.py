import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent


def main():
    compiler = os.environ.get('CXX') or shutil.which('clang++') or shutil.which('g++')
    if not compiler:
        raise RuntimeError('A C++20 compiler is required')
    text = lambda value: value.encode('utf-8') + b'\0'
    fixture = b''.join(text(value) for value in ('42', '', '', '')) + struct.pack('<I', 10)
    fixture += b''.join(text(value) for value in ('Author', 'Fixture', 'Summary', 'Description', 'Icon'))
    fixture += struct.pack('<IQQIIII', 0, 0x123456789, 0x123456790, 1, 4, 1, 1)
    fixture += struct.pack('<IIBBBBI', 100, 1, 1, 0, 0, 0, 0) + text('spell')
    fixture += struct.pack('<IIIII', 1, 200, 2, 10, 0) + text('enchant')
    fixture += struct.pack('<II', 1, 2) + text('bow')
    fixture += struct.pack('<II', 1, 3) + text('mail')
    fixture += struct.pack('<BII', 0, 1, 2)
    with tempfile.TemporaryDirectory(prefix='coa-build-protocol-') as directory:
        path = Path(directory)
        sample = path / 'entry.bin'
        sample.write_bytes(fixture)
        executable = path / ('test.exe' if os.name == 'nt' else 'test')
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'src/server/coa'), str(HERE / 'harness.cpp'),
                        '-o', str(executable)], check=True, timeout=120)
        subprocess.run([str(executable), str(sample)], check=True, timeout=15)
        if os.environ.get('COA_BUILD_PACKET_SAMPLE'):
            subprocess.run([str(executable), os.environ['COA_BUILD_PACKET_SAMPLE'], 'native'],
                           check=True, timeout=15)
            print('PASS: native test draft matches expected spell/enchant counts and round-trips byte for byte')
    print('PASS: independent wire fixture, round trip, every truncation, duplicates, bounds and atomic decode')


if __name__ == '__main__':
    main()
