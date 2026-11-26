from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
with tempfile.TemporaryDirectory() as directory:
    binary = Path(directory) / 'preview'
    subprocess.run(['g++', '-std=c++20', '-O2', '-Wall', '-Wextra', '-Werror',
        '-I', str(root / 'src/server/coa'), str(Path(__file__).parent / 'native_test.cpp'), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
subprocess.run(['python3', str(Path(__file__).parent / 'integration_test.py')], check=True)
