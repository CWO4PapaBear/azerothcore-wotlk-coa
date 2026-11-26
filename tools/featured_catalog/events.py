from pathlib import Path
import ctypes,json,os,re,select,struct,time
from common import save

class Watcher:
    def __init__(self,path):
        self.path=Path(path)
        self.lib=ctypes.CDLL(None,use_errno=True)
        self.fd=self.lib.inotify_init1(os.O_CLOEXEC|os.O_NONBLOCK)
        if self.fd<0:raise OSError(ctypes.get_errno(),'Notification initialization failed')
        self.watch=self.lib.inotify_add_watch(self.fd,os.fsencode(self.path.parent),0x8|0x80|0x100|0x400|0x800)
        if self.watch<0:
            self.close()
            raise OSError(ctypes.get_errno(),'Notification directory unavailable')

    def close(self):
        if self.fd>=0:
            os.close(self.fd)
            self.fd=-1

    def wait(self,timeout):
        if not select.select([self.fd],[],[],timeout)[0]:return False
        data=os.read(self.fd,65536)
        offset=0;changed=False
        while offset<len(data):
            wd,mask,cookie,size=struct.unpack_from('iIII',data,offset)
            name=data[offset+16:offset+16+size].split(b'\0',1)[0]
            offset+=16+size
            if mask&(0x400|0x800|0x8000):raise RuntimeError('Notification directory changed; listener restart required')
            if mask&0x4000 or name==os.fsencode(self.path.name):changed=True
        return changed


def signal(path):
    try:
        with Path(path).open('rb') as stream:data=stream.read(129)
    except FileNotFoundError:return ''
    if not re.fullmatch(rb'\d{1,24}-\d{1,24}\n',data):raise ValueError('Invalid catalog notification')
    return data.decode().strip()


class Delivery:
    def __init__(self,path,read_signal,deliver,clock=time.time):
        self.path=Path(path);self.read_signal=read_signal;self.deliver=deliver;self.clock=clock
        self.state=json.loads(self.path.read_text()) if self.path.exists() else {'schema':1,'pending':False,'attempt':0,'last_signal':''}
        if self.state['schema']!=1 or type(self.state['pending']) is not bool:raise ValueError('Invalid catalog event state')
        now=clock()
        if self.state['pending']:
            self.state['due']=min(float(self.state.get('due',now)),now+1800)
        else:
            self.state.update(pending=True,due=now+5,attempt=0)
        self.persist()

    def persist(self):save(self.path,self.state)

    def notify(self):
        current=self.read_signal()
        if current==self.state.get('last_signal') and not self.state['pending']:return
        due=self.clock()+5
        if self.state['pending']:due=min(due,self.state['due'])
        self.state.update(pending=True,due=due)
        self.persist()

    def delay(self):
        return max(0,self.state['due']-self.clock()) if self.state['pending'] else None

    def run_due(self):
        if not self.state['pending'] or self.clock()<self.state['due']:return False
        attempt=min(int(self.state.get('attempt',0))+1,4)
        self.state.update(attempt=attempt,due=self.clock()+(30,120,600,1800)[attempt-1])
        self.persist()
        try:
            before=self.read_signal()
            result=self.deliver()
            after=self.read_signal()
            if result not in ('unchanged','published','staged'):raise RuntimeError('Unverified catalog delivery result')
        except Exception as error:
            self.state['last_result']='retry:'+type(error).__name__
        else:
            self.state.update(last_signal=before,last_result=result,attempt=0,pending=before!=after)
            if self.state['pending']:self.state['due']=self.clock()+5
        self.persist()
        return True
