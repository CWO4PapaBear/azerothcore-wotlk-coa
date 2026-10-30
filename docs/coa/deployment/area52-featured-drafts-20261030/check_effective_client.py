from pathlib import Path
import sys,json,hashlib
T=Path(__file__).resolve().parent
sys.path.insert(0,'C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools')
from lib.mpq import MPQArchive
base=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data')
readback=json.loads((T/'readback.json').read_text())
names=set(readback['source_archives'])
for name in ('CharacterCreationArchetypes','CharacterCreationArchetypeCategories','CharacterCreationArchetypeRoles'):
 names.add('DBFilesClient\\'+name+'.dbc')
effective={};origins={}
for path in sorted(base.glob('*.MPQ'))+sorted((base/'area-52').glob('*.MPQ')):
 with MPQArchive(path) as archive:
  members=set(archive.read_file('(listfile)').decode().splitlines())
  for name in names&members:effective[name]=archive.read_file(name);origins[name]=str(path)
for name,path in readback['source_archives'].items():
 with MPQArchive(path) as archive:assert effective[name]==archive.read_file(name),name
for name in names-set(readback['source_archives']):
 assert effective[name]==(T/'catalog'/name.split('\\')[-1]).read_bytes(),name
(T/'effective-client-checks.json').write_text(json.dumps({'passed':True,'files':{name:{'source':origins[name],'sha256':hashlib.sha256(effective[name]).hexdigest()} for name in sorted(names)},'installed_changes':False},indent=2))
print('Effective installed UI and creation catalogs match the staged inputs across archive layers')
