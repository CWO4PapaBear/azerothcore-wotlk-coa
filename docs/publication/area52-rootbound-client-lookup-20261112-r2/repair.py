from pathlib import Path
import bisect,hashlib,json,struct,sys
r=Path(__file__).resolve().parent
w=r.parent
sys.path.insert(0,str(w))
from validate_white_walker_tooltips import archive_files
tools=Path('C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools')
src=Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data/area-52/patch-E.MPQ')
members=archive_files(src,tools)
from lib.mpq import write_archive
report={}
for table,count in [('Item',1),('Spell',10),('VanityCollection',1)]:
 key=next(k for k in members if k.lower()=='dbfilesclient\\'+table.lower()+'.dbc')
 b=members[key]; n,k,z=struct.unpack_from('<3I',b,4)
 ids=[struct.unpack_from('<I',b,20+i*z)[0] for i in range(n)]
 assert ids[-count:]==list(range(1999412,1999412+count))
 base=ids[:-count]
 pivot=bisect.bisect_left(base,1999412)
 assert base[pivot-1]<1999412 and base[pivot]>=1999412+count
 lo=20+pivot*z;hi=20+(n-count)*z;end=20+n*z
 new=b[:lo]+b[hi:end]+b[lo:hi]+b[end:]
 assert new[:lo]+new[lo+count*z:end]+new[end:]==b[:hi]+b[end:]
 ordered=base[:pivot]+ids[-count:]+base[pivot:]
 for id in ids[-count:]:
  pos=bisect.bisect_left(ordered,id)
  assert struct.unpack_from('<I',new,20+pos*z)[0]==id
 members[key]=new
 report[table]={'old_index':n-count,'correct_index':pivot,'private_rows':count}
out=r/'patch-E.MPQ'
write_archive(out,members)
assert archive_files(out,tools)==members
report['archive_sha256']=hashlib.file_digest(out.open('rb'),'sha256').hexdigest()
(r/'verification.json').write_text(json.dumps(report,indent=2))
print(report)

