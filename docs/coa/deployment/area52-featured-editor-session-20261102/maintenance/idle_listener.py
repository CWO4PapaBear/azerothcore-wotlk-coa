from pathlib import Path
import json,os,signal,time

def find(script):
    found=[]
    for p in Path('/proc').iterdir():
        if not p.name.isdigit():continue
        try:args=(p/'cmdline').read_bytes().split(b'\0')
        except (FileNotFoundError,PermissionError):continue
        if os.fsencode(script) in args:found.append(int(p.name))
    if len(found)>1:raise RuntimeError('Multiple catalog listeners; reconcile before activation')
    return found[0] if found else None

def idle(pid,outbox):
    state=json.loads((Path(outbox)/'event-state.json').read_text())
    delivery=json.loads((Path(outbox)/'delivery-state.json').read_text())
    call=(Path('/proc')/str(pid)/'syscall').read_text().split()
    return not state.get('pending') and not delivery.get('pending') and call and call[0]=='270'

def stop(script,outbox):
    pid=find(script)
    if pid is None:return
    handle=os.pidfd_open(pid)
    frozen=False;terminated=False
    try:
        if find(script)!=pid or not idle(pid,outbox):raise RuntimeError('Catalog listener is busy; no termination attempted')
        signal.pidfd_send_signal(handle,signal.SIGSTOP)
        frozen=True
        deadline=time.monotonic()+3
        status=Path('/proc')/str(pid)/'status'
        while '\nState:\tT' not in status.read_text():
            if time.monotonic()>deadline:raise RuntimeError('Catalog listener pause was not confirmed')
            time.sleep(.02)
        if not idle(pid,outbox):raise RuntimeError('Catalog listener became busy; resume without termination')
        signal.pidfd_send_signal(handle,signal.SIGTERM)
        signal.pidfd_send_signal(handle,signal.SIGCONT)
        frozen=False;terminated=True
        deadline=time.monotonic()+5
        while find(script)==pid:
            if time.monotonic()>deadline:raise RuntimeError('Idle listener did not exit')
            time.sleep(.05)
    finally:
        if frozen and not terminated:signal.pidfd_send_signal(handle,signal.SIGCONT)
        os.close(handle)
