from pathlib import Path
import json, struct, sys, collections

p = Path(__file__).resolve().parent
sys.path.insert(0, str(p.parents[1] / 'work/github-upload/tools'))
from lib.mpq import MPQArchive

def dbc(raw):
    _, n, f, size, _ = struct.unpack_from('<4s4I', raw)
    return {struct.unpack_from('<I', raw, 20+i*size)[0]: struct.unpack_from('<'+'I'*f, raw, 20+i*size) for i in range(n)}, raw[20+n*size:]

with MPQArchive(Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data/area-52/patch-D.MPQ')) as archive:
    client, strings = dbc(archive.read_file('DBFilesClient\\Spell.dbc'))
server, _ = dbc((p / 'server-Spell.dbc').read_bytes())
overrides = {int(v[0]): list(map(int, v[1:])) for line in (p / 'overrides.tsv').read_text().splitlines() if (v := line.split('\t'))}
times, _ = dbc((p / 'SpellCastTimes.dbc').read_bytes())
effective = json.loads((p.parent / 'Area52_Legendary_Batch7/full-catalog-activation/effective.json').read_text())
spells = {int(s['id']): s for s in effective['spells']}
ledger = json.loads((p.parent / 'Area52_MysticEnchant_Review/engineering-verification/status.json').read_text())
results = []
for entry in ledger['entries']:
    labels = []
    for sid in entry['spells']:
        if sid in client:
            off = client[sid][170]
            labels.append(strings[off:strings.find(b'\0', off)].decode(errors='replace'))
    if entry['status'] not in ('VERIFIED', 'CERTIFIED') and not any('|cffffff00VERIFIED|r' in label or '|cff00ff00CERTIFIED|r' in label for label in labels):
        continue
    queue = list(entry['spells'])
    seen = set()
    casts = []
    while queue:
        sid = int(queue.pop())
        if sid in seen:
            continue
        seen.add(sid)
        s = spells.get(sid, {})
        queue.extend(int(edge['target']) for edge in s.get('edges', []) if isinstance(edge, dict))
        row = client.get(sid)
        if row is None:
            continue
        t = times.get(row[28], (0, 0, 0, 0))
        cast_ms = struct.unpack('<i', struct.pack('<I', t[1]))[0]
        channeled = bool(row[5] & (4 | 64))
        if s.get('passive') == 'true' or row[4] & 64 or not (cast_ms > 0 or channeled):
            continue
        def string(field):
            off = row[field]
            return strings[off:strings.find(b'\0', off)].decode(errors='replace')
        casts.append(dict(id=sid, name=string(136), cast_ms=cast_ms, channel=channeled,
                          interrupt=row[31], channel_interrupt=row[33], server_interrupt=server.get(sid, [0]*32)[31],
                          sql_override=overrides.get(sid),
                          movement=bool(row[31]&1), description=string(170), scripts=s.get('scripts', ''),
                          direct=sid in entry['spells']))
    if casts:
        results.append(dict(kind=entry['kind'], id=entry['id'], name=entry['name'], status=entry['status'], casts=casts))
(p / 'audit.json').write_text(json.dumps(results, indent=2))
unique = {c['id']:c for e in results for c in e['casts']}
print('Entries with cast/channel graph:', len(results), 'unique spells:', len(unique))
for c in unique.values():
    if not c['movement']:
        print(c['id'], c['name'], c['cast_ms'], 'channel', c['channel'], 'direct', c['direct'], 'server', c['server_interrupt'])
