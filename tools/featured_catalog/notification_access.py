import ctypes
import ctypes.util
import json
import os
from pathlib import Path
import stat
import subprocess
import sys


def check(directory, uid=1000, gid=1000):
    descriptor = os.open(directory, os.O_RDONLY | os.O_DIRECTORY)
    code = '''import ctypes,os,sys
fd,uid,gid=map(int,sys.argv[1:])
os.setgroups([]);os.setgid(gid);os.setuid(uid);os.fchdir(fd)
with open('area52-featured-catalog.signal','rb') as stream: assert stream.read(129)
lib=ctypes.CDLL(None,use_errno=True)
watch=lib.inotify_init1(os.O_CLOEXEC|os.O_NONBLOCK)
assert watch>=0
assert lib.inotify_add_watch(watch,b'.',0x8|0x80|0x100)>=0
os.close(watch)
print('Notification read and watch passed as uid='+str(os.getuid()))
'''
    try:
        result = subprocess.run([sys.executable, '-c', code, str(descriptor), str(uid), str(gid)],
                                pass_fds=(descriptor,), capture_output=True, text=True)
        if result.returncode:
            raise RuntimeError('Worker user cannot read or watch notification directory')
        return result.stdout.strip()
    finally:
        os.close(descriptor)


def grant(directory, receipt):
    directory = Path(directory)
    before = directory.stat()
    if before.st_uid != 0 or stat.S_IMODE(before.st_mode) != 0o700:
        raise RuntimeError('Notification directory differs from reviewed root-only baseline')
    lib = ctypes.CDLL(ctypes.util.find_library('acl'), use_errno=True)
    lib.acl_from_text.argtypes = [ctypes.c_char_p]
    lib.acl_from_text.restype = ctypes.c_void_p
    lib.acl_set_file.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_void_p]
    lib.acl_free.argtypes = [ctypes.c_void_p]
    with Path(receipt).open('x') as stream:
        json.dump({'path':str(directory),'uid':before.st_uid,'gid':before.st_gid,'mode':'0700',
                   'grant':'user:1000:r-x; no write, no recursive changes'},stream)
    acl = lib.acl_from_text(b'user::rwx,user:1000:r-x,group::---,mask::r-x,other::---')
    if not acl:
        raise RuntimeError('ACL construction failed')
    try:
        if lib.acl_set_file(os.fsencode(directory), 0x8000, acl):
            raise RuntimeError('Notification ACL update failed')
    finally:
        lib.acl_free(acl)
    return check(directory)


if __name__ == '__main__':
    if sys.argv[1] == 'check':
        print(check(sys.argv[2]))
    elif sys.argv[1] == 'grant':
        print(grant(sys.argv[2], sys.argv[3]))
    else:
        raise SystemExit('Expected check or grant')
