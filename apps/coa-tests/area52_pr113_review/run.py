import contextlib
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from unittest.mock import patch

root = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(root / 'tools'))
from inherited_sql import preserved_paths, MANIFEST, SOURCE_COMMIT

source = (root / 'src/server/coa/CoAGameplayTest.cpp').read_text()
start = source.index('                uint32 const reaction = definition.get<uint32>("reaction", REACT_PASSIVE);')
end = source.index('\n            }', start)
with tempfile.TemporaryDirectory() as temporary:
    path = Path(temporary)
    cpp = path / 'fixture.cpp'
    cpp.write_text(Path(__file__).with_name('fixture.cpp.in').read_text().replace('@NATIVE@', source[start:end]))
    subprocess.run(['g++', '-std=c++20', '-Wall', '-Wextra', '-Werror', str(cpp), '-o', str(path / 'fixture')], check=True)
    subprocess.run([str(path / 'fixture')], check=True)
    sql = path / 'data/sql/base/example.sql'
    sql.parent.mkdir(parents=True)
    sql.write_bytes(b'SELECT 1;\n')
    blob = hashlib.sha1(b'blob 10\0SELECT 1;\n').hexdigest()
    manifest = path / MANIFEST
    manifest.parent.mkdir(parents=True)
    data = {'source_commit': SOURCE_COMMIT, 'files': {'data/sql/base/example.sql': blob}}
    manifest.write_text(json.dumps(data))
    tree = b'100644 blob ' + blob.encode() + b'\tdata/sql/base/example.sql\0'
    with patch('inherited_sql.subprocess.run', return_value=type('Result', (), {'stdout': tree})()):
        assert preserved_paths(path) == {'data/sql/base/example.sql'}
        for mutation in ['bytes', 'provenance', 'path', 'missing', 'symlink']:
            sql.unlink(missing_ok=True)
            sql.write_bytes(b'SELECT 1;\n')
            altered = json.loads(json.dumps(data))
            if mutation == 'bytes': sql.write_bytes(b'SELECT 2;\n')
            elif mutation == 'provenance': altered['files']['data/sql/base/example.sql'] = '0' * 40
            elif mutation == 'path': altered['files']['../escape.sql'] = blob
            elif mutation == 'missing': sql.unlink()
            else:
                sql.unlink()
                sql.symlink_to(path / 'elsewhere.sql')
            manifest.write_text(json.dumps(altered))
            try: preserved_paths(path)
            except ValueError: pass
            else: raise AssertionError(mutation)

spec = importlib.util.spec_from_file_location('cpp_lint', root / 'apps/codestyle/codestyle-cpp.py')
lint = importlib.util.module_from_spec(spec)
spec.loader.exec_module(lint)
for code, bad in [('auto s = "a;;b";\n', False), ('int x = 1;;\n', True), ('for (;;) {}\n', False), ('/* ;; */ int x;\n', False)]:
    lint.error_handler = False
    with contextlib.redirect_stdout(io.StringIO()): lint.misc_codestyle_check(io.StringIO(code), 'fixture.cpp')
    assert lint.error_handler == bad, code
print('Native fixture options and rejection paths, immutable SQL protection and literal-safe C++ lint passed; engine adapters used.')
