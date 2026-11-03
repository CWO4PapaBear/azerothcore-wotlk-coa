from pathlib import Path
import fcntl,hashlib,json,os,shutil,subprocess,sys,tempfile
from client_state import ensure_closed
from idle_listener import find,idle,stop
ROOT=Path(os.environ.get('AREA52_PACKAGE_DIR',Path(__file__).resolve().parents[1]))
TX=Path(os.environ.get('AREA52_TRANSACTION_DIR',ROOT))
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def cfg():return json.loads((ROOT/'qualification.json').read_text())
def task(phase):
    q=cfg()
    with tempfile.TemporaryDirectory(prefix='editor-task-',dir=q['bridge']) as directory:
        root=Path(directory)
        for name in ('Task.ps1','task-before.xml'):
            shutil.copyfile(ROOT/'hooks'/name,root/name)
            if sha(ROOT/'hooks'/name)!=sha(root/name):raise RuntimeError('Task bridge mismatch')
        win=lambda p:subprocess.check_output(['wslpath','-w',str(p)],text=True).strip()
        result=subprocess.run([q['powershell'],'-NoProfile','-NonInteractive','-ExecutionPolicy','Bypass','-File',win(root/'Task.ps1'),'-Before',win(root/'task-before.xml'),'-Phase',phase],cwd=root,capture_output=True,timeout=90)
        if result.returncode or ('A52_EDITOR_TASK_OK:'+phase).encode() not in result.stdout.splitlines():raise RuntimeError('Catalog task '+phase+' failed; Administrator PowerShell and unchanged task required; private response withheld')
def files(after=False):
    q=cfg()
    for name,digest in q['after' if after else 'before'].items():
        if sha(Path(q['runtime'])/name)!=digest:raise RuntimeError('Catalog runtime drift: '+name)
    if sha(q['client'])!=q['client_after' if after else 'client_before']:raise RuntimeError('Installed client drift')
def preflight():
    ensure_closed();files();task('preflight')
    q=cfg();pid=find(str(Path(q['runtime'])/'catalog_events.py'))
    if pid and not idle(pid,Path(q['runtime'])/'outbox'):raise RuntimeError('Catalog delivery busy; wait for its existing job')
def backup():
    files();q=cfg();root=TX/'editor-before'
    if root.exists():raise RuntimeError('Preserve existing editor backup')
    root.mkdir()
    for name,digest in q['before'].items():
        p=root/name;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(Path(q['runtime'])/name,p)
        if sha(p)!=digest:raise RuntimeError('Backup mismatch')
    shutil.copyfile(q['client'],root/'client.MPQ')
    if sha(root/'client.MPQ')!=q['client_before']:raise RuntimeError('Client backup mismatch')
def replace(source,target):
    target=Path(target);pending=target.with_name(target.name+'.editor-pending')
    if pending.exists():raise RuntimeError('Preserve pending editor copy')
    shutil.copyfile(source,pending);pending.replace(target)
def apply():
    ensure_closed();files();q=cfg();task('disable')
    stop(str(Path(q['runtime'])/'catalog_events.py'),Path(q['runtime'])/'outbox')
    with (Path(q['runtime'])/'outbox/worker.lock').open('r') as lock:
        fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        files()
        for name in q['after']:replace(ROOT/'runtime'/name,Path(q['runtime'])/name)
        replace(ROOT/'client/patch-E.MPQ',q['client'])
def verify():
    ensure_closed();files(True);task('verify')
    if find(str(Path(cfg()['runtime'])/'catalog_events.py')):raise RuntimeError('Catalog listener started before publication')
def rollback():
    ensure_closed();q=cfg();runtime=Path(q['runtime'])
    if find(str(runtime/'catalog_events.py')):
        files();task('enable');return
    with (runtime/'outbox/worker.lock').open('r') as lock:
        fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        for name,before in q['before'].items():
            if sha(runtime/name) not in (before,q['after'][name]) or sha(TX/'editor-before'/name)!=before:raise RuntimeError('Runtime rollback conflict')
        if sha(q['client']) not in (q['client_before'],q['client_after']) or sha(TX/'editor-before/client.MPQ')!=q['client_before']:raise RuntimeError('Client rollback conflict')
        for name in q['before']:replace(TX/'editor-before'/name,runtime/name)
        replace(TX/'editor-before/client.MPQ',q['client'])
    task('enable')
def enable():
    files(True);task('enable')
if __name__=='__main__':
    {'preflight':preflight,'backup':backup,'apply':apply,'verify':verify,'rollback':rollback,'enable':enable}[sys.argv[1]]()
