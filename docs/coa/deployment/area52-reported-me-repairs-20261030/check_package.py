from pathlib import Path
import hashlib,importlib.util,json,os,sys,tempfile
T=Path(os.environ['REPORTED_COMBINED_ROOT']);R=T.parent;D=T/'activation-package'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
manifest=json.loads((D/'package.json').read_text())
for row in manifest['artifacts']:assert sha(D/row['path'])==row['sha256'],row['path']
for a,b in [('collection/collection.py','scroll-candidate/collection.py'),('collection/schema.json','scroll-candidate/schema.json'),('cosmic/proc_hook.py','cosmic/proc_hook.py'),('cosmic/native-provenance.json','cosmic/native-provenance.json')]:assert (D/a).read_bytes()==(R/b).read_bytes()
for row in json.loads((T/'baseline.json').read_text()):
 rel=row['path'];p=D/'source'/rel
 original=R/'candidate'/rel
 if not original.exists():original=R/'cosmic/candidate'/rel
 assert p.read_text()==original.read_text()
def load(name,path):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def rejected(f):
 try:f()
 except RuntimeError:return
 raise AssertionError('Expected drift rejection')
with tempfile.TemporaryDirectory() as tmp:
 root=Path(tmp);payload=root/'payload';payload.mkdir();tx=root/'tx';tx.mkdir()
 rows=[]
 for i in range(3):
  target=root/('live'+str(i));target.write_bytes(b'before'+bytes([i]));artifact=payload/str(i);artifact.write_bytes(b'after'+bytes([i]))
  rows.append(dict(name=str(i),target=str(target),artifact=str(i),before_sha256=sha(target),after_sha256=sha(artifact)))
 (payload/'data-files.json').write_text(json.dumps(rows))
 m=load('data_files',D/'hooks/data_files.py');m.ROOT=payload;m.TX=tx
 m.preflight();m.backup();m.apply();m.verify();m.apply();m.verify();m.rollback();m.preflight()
 m.apply();Path(rows[0]['target']).write_bytes(b'concurrent-edit');rejected(m.apply);rejected(m.rollback)
 Path(rows[0]['target']).write_bytes((payload/'0').read_bytes());m.rollback();m.preflight()
 Path(rows[0]['target']).write_bytes((payload/'0').read_bytes());m.rollback();m.preflight()
 Path(rows[1]['target']).write_bytes(b'new-certification');rejected(m.preflight)
 sys.path.insert(0,str(D/'hooks'));client=load('client_files',D/'hooks/client_files.py')
 client.ROOT=payload;client.TX=tx;client.ensure_closed=lambda:None
 Path(rows[1]['target']).write_bytes((tx/'reported-data-backup/1').read_bytes())
 (payload/'client-files.json').write_text(json.dumps(rows))
 client.preflight();client.backup();client.apply();client.verify();client.rollback();client.preflight()
 def open_client():raise RuntimeError('open client')
 client.ensure_closed=open_client;rejected(client.preflight);rejected(client.apply);rejected(client.rollback)
(T/'package-hook-checks.json').write_text(json.dumps({'status':'passed','cases':['artifact integrity','qualified SQL hooks byte-identical','qualified source semantic identity','data/client apply and rollback','partial application rollback','drift and certification edit rejection','client closure rejection']},indent=2))
print('Combined package integrity and actual file-hook regression checks passed')
