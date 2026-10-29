"""Install/revert only the reviewed new module source directory; no runtime commands."""
from pathlib import Path
import hashlib,json,os,shutil,sys,tarfile
PRIVATE=Path('/home/dml/games/ascension-freepick-test/private')
PAYLOAD=Path(os.environ.get('AREA52_PACKAGE_DIR',Path(__file__).resolve().parents[1]))
TARGET=PRIVATE/'active-build-source/modules/mod-playerbots'
TRANSACTION=Path(os.environ.get('AREA52_TRANSACTION_DIR','/nonexistent'))
def require(ok,message):
    if not ok:raise RuntimeError(message)
def inventory():return json.loads((PAYLOAD/'module-inventory.json').read_text())
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def preflight():
    require(TARGET.parent.is_dir() and not TARGET.exists() and not TARGET.is_symlink(),'New module source target must be absent')
    expected=inventory()
    with tarfile.open(PAYLOAD/'mod-playerbots-source.tar.gz') as archive:
        entries=archive.getmembers()
        require(len(entries)==len(expected),'Module archive entry count mismatch')
        for entry in entries:
            name=Path(entry.name)
            require(entry.isfile() and not name.is_absolute() and '..' not in name.parts and entry.name in expected,'Unsafe module archive entry')
            require(hashlib.sha256(archive.extractfile(entry).read()).hexdigest()==expected[entry.name],'Module archive content mismatch')
def backup():
    preflight();(TRANSACTION/'module-was-absent').write_text('true')
def apply():
    preflight();TARGET.mkdir()
    with tarfile.open(PAYLOAD/'mod-playerbots-source.tar.gz') as archive:
        for entry in archive.getmembers():
            target=TARGET/entry.name;target.parent.mkdir(parents=True,exist_ok=True)
            target.write_bytes(archive.extractfile(entry).read());target.chmod(entry.mode&0o777)
def verify():
    expected=inventory()
    actual={p.relative_to(TARGET).as_posix():digest(p) for p in TARGET.rglob('*') if p.is_file()}
    require(actual==expected,'Installed module source differs from reviewed inventory')
def rollback():
    require((TRANSACTION/'module-was-absent').is_file(),'Missing module source baseline')
    if TARGET.exists():
        expected=inventory()
        for path in TARGET.rglob('*'):
            require(not path.is_symlink(),'Unexpected source symlink')
            if path.is_file():require(expected.get(path.relative_to(TARGET).as_posix())==digest(path),'Source edited since apply; preserve it')
        require(TARGET.resolve()==PRIVATE/'active-build-source/modules/mod-playerbots','Invalid removal target')
        shutil.rmtree(TARGET)
if __name__=='__main__':
    require(len(sys.argv)==2 and sys.argv[1] in ('preflight','backup','apply','verify','rollback'),'Invalid phase')
    globals()[sys.argv[1]]()
