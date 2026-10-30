from pathlib import Path
import hashlib,json,subprocess,shutil,os,sys
W=Path(__file__).resolve().parent.parent
T=W/'draft-save-20261030'
report=sys.argv[1] if len(sys.argv)>1 else 'full-build-r4'
assert report.startswith('full-build-') and Path(report).name==report
B=Path('/home/dml/builds/area52-playerbots-20261026');S=B/'source'
A=Path('/home/dml/games/ascension-freepick-test/private/active-build-source')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
base=json.loads((W/'bot-integration-review/arena-companion-final-rebased-20261030-provenance.json').read_text())
previous=json.loads((B/'reviewed-build-inputs.json').read_text())
mounted=B/'last-mounted-overlays.json'
last=json.loads(mounted.read_text()) if mounted.exists() else previous
overlays=dict(base['core_overlays'])
for rel,digest in json.loads((T/'baseline.json').read_text()).items():
 before=A/rel if (A/rel).is_file() else S/rel
 if sha(before)!=digest:raise RuntimeError('Deployed source drift: '+rel)
for p in A.rglob('*'):
 if p.is_file() and p.suffix in ('.cpp','.h','.inc') and not str(p.relative_to(A)).startswith('modules/mod-playerbots/'):
  rel=p.relative_to(A).as_posix();digest=sha(p)
  if rel not in overlays and (S/rel).is_file() and sha(S/rel)==digest:continue
  frozen=B/'source-cache'/digest/p.name
  frozen.parent.mkdir(parents=True,exist_ok=True)
  if not frozen.exists():shutil.copyfile(p,frozen)
  overlays[rel]=digest
for p in (T/'source').rglob('*'):
 if p.is_file() and p.suffix in ('.cpp','.h','.inc'):
  rel=p.relative_to(T/'source').as_posix();digest=sha(p);frozen=B/'source-cache'/digest/p.name
  frozen.parent.mkdir(parents=True,exist_ok=True)
  if not frozen.exists():shutil.copyfile(p,frozen)
  overlays[rel]=digest
args=['docker','run','--rm','--network','none','--cpus','3','--cpu-shares','128','-v',str(S)+':/workspace','-v',str(B/'build')+':/botbuild','-v',str(T)+':/results','-v',base['module_snapshot']+':/workspace/modules/mod-playerbots:ro']
for rel,digest in overlays.items():
 frozen=B/'source-cache'/digest/Path(rel).name
 if sha(frozen)!=digest:raise RuntimeError('Frozen input mismatch: '+rel)
 if not (S/rel).exists():
  (S/rel).parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(frozen,S/rel)
 if last['core_overlays'].get(rel)!=digest:os.utime(frozen,None)
 args+=['-v',str(frozen)+':/workspace/'+rel+':ro']
for mount in json.loads(subprocess.check_output(['docker','inspect','ascension-freepick-worldserver','--format','{{json .Mounts}}'],text=True)):
 if mount['Destination']=='/data' or mount['Destination'].startswith('/data/dbc'):
  args+=['-v',mount['Source']+':'+mount['Destination']+':ro']
settings={'build_directory':'/botbuild','dbc_directory':'/data/dbc','jobs':3}
(T/'full-build-settings.json').write_text(json.dumps(settings))
provenance=dict(base,core_overlays=overlays)
(T/'full-build-provenance.json').write_text(json.dumps(provenance,indent=2))
mounted.write_text(json.dumps({'core_overlays':overlays,'report':report,'qualified':False},indent=2))
args+=['-w','/workspace','local/ascension-freepick-builder:20261001','nice','-n','15','python3','-B','tools/verify_all.py','--settings','/results/full-build-settings.json','--stages','build,unit','--jobs','3','--output','/results/'+report]
with (T/(report+'.log')).open('w') as log:r=subprocess.run(args,stdout=log,stderr=subprocess.STDOUT)
print((T/(report+'.log')).read_text()[-3000:])
if r.returncode==0:
 previous['core_overlays']=overlays;previous['report']='draft-save-20261030/'+report
 (B/'reviewed-build-inputs.json').write_text(json.dumps(previous,indent=2))
raise SystemExit(r.returncode)
