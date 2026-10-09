from pathlib import Path
import json
import struct
import sys
import hashlib

p = Path(__file__).resolve().parents[3].parent / 'Area52_Legendary_Dependencies'
effective = json.loads((p / 'full-catalog-activation/effective.json').read_text())
spells = {int(s['id']): s for s in effective['spells']}
audit = json.loads((p / 'verification-ready-bindings/report.json').read_text())
summary = audit['stages']['gameplay']['summary']
assert summary['counts']['passed'] == 1 and not summary['counts']['failed']
assert not summary['failed_cases']
queue = json.loads((p / 'queue.json').read_text())
assert queue['legendary_count'] == 20
assert len(queue['entries']) == len({e['id'] for e in queue['entries']}) == 54
assert {577, 728, 807, 1301, 2958}.isdisjoint(e['id'] for e in queue['entries'])
roots = {78: 13, 139: 14, 172: 10, 348: 11, 689: 9, 1464: 8,
         2687: 1, 30455: 5, 11366: 12, 5570: 7, 34914: 5, 47540: 8, 48505: 4}
required = {139: 'spell_pri_renew', 1464: 'spell_warr_slam', 2687: 'spell_area52_hoplite_bloodrage',
            30455: 'spell_area52_battlemage_damage', 5570: 'spell_dru_insect_swarm',
            34914: 'spell_pri_vampiric_touch', 47540: 'spell_pri_penance'}
checked = []
for root, count in roots.items():
    chain = sorted((s for s in spells.values() if int(s.get('rank_root', 0)) == root), key=lambda s: int(s['rank']))
    assert len(chain) == count, root
    assert [int(s['rank']) for s in chain] == list(range(1, count + 1))
    for s in chain:
        assert s['present'] == 'true'
        if root in required:
            assert required[root] in s['scripts'], s['id']
        if int(s['cast_ms']) or s['channel'] == 'true':
            assert int(s['interrupt']) & 8, s['id']
        if s['channel'] == 'true':
            assert int(s['channel_interrupt']) & 8, s['id']
        assert all(e['effect_handler'] == 'true' and e['aura_handler'] == 'true' for e in s['effects'])
        assert not any(e['effect'] == '36' for e in s['effects'])
        checked.append(int(s['id']))
for root in (47757, 47758, 47666, 47750):
    chain = [s for s in spells.values() if int(s.get('rank_root', 0)) == root]
    assert {int(s['rank']) for s in chain} == set(range(1, 9)), root
    if root in (47757, 47758):
        assert all(s['channel'] == 'true' and int(s['channel_interrupt']) & 8 for s in chain)
assert 'spell_dru_starfall_dummy' in spells[50286]['scripts']
assert 'spell_dru_starfall_aoe' in spells[50294]['scripts']
assert all(spells[s]['duration_ms'] == '8000' for s in (48505, 53199, 53200, 53201))
assert any(e['effect'] == '31' for e in spells[50783]['effects'])
assert any(e['aura'] == '24' and e['amplitude_ms'] == '1000' for e in spells[29131]['effects'])
assert int(spells[2687]['power_type']) == -2 and int(spells[2687]['percent_cost']) == 8
assert 'spell_gen_replenishment' in spells[57669]['scripts']
sys.path.insert(0, str(p.parents[1] / 'work/github-upload/tools'))
from lib.mpq import MPQArchive

client = Path('D:/DML WOTLK Client Side/Ascension A52 Free Pick/ascension-live/ascension-live/Data/area-52/patch-D.MPQ')
with MPQArchive(client) as archive:
    raw = archive.read_file('DBFilesClient\\Spell.dbc')
    ranks = archive.read_file('DBFilesClient\\SpellRank.dbc')
_, count, fields, size, _ = struct.unpack_from('<4s4I', raw)
rows = {struct.unpack_from('<I', raw, 20 + i * size)[0]:
        struct.unpack_from('<' + 'I' * fields, raw, 20 + i * size) for i in range(count)}
client_chains = {}
for i in range(struct.unpack_from('<I', ranks, 4)[0]):
    _, root, spell, rank = struct.unpack_from('<4I', ranks, 20 + i * 16)
    client_chains.setdefault(root, []).append((rank, spell))
for root in roots:
    chain = sorted((int(s['rank']), int(s['id'])) for s in spells.values() if int(s.get('rank_root', 0)) == root)
    assert sorted(client_chains.get(root, [(1, root)])) == chain, root
    levels = [max(rows[s][38], rows[s][39]) for _, s in chain]
    assert levels == sorted(levels), root
    for _, sid in chain:
        row, server = rows[sid], spells[sid]
        assert row[39] == int(server['level']), sid
        if int(server['cast_ms']) or server['channel'] == 'true':
            assert row[31] & 8, sid
        if server['channel'] == 'true':
            assert row[5] & 68 and row[33] & 8, sid
        if root == 30455:
            assert row[133] == 186 and int(server['family']) == 3, sid
if (p / 'client-stage.json').exists():
    stage = json.loads((p / 'client-stage.json').read_text())
    assert hashlib.sha256(client.read_bytes()).hexdigest() == stage['before_sha256']
    assert hashlib.sha256((p / 'patch-D.MPQ').read_bytes()).hexdigest() == stage['after_sha256']
    with MPQArchive(client) as before, MPQArchive(p / 'patch-D.MPQ') as after:
        names = before.read_file('(listfile)').decode().splitlines()
        for name in names:
            if not name or name.lower() in ('(listfile)', '(attributes)', '(signature)'):
                continue
            old, new = before.read_file(name), after.read_file(name)
            if name != 'DBFilesClient\\Spell.dbc':
                assert old == new, name
                continue
            old_pool, new_pool = old[20 + count * size:], new[20 + count * size:]
            changed = []
            for i in range(count):
                a = list(struct.unpack_from('<' + 'I' * fields, old, 20 + i * size))
                b = list(struct.unpack_from('<' + 'I' * fields, new, 20 + i * size))
                x = old_pool[a[170]:old_pool.find(b'\0', a[170])]
                y = new_pool[b[170]:new_pool.find(b'\0', b[170])]
                if x != y:
                    assert a[0] in checked
                    assert x.replace(b'|cffff0000NOT VERIFIED|r', b'|cffffff00VERIFIED|r') == y
                    changed.append(a[0])
                a[170] = b[170] = 0
                assert a == b, a[0]
            assert sorted(changed) == sorted(stage['changed_tooltips']) == sorted(checked)
    candidate = json.loads((p / 'status.candidate.json').read_text())
    baseline = json.loads((p.parent / 'Area52_MysticEnchant_Review/engineering-verification/status.json').read_text())
    modified = []
    for old, new in zip(baseline['entries'], candidate['entries'], strict=True):
        if old != new:
            assert old['kind'] == 'advancement' and old['id'] == new['id']
            assert not old.get('certified') and new['status'] == 'VERIFIED'
            assert all(check['result'] == 'pass' for check in new['checks'].values())
            modified.append(old['id'])
    assert sorted(modified) == stage['entries']
(p / 'native-pass.json').write_text(json.dumps({'roots': roots, 'checked_ranks': checked,
    'scope': 'Native binding, rank and movement structure; not combat certification'}, indent=2))
print(f'PASS: 54-entry dependency batch; 13 native ability contracts across {len(checked)} ranks and 32 Penance helper ranks.')
