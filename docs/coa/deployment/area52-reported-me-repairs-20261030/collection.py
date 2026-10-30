from pathlib import Path
import json
import os
import re
import subprocess

TABLE = 'account_freepick_mystic_collection'
OLD = 94986
NEW = 1294986


def query(statement):
    result = subprocess.run(['docker', 'exec', '-i', 'ascension-freepick-database', 'mysql',
        '--defaults-extra-file=/run/secrets/mysql-root.cnf', '-BNr', 'acore_characters'],
        input=statement, text=True, capture_output=True)
    if result.returncode:
        code = re.search(r'ERROR \d+ \([^)]+\)', result.stderr)
        raise RuntimeError('Collection migration SQL failed: ' + (code.group() if code else 'details withheld'))
    return result.stdout.strip()


def rows():
    return [list(map(int, line.split('\t'))) for line in query(
        f'SELECT account,spell FROM {TABLE} WHERE spell IN ({OLD},{NEW}) ORDER BY account,spell;').splitlines()]


def preflight():
    expected = json.loads((Path(__file__).parent/'schema.json').read_text())
    for table, schema in expected.items():
        if query('SHOW CREATE TABLE ' + table + ';') != schema:
            raise RuntimeError('Collection migration schema changed: ' + table)
    if query("SELECT COUNT(*) FROM character_settings WHERE source LIKE 'core.freepick.mystic.%' "
             "AND data REGEXP '(^|[^0-9])94986([^0-9]|$)';") != '0':
        raise RuntimeError('Legacy equipped enchant requires reconciliation; preserve saved slots')


def saved():
    return Path(os.environ['AREA52_TRANSACTION_DIR'])/'critical-judgement-collection.json'


def backup():
    preflight()
    before = rows()
    after = sorted({tuple(r) for r in before} | {(r[0], NEW) for r in before if r[1] == OLD})
    value = {'before': before, 'after': [list(r) for r in after]}
    path = saved()
    if path.exists():
        if json.loads(path.read_text()) != value:
            raise RuntimeError('Existing collection backup differs; preserve it')
    else:
        with path.open('x') as stream:
            json.dump(value, stream, indent=2)
            stream.flush()
            os.fsync(stream.fileno())


def transition(reverse=False):
    preflight()
    data = json.loads(saved().read_text())
    before, after = {tuple(r) for r in data['before']}, {tuple(r) for r in data['after']}
    if after != before | {(a, NEW) for a, spell in before if spell == OLD}:
        raise RuntimeError('Invalid collection backup')
    actual = {tuple(r) for r in rows()}
    if not before <= actual <= after:
        raise RuntimeError('Collection changed after backup; preserve it')
    target = before if reverse else after
    statements = ['CREATE TEMPORARY TABLE cj_guard (ok INT NOT NULL);', 'START TRANSACTION;']
    predicate = ' OR '.join(f'(account={a} AND spell={s})' for a, s in sorted(actual)) or 'FALSE'
    statements.append(f'INSERT INTO cj_guard SELECT IF(COUNT(*)={len(actual)} AND '
                      f'COALESCE(SUM({predicate}),0)={len(actual)},1,NULL) FROM {TABLE} '
                      f'WHERE spell IN ({OLD},{NEW}) FOR UPDATE;')
    for account, spell in sorted(actual - target):
        statements.append(f'DELETE FROM {TABLE} WHERE account={account} AND spell={spell};')
        statements.append('INSERT INTO cj_guard VALUES(IF(ROW_COUNT()=1,1,NULL));')
    for account, spell in sorted(target - actual):
        statements.append(f'INSERT INTO {TABLE} (account,spell) VALUES ({account},{spell});')
        statements.append('INSERT INTO cj_guard VALUES(IF(ROW_COUNT()=1,1,NULL));')
    query('\n'.join(statements + ['COMMIT;']))
    if {tuple(r) for r in rows()} != target:
        raise RuntimeError('Collection readback mismatch; preserve transaction')


def apply():
    transition()


def verify():
    if rows() != json.loads(saved().read_text())['after']:
        raise RuntimeError('Collection result differs')


def rollback():
    transition(True)


if __name__ == '__main__':
    import sys
    {'preflight': preflight, 'backup': backup, 'apply': apply, 'verify': verify, 'rollback': rollback}[sys.argv[1]]()
