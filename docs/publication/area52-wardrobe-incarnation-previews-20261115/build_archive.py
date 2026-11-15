from pathlib import Path
import hashlib,json,sys
root=Path(__file__).resolve().parent
sys.path.insert(0,str(root.parent/'moonkin-wardrobe-review'))
from audit import MPQArchive,lookup,d
from lib.mpq import write_archive
installed=d/'area-52/patch-E.MPQ'
def contents(path):
 with MPQArchive(path) as a:
  return {n:a.read_file(n) for n in a.read_file('(listfile)').decode().splitlines() if n not in ('(listfile)','(attributes)','(signature)')}
published=Path.home()/'Documents/Codex/area52-featured-catalog-runtime/outbox/c56f0181a6ac84e40dbfbde7a8956d4b41476894f0e302994c47daec1afeb253/patch-E.MPQ'
public_row=next(x for x in json.loads((root/'release-baseline/manifest.json').read_text())['baseline'] if x['path'].lower()=='data/area-52/patch-e.mpq')
assert hashlib.sha256(published.read_bytes()).hexdigest()==public_row['sha256']
local=contents(installed);files=contents(published)
catalog_delta=[k for k in local.keys()|files.keys() if local.get(k)!=files.get(k)]
print('Preserved published catalog changes:',catalog_delta,flush=True)
assert all(k==r'Interface\GlueXML\CharacterCreate.lua' or k.lower().startswith('dbfilesclient') and k.split(chr(92))[-1] in ('CharacterCreationArchetypeDetails.dbc','CharacterCreationArchetypes.dbc','ItemSet.dbc','VanityCollection.dbc') for k in catalog_delta)
changes={}
for p in (root/'candidate').glob('*.lua'):
 key=r'Interface\AddOns\Ascension_AppearanceUI'+'\\'+p.name
 assert lookup(key)[1]==(root.parent/'moonkin-wardrobe-review/baseline'/p.name).read_bytes()
 changes[key]=p.read_bytes()
assets=root/'oakheart/candidate'
for p in assets.rglob('*'):
 if p.is_file():
  key=str(p.relative_to(assets)).replace('/','\\')
  prior=next((v for k,v in files.items() if k.lower()==key.lower()),None)
  assert prior is None or prior==p.read_bytes(),key
  changes[key]=p.read_bytes()
assert len(changes)==11 and len({n.lower() for n in files})==len(files)
for key,data in changes.items():
 dest=next((n for n in files if n.lower()==key.lower()),key)
 if dest in files and key.endswith('.lua'):assert files[dest]==(root.parent/'moonkin-wardrobe-review/baseline'/key.split(chr(92))[-1]).read_bytes()
 files[dest]=data
out=root/'patch-E.MPQ';write_archive(out,files);assert contents(out)==files
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(root/'lookup-report.json').write_text(json.dumps(dict(status='passed',before=sha(installed),after=sha(out),public_before=sha(published),catalog_delta=catalog_delta,members=len(files),changed=list(changes)),indent=2))


