"""Guarded client, catalog-worker and server-catalog transaction files."""
from pathlib import Path
import fcntl
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile

from certification_guard import validate_certifications
from client_state import ensure_closed
from idle_listener import find, idle, stop

ROOT = Path(os.environ.get('AREA52_PACKAGE_DIR', Path(__file__).resolve().parents[1]))
TX = Path(os.environ.get('AREA52_TRANSACTION_DIR', ROOT))


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def config():
    return json.loads((ROOT / 'qualification.json').read_text())


def certifications():
    result = subprocess.run([
        'docker', 'exec', '-i', 'ascension-freepick-database', 'mysql',
        '--defaults-extra-file=/run/secrets/mysql-root.cnf', '-BNr', 'acore_characters',
    ], input=b'SELECT entry,checks_confirmed FROM area52_tester_certification ORDER BY entry;',
        capture_output=True, timeout=45)
    if result.returncode:
        raise RuntimeError('Read-only certification guard failed; credentials withheld')
    validate_certifications(result.stdout, config())


def task(phase):
    settings = config()
    with tempfile.TemporaryDirectory(prefix='primary-stat-task-', dir=settings['bridge']) as directory:
        bridge = Path(directory)
        for name in ('Task.ps1', 'task-before.xml'):
            shutil.copyfile(ROOT / 'hooks' / name, bridge / name)
            if sha(ROOT / 'hooks' / name) != sha(bridge / name):
                raise RuntimeError('Catalog task bridge hash mismatch')
        def windows_path(path):
            return subprocess.check_output(['wslpath', '-w', str(path)], text=True).strip()
        result = subprocess.run([
            settings['powershell'], '-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass',
            '-File', windows_path(bridge / 'Task.ps1'), '-Before', windows_path(bridge / 'task-before.xml'),
            '-Phase', phase,
        ], cwd=bridge, capture_output=True, timeout=90)
        success = ('A52_VANITY_TASK_OK:' + phase).encode()
        if result.returncode or success not in result.stdout.splitlines():
            raise RuntimeError('Catalog task ' + phase + ' failed; private response withheld')


def files(after=False):
    for record in config()['files']:
        if sha(record['target']) != record['after' if after else 'before']:
            raise RuntimeError('File baseline drift: ' + record['artifact'])
        if sha(ROOT / record['artifact']) != record['after']:
            raise RuntimeError('Payload mismatch: ' + record['artifact'])


def preflight():
    ensure_closed()
    files()
    certifications()
    task('preflight')
    runtime = Path(config()['runtime'])
    pid = find(str(runtime / 'catalog_events.py'))
    if pid and not idle(pid, runtime / 'outbox'):
        raise RuntimeError('Catalog delivery busy; preserve pending work')
    print('PRIMARY STAT FILE PREFLIGHT PASSED')


def backup():
    preflight()
    before = TX / 'primary-stat-before'
    before.mkdir()  # Never replace an existing transaction backup.
    for record in config()['files']:
        destination = before / record['artifact']
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(record['target'], destination)
        if sha(destination) != record['before']:
            raise RuntimeError('Backup readback failed')


def replace(source, target):
    target = Path(target)
    temporary = target.with_name(target.name + '.primary-stat-pending')
    if temporary.exists():
        raise RuntimeError('Preserve unfinished file copy')
    try:
        shutil.copyfile(source, temporary)
        if sha(source) != sha(temporary):
            raise RuntimeError('Copy readback failed')
        temporary.replace(target)
    finally:
        temporary.unlink(missing_ok=True)


def apply():
    ensure_closed()
    files()
    certifications()
    settings = config()
    before = TX / 'primary-stat-before'
    for record in settings['files']:
        if sha(before / record['artifact']) != record['before']:
            raise RuntimeError('Valid backup required')
    runtime = Path(settings['runtime'])
    task('disable')
    stop(str(runtime / 'catalog_events.py'), runtime / 'outbox')
    with (runtime / 'outbox/worker.lock').open('r') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        files()
        certifications()
        for record in settings['files']:
            replace(ROOT / record['artifact'], record['target'])


def verify():
    files(True)
    certifications()
    task('verify')
    if find(config()['runtime'] + '/catalog_events.py'):
        raise RuntimeError('Catalog worker started before publication')
    print('PRIMARY STAT FILE VERIFICATION PASSED')


def rollback():
    ensure_closed()
    settings = config()
    before = TX / 'primary-stat-before'
    for record in settings['files']:
        if sha(record['target']) not in (record['before'], record['after']):
            raise RuntimeError('Rollback conflict; preserve later edits')
        if sha(before / record['artifact']) != record['before']:
            raise RuntimeError('Rollback backup mismatch')
    runtime = Path(settings['runtime'])
    if find(str(runtime / 'catalog_events.py')):
        files()
        task('enable')
        return
    with (runtime / 'outbox/worker.lock').open('r') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        for record in settings['files']:
            replace(before / record['artifact'], record['target'])
    files()
    task('enable')


def enable():
    files(True)
    certifications()
    task('enable')


if __name__ == '__main__':
    phases = {name: globals()[name] for name in ('preflight', 'backup', 'apply', 'verify', 'rollback', 'enable')}
    phases[sys.argv[1]]()
