import os
import json
from pathlib import Path
import re
import subprocess
import time
import uuid
from concurrent.futures import ThreadPoolExecutor


ROOT = Path(__file__).resolve().parents[3]
MIGRATION = ROOT / 'data/sql/updates/pending_db_characters/rev_20261003_01_area52_build_storage.sql'


def statement(name, arguments):
    source = (ROOT / 'src/server/database/Database/Implementation/CharacterDatabase.cpp').read_text()
    body = re.search(r'PrepareStatement\(' + name + r',\s*((?:"(?:[^"\\]|\\.)*"\s*)+),', source)[1]
    sql = ''.join(json.loads(part) for part in re.findall(r'"(?:[^"\\]|\\.)*"', body))
    pieces = sql.split('?')
    assert len(pieces) == len(arguments) + 1
    result = pieces[0]
    for value, piece in zip(arguments, pieces[1:]):
        result += (str(value) if isinstance(value, int) else "'" + value.replace("'", "''") + "'") + piece
    return result + ';'


def run(*args, sql=None):
    return subprocess.run(args, input=sql, text=True, capture_output=True, timeout=90)


def main():
    name = 'coa-build-storage-' + uuid.uuid4().hex
    image = os.environ.get('COA_TEST_MYSQL_IMAGE', 'mysql:8.4')
    started = run('docker', 'run', '--pull=never', '--detach', '--rm', '--network=none',
                  '--name', name, '-e', 'MYSQL_ALLOW_EMPTY_PASSWORD=yes', image)
    if started.returncode:
        raise RuntimeError(started.stderr)

    def query(sql, error=None):
        result = run('docker', 'exec', '-i', name, 'mysql', '-uroot', '--batch', '--skip-column-names', sql=sql)
        if error:
            if result.returncode == 0 or error not in result.stderr:
                raise AssertionError(result.stdout + result.stderr)
        elif result.returncode:
            raise RuntimeError(result.stderr)
        return result.stdout.strip()

    def db(sql, error=None):
        return query('USE coa_test_build_storage;\n' + sql, error)

    try:
        deadline = time.monotonic() + 180
        while True:
            result = run('docker', 'exec', name, 'mysql', '-uroot', '-NBe', 'SELECT 1')
            logs = run('docker', 'logs', name)
            if (result.returncode == 0 and result.stdout.strip() == '1'
                    and 'MySQL init process done.' in logs.stdout + logs.stderr):
                break
            if time.monotonic() >= deadline:
                raise RuntimeError('Isolated MySQL did not become ready')
            time.sleep(0.5)
        query('CREATE DATABASE coa_test_build_storage;')
        migration = MIGRATION.read_text(encoding='utf-8')
        db(migration)
        db("INSERT INTO area52_build (id, owner_account) VALUES (1, 100), (2, 200);")
        db("INSERT INTO area52_build_revision (build_id, revision, category, name, document) VALUES "
           "(1, 1, 2, 'Bulwark', JSON_OBJECT('spells', JSON_ARRAY(100, 200))), "
           "(2, 1, 2, 'Copy', JSON_OBJECT('spells', JSON_ARRAY(100, 200))); ")
        db('INSERT INTO area52_build_featured (build_id, revision, featured_by_account) VALUES (1, 1, 300);')
        db(migration)
        assert db('SELECT COUNT(*) FROM area52_build_revision;') == '2'
        db("INSERT INTO area52_build_revision (build_id, revision, category, name, document) "
           "VALUES (1, 2, 2, 'Edited', JSON_OBJECT('spells', JSON_ARRAY(300))); ")
        assert db('SELECT revision FROM area52_build_featured WHERE build_id=1;') == '1'
        assert db('SELECT owner_account FROM area52_build WHERE id=2;') == '200'
        assert db('SELECT COUNT(*) FROM area52_build_featured WHERE build_id=2;') == '0'
        db('DELETE FROM area52_build_revision WHERE build_id=1 AND revision=1;', '1451')
        db('INSERT INTO area52_build_featured VALUES (2, 9, 300, CURRENT_TIMESTAMP);', '1452')
        db("INSERT INTO area52_build_revision (build_id, revision, category, name, document) "
           "VALUES (1, 2, 2, 'Duplicate', '{}');", '1062')
        for document in ("'[]'", "JSON_OBJECT('text', REPEAT('x', 65537))"):
            db('INSERT INTO area52_build_revision (build_id, revision, category, name, document) '
               f"VALUES (1, 3, 2, 'Invalid', {document});", '3819')
        db("START TRANSACTION; INSERT INTO area52_build_revision "
           "(build_id, revision, category, name, document) VALUES (2, 2, 2, 'Abort', '{}'); ROLLBACK;")
        assert db('SELECT COUNT(*) FROM area52_build_revision WHERE build_id=2;') == '1'
        db('DELETE FROM area52_build WHERE id=2;')
        assert db('SELECT COUNT(*) FROM area52_build_revision WHERE build_id=2;') == '0'
        assert db('SELECT COUNT(*) FROM area52_build_revision WHERE build_id=1;') == '2'
        db(statement('CHAR_INS_A52_BUILD', [3, 100]))
        assert db(statement('CHAR_SEL_A52_BUILD_COUNT', [3, 100])) == '2\t1'
        assert db(statement('CHAR_SEL_A52_BUILD_COUNT', [4, 200])) == '0\t0'
        db(statement('CHAR_INS_A52_BUILD_REVISION', [1, 1, 'Native', 'abcd', 3, 100, 0]))
        assert db(statement('CHAR_SEL_A52_BUILD', [3])).split('\t')[:3] == ['100', '1', 'abcd']
        db(statement('CHAR_INS_A52_BUILD_REVISION', [2, 1, 'Intruder', 'ffff', 3, 200, 1]))
        assert db(statement('CHAR_SEL_A52_BUILD', [3])).split('\t')[:3] == ['100', '1', 'abcd']
        db(statement('CHAR_INS_A52_BUILD_REVISION', [2, 1, 'Update', 'cdef', 3, 100, 1]))
        db(statement('CHAR_INS_A52_BUILD_REVISION', [2, 1, 'Stale', 'ffff', 3, 100, 1]))
        assert db(statement('CHAR_SEL_A52_BUILD', [3])).split('\t')[:3] == ['100', '2', 'cdef']
        db('INSERT INTO area52_build_featured VALUES (3, 1, 300, CURRENT_TIMESTAMP);')
        assert db(statement('CHAR_SEL_A52_BUILD_PUBLIC', [3, 200])).split('\t')[1:] == ['1', 'abcd', '1']
        assert db(statement('CHAR_SEL_A52_BUILD_PUBLIC', [3, 100])).split('\t')[1:] == ['2', 'cdef', '0']
        assert '100\t2\tcdef\t0' in db(statement('CHAR_SEL_A52_BUILD_LIST', [100, 0, 0, 100])).splitlines()
        assert '100\t1\tabcd\t1' in db(statement('CHAR_SEL_A52_BUILD_LIST', [200, 0, 0, 200])).splitlines()
        def competing_save(wire):
            try:
                db('START TRANSACTION;' + statement('CHAR_INS_A52_BUILD_REVISION',
                   [3, 1, 'Concurrent', wire, 3, 100, 2]) + 'COMMIT;')
            except RuntimeError as error:
                if not any(code in str(error) for code in ('1213', '1062')):
                    raise
        with ThreadPoolExecutor(max_workers=2) as workers:
            list(workers.map(competing_save, ['aaaa', 'bbbb']))
        assert db('SELECT COUNT(*) FROM area52_build_revision WHERE build_id=3 AND revision=3;') == '1'
        assert db(statement('CHAR_SEL_A52_BUILD', [3])).split('\t')[2] in ('aaaa', 'bbbb')
        db('UPDATE area52_build SET archived=1 WHERE id=3;')
        assert db(statement('CHAR_SEL_A52_BUILD', [3])) == ''
        db(statement('CHAR_INS_A52_BUILD_REVISION', [4, 1, 'Archived', 'ffff', 3, 100, 3]))
        assert db('SELECT COUNT(*) FROM area52_build_revision WHERE build_id=3;') == '3'
        print('PASS: production prepared statements reject foreign-owner, stale and archived writes; Featured is pinned')
        print('PASS: migration replay, separate copies, pinned Featured revision, integrity, size limits and rollback')
    finally:
        stopped = run('docker', 'rm', '--force', name)
        if stopped.returncode:
            raise RuntimeError('Failed to remove isolated test container: ' + stopped.stderr)


if __name__ == '__main__':
    main()
