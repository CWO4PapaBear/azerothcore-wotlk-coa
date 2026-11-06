from pathlib import Path
import sys,struct,json
T=Path(__file__).resolve().parent
sys.path.insert(0,'C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools')
from lib.mpq import MPQArchive
D=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data')
key='DBFilesClient\\SpellItemEnchantment.dbc'
for p in sorted(D.glob('*.MPQ'))+sorted((D/'area-52').glob('*.MPQ')):
 with MPQArchive(p) as a:
  if a.has_file(key):raw=a.read_file(key);origin=p
_,n,f,z,_=struct.unpack_from('<4s4I',raw); print(origin.name,n,f,z)
b=(T/'Spell.dbc').read_bytes();_,count,w,sz,_=struct.unpack_from('<4s4I',b);pool=b[20+count*sz:];spells={r[0]:r for r in struct.iter_unpack('<234i',b[20:20+count*sz])}
ids={r[110+j] for r in spells.values() for j in range(3) if r[71+j]==54 and pool[r[136]:pool.index(0,r[136])]==b'Rockbiter Weapon'};result=[]
for r in struct.iter_unpack('<'+'I'*f,raw[20:20+n*z]):
 if r[0] in ids:result.append(r);print(r[:14])
(T/'rockbiter-client-enchants.json').write_text(json.dumps({'source':str(origin),'rows':result},indent=2))
