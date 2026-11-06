from pathlib import Path
import hashlib
import json
import struct
import sys
import argparse

root = Path(__file__).resolve().parent
sys.path.insert(0, 'C:/Users/danie/Documents/Codex/2026-09-12/can/work/github-upload/tools')
from lib.mpq import MPQArchive, write_archive
data = Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data')
source = data/'area-52/patch-E.MPQ'
out = root/'client-candidate'
parser = argparse.ArgumentParser()
parser.add_argument('--worker', action='store_true')
parser.add_argument('--verified', action='store_true')
options = parser.parse_args()
if options.verified:
    gate = json.loads((root/'engineering-qualified.json').read_text())
    assert gate['status'] == 'passed' and gate['verified_entries'] == [1149,1150,1151,1152]
    for relative, expected in gate['candidate_hashes'].items():
        assert hashlib.sha256((root/relative).read_bytes()).hexdigest() == expected
if options.worker:
    runtime = Path('C:/Users/danie/Documents/Codex/area52-featured-catalog-runtime')
    source = runtime/'inputs/archive'
    assert hashlib.sha256(source.read_bytes()).hexdigest() == json.loads((runtime/'config.json').read_text())['archive']['sha256']
    out = root/'worker-candidate'
out.mkdir(exist_ok=True)
spell_key = r'DBFilesClient\Spell.dbc'
enchant_key = r'DBFilesClient\SpellItemEnchantment.dbc'
sha = lambda b: hashlib.sha256(b).hexdigest()

def parts(blob):
    magic,n,f,z,s=struct.unpack_from('<4s4I',blob)
    assert magic==b'WDBC' and z==4*f and len(blob)==20+n*z+s
    offsets={struct.unpack_from('<I',blob,i)[0]:i for i in range(20,20+n*z,z)}
    assert len(offsets)==n
    return offsets,20+n*z,z

with MPQArchive(source) as archive:
    names=archive.read_file('(listfile)').decode().splitlines()
    files={k:archive.read_file(k) for k in names if k and k.lower()!='(listfile)'}
spell_key=next(k for k in files if k.lower()==spell_key.lower())
base=files[spell_key]
before={k:sha(v) for k,v in files.items()}
offset,end,z=parts(base)
server=(root/'candidate/Spell.dbc').read_bytes()
so,se,sz=parts(server)
assert sz==z==936
result=bytearray(base[:end])
pool=bytearray(base[end:])
def string(data,offset):
    return data[offset:data.index(0,offset)]
def update_text(row,field,before,after):
    address=row+field*4
    old=string(pool,struct.unpack_from('<I',result,address)[0])
    assert old.count(before)==1
    struct.pack_into('<I',result,address,len(pool))
    pool.extend(old.replace(before,after)+bytes(1))

texts={
 84864:('Grants you bonus Strength, and each point of Strength now also increases your |cFFFFFFFFAttack Power|r and |cFFFFFFFFParry|r.',
 'Gain 2 Strength and 1 Attack Power per level. Each point of Strength now grants 1 melee Attack Power. Gain parry rating equal to 50% of your Strength.'),
 84865:('Grants you bonus Agility, and each point of Agility now also increases your |cFFFFFFFFAttack Power|r, |cFFFFFFFFCritical Strike Chance|r, and |cFFFFFFFFCritical Strike Damage|r.',
 'Gain 2 Agility and 1 Attack Power per level. Each point of Agility now grants 1 melee Attack Power, matching ranged Attack Power. Agility grants twice its usual melee and ranged critical strike chance. On melee and ranged critical hits, Attack Power from Agility contributes twice its usual damage.'),
 84866:('Grants you bonus |cFFFFFFFFSpell Power|r, Intellect, Spirit, and |cFFFFFFFFSpell Power|r gained from items and effects is doubled.',
 'Gain 2 Intellect and 1 Spirit per level. Each point of Spell Power from items and effects grants 2 spell damage power and 1 healing power.'),
 84867:('Grants you bonus |cFFFFFFFFHealing Power|r, |cFFFFFFFFIntellect|r, |cFFFFFFFFSpirit|r, and each point of |cFFFFFFFFSpell Power|r now also increases your |cFFFFFFFFHealing Power|r.',
 'Gain 2 Intellect and 1 Spirit per level. Healing power from Spell Power is doubled: each point grants 2 healing power and 1 spell damage power.')}
for sid,(old,new) in texts.items():
    update_text(offset[sid],170,old.encode(),new.encode())
    if options.verified:
        update_text(offset[sid],170,b'|cffff0000NOT VERIFIED|r',b'|cff00ff00VERIFIED|r')
for sid in (986203,986201,92840,92843,986202,92839,92842):
    start=offset[sid]+68*4
    result[start:start+8]=server[so[sid]+68*4:so[sid]+70*4]
for sid,donor in enumerate((49410,36744,36753,36754,36761,36758,36760,36762),13977921):
    assert sid not in offset
    row=bytearray(base[offset[donor]:offset[donor]+z])
    struct.pack_into('<I',row,0,sid)
    if sid==13977921:
        struct.pack_into('<I',row,80*4,49)
        struct.pack_into('<I',row,136*4,len(pool))
        pool.extend(b'Path of Strength: Parry Rating\0')
    else:
        row[440:444]=server[so[sid]+440:so[sid]+444]
    result.extend(row)
struct.pack_into('<I',result,4,len(offset)+8)
struct.pack_into('<I',result,16,len(pool))
files[spell_key]=bytes(result+pool)
with MPQArchive(data/'patch-S.MPQ') as a:original_enchants=a.read_file(enchant_key)
assert sha(original_enchants)=='da85ddff6cb70b91f295910a6b203d50690a38f2fb0be64edcc561927f07b5b0'
assert not any(k.lower()==enchant_key.lower() for k in files)
files[enchant_key]=(root/'candidate/SpellItemEnchantment.dbc').read_bytes()
write_archive(out/'patch-E.MPQ',files)
with MPQArchive(out/'patch-E.MPQ') as a:
    for k,v in files.items():assert a.read_file(k)==v
assert all(k==spell_key or before[k]==sha(files[k]) for k in before)
new_offsets,pe,_=parts(files[spell_key])
for sid in texts:
    description=string(files[spell_key][pe:],struct.unpack_from('<I',files[spell_key],new_offsets[sid]+680)[0])
    status = b'|cff00ff00VERIFIED|r' if options.verified else b'|cffff0000NOT VERIFIED|r'
    assert texts[sid][1].encode() in description and status in description and description.count(b'@s:')==2
(out/'readback.json').write_text(json.dumps(dict(baseline=sha(source.read_bytes()),candidate=sha((out/'patch-E.MPQ').read_bytes()),members=len(files),modified=[spell_key],added=[enchant_key],status='offline staging only'),indent=2))
print((out/'readback.json').read_text())
