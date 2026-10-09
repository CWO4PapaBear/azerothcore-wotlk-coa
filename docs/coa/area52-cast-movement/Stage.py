from pathlib import Path
import json, struct, sys, hashlib, collections

p = Path(__file__).resolve().parent
assert json.loads((p/'verification-complete/report.json').read_text())['status'] == 'passed'
sys.path.insert(0, str(p.parents[1]/'work/github-upload/tools'))
from lib.mpq import MPQArchive, write_archive

ids = set(json.loads((p/'repair-ids.json').read_text()))
client = Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data/area-52/patch-D.MPQ')
with MPQArchive(client) as a:
    files = {n:a.read_file(n) for n in a.read_file('(listfile)').decode().splitlines()
             if n and n.lower() not in ('(listfile)', '(signature)', '(attributes)')}
raw = files['DBFilesClient\\Spell.dbc']
_, count, fields, size, _ = struct.unpack_from('<4s4I', raw)
rows = bytearray(raw[:20+count*size])
pool = bytearray(raw[20+count*size:])
changed = []
for i in range(count):
    off = 20+i*size
    sid = struct.unpack_from('<I', raw, off)[0]
    if sid in ids:
        flag = struct.unpack_from('<I', raw, off+31*4)[0]
        assert flag == 14
        struct.pack_into('<I', rows, off+31*4, flag | 1)
        changed.append(sid)
    if sid == 954611:
        ptr = struct.unpack_from('<I', raw, off+170*4)[0]
        old = pool[ptr:pool.find(b'\0', ptr)].decode()
        new = old.replace('|cffffff00VERIFIED|r', '|cffff0000NOT VERIFIED|r')
        assert old != new
        struct.pack_into('<I', rows, off+170*4, len(pool))
        pool.extend(new.encode()+b'\0')
assert set(changed) == ids
struct.pack_into('<I', rows, 16, len(pool))
files['DBFilesClient\\Spell.dbc'] = bytes(rows)+pool
write_archive(p/'patch-D.MPQ', files)
audit = json.loads((p/'audit.json').read_text())
ledger = json.loads((p.parent/'Area52_MysticEnchant_Review/engineering-verification/status.json').read_text())
indexed = {(e['kind'], e['id']):e for e in ledger['entries']}
for e in audit:
    unresolved = any(c['id'] == 954611 for c in e['casts'])
    dispositions = []
    for c in e['casts']:
        sid = c['id']
        if sid in ids:
            result = 'repaired_candidate'
            reason = 'Restore stationary cast contract from PTR; extended ranks follow the same family; retain aura313 exceptions.'
        elif sid == 954611:
            result = 'needs_investigation'
            reason = 'Missing movement bit; no independent expected-behavior reference yet.'
        elif sid == 955072:
            result = 'intentional_mobile_cast'
            reason = 'Current SHIFT description explicitly allows casting while moving; retain CERTIFIED tooltip.'
        elif sid == 834491:
            result = 'triggered_helper'
            reason = 'Triggered by instant Silencing Shot 34490; not a directly learned cast.'
        else:
            assert c['movement'] and c['server_interrupt'] & 1 and c['sql_override'] is None
            assert not c['channel'] or c['channel_interrupt'] & 8
            result = 'existing_restriction'
            reason = 'Client/server flags match core start/update checks; channel move flag present where applicable.'
        dispositions.append(dict(spell=sid, result=result, evidence=reason))
    entry = indexed[(e['kind'], e['id'])]
    entry['checks']['movement_casting'] = dict(result='fail' if unresolved else 'pass', evidence=dispositions,
                                             scope='structural candidate; no in-game certification')
    if unresolved:
        entry['status'] = 'Needs Investigation'
    e['movement_result'] = 'needs_investigation' if unresolved else 'pass'
    e['dispositions'] = dispositions
ledger['definition'] = 'VERIFIED: engineering connections, grants, persistence, ranks and movement-casting rules; CERTIFIED: separate player acceptance.'
ledger['counts'] = {k:dict(collections.Counter(e['status'] for e in ledger['entries'] if e['kind']==k)) for k in ('mystic','advancement')}
(p/'status.candidate.json').write_text(json.dumps(ledger, indent=2))
(p/'reviewed.json').write_text(json.dumps(audit, indent=2))
(p/'client-stage.json').write_text(json.dumps(dict(status='staged_not_installed',
    before_sha256=hashlib.sha256(client.read_bytes()).hexdigest(),
    after_sha256=hashlib.sha256((p/'patch-D.MPQ').read_bytes()).hexdigest(),
    repaired_spells=sorted(ids), unresolved=[954611], upstream='fc359be9bf79ffb532c192d7afe754be21feba40'), indent=2))
print('Staged movement flags and one NOT VERIFIED label; no installed files changed.')
