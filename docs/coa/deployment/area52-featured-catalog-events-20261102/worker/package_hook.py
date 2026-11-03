from pathlib import Path
import fcntl,hashlib,json,os,shutil,subprocess,sys,tempfile
ROOT=Path(os.environ.get('AREA52_PACKAGE_DIR',Path(__file__).resolve().parents[1]))
TX=Path(os.environ.get('AREA52_TRANSACTION_DIR',ROOT))

def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def cfg():return json.loads((ROOT/'qualification.json').read_text())

def task(phase):
    row=cfg();bridge_root=Path(row['bridge']).resolve()
    if not str(bridge_root).startswith('/mnt/c/') or not bridge_root.is_dir():raise RuntimeError('Windows task bridge unavailable')
    with tempfile.TemporaryDirectory(prefix='catalog-event-bridge-',dir=bridge_root) as temporary:
        bridge=Path(temporary)
        for name in ('Task-Control.ps1','task-before.xml','task-events.xml'):
            source=ROOT/'hooks'/name;shutil.copyfile(source,bridge/name)
            if sha(source)!=sha(bridge/name):raise RuntimeError('Task bridge hash mismatch')
        windows=lambda path:subprocess.check_output(['wslpath','-w',str(path)],text=True).strip()
        result=subprocess.run([row['powershell'],'-NoProfile','-NonInteractive','-ExecutionPolicy','Bypass','-File',windows(bridge/'Task-Control.ps1'),'-Before',windows(bridge/'task-before.xml'),'-After',windows(bridge/'task-events.xml'),'-Phase',phase],cwd=bridge,capture_output=True,timeout=180)
        if result.returncode or ('A52_EVENT_TASK_OK:'+phase).encode() not in result.stdout.splitlines():
            reason='execution_or_readback_failed'
            for needle,label in ((b'Administrator PowerShell','administrator_required'),(b'baseline changed','baseline_changed'),(b'busy','worker_busy'),(b'does not exist','script_not_accessible')):
                if needle in result.stderr:reason=label
            raise RuntimeError('Catalog event task '+phase+' failed: exit='+str(result.returncode)+', reason='+reason+'; response bodies withheld')

def files(after=False):
    row=cfg();runtime=Path(row['runtime'])
    expected=dict(row['before'])
    if after:expected.update(row['after'])
    for name,digest in expected.items():
        path=runtime/name
        actual=sha(path) if path.exists() else None
        if actual!=digest:raise RuntimeError('Catalog runtime baseline mismatch: '+name)

def idle():
    with (Path(cfg()['runtime'])/'outbox/worker.lock').open('r') as lock:
        try:fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        except BlockingIOError:raise RuntimeError('Existing catalog job is still running; preserve it') from None

def preflight():
    files();task('preflight');idle()
    if not Path(cfg()['signal']).parent.is_dir():raise RuntimeError('Server notification directory unavailable')

def backup():
    files()
    target=TX/'catalog-event-before'
    if target.exists():raise RuntimeError('Preserve existing worker backup')
    target.mkdir()
    for name in cfg()['after']:
        source=Path(cfg()['runtime'])/name
        if source.exists():
            shutil.copyfile(source,target/name)
            if sha(target/name)!=cfg()['before'][name]:raise RuntimeError('Worker backup readback mismatch')

def replace(source,target):
    target=Path(target);pending=target.with_name(target.name+'.event-pending')
    if pending.exists():raise RuntimeError('Existing pending worker file requires recovery')
    shutil.copyfile(source,pending);pending.replace(target)

def apply():
    files();task('pause');idle()
    for name in cfg()['after']:replace(ROOT/'runtime'/name,Path(cfg()['runtime'])/name)
    task('install')

def verify():
    files(True);task('verify')

def rollback():
    row=cfg();runtime=Path(row['runtime']);task('rollback-check');idle()
    for name,digest in row['after'].items():
        path=runtime/name;actual=sha(path) if path.exists() else None
        if actual not in (row['before'][name],digest):raise RuntimeError('Concurrent worker change; preserve recovery files')
        if row['before'][name] is not None and sha(TX/'catalog-event-before'/name)!=row['before'][name]:raise RuntimeError('Worker rollback backup mismatch')
    for name in row['after']:
        if row['before'][name] is None:(runtime/name).unlink(missing_ok=True)
        else:replace(TX/'catalog-event-before'/name,runtime/name)
    task('rollback')

def enable():
    files(True);task('enable')

if __name__=='__main__':
    {'preflight':preflight,'backup':backup,'apply':apply,'verify':verify,'rollback':rollback,'enable':enable}[sys.argv[1]]()
