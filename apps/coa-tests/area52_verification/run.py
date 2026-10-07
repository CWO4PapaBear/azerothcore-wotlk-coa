import importlib.util
import json
from pathlib import Path
import struct
import tempfile

ROOT = Path(__file__).resolve().parents[3]
spec = importlib.util.spec_from_file_location('status', ROOT / 'tools/area52_verification/status.py')
status = importlib.util.module_from_spec(spec)
spec.loader.exec_module(status)


def main():
    passed = {key: {'result': 'pass'} for key in status.GATES}
    assert status.classify(passed) == 'VERIFIED'
    assert status.classify({}) != 'VERIFIED'
    for key in status.GATES:
        for result, expected in [('pending', status.STATUSES[1]), ('fail', status.STATUSES[2])]:
            checks = dict(passed)
            checks[key] = {'result': result}
            assert status.classify(checks) == expected
    assert status.classify(passed, ['known defect']) == 'Confirmed Defect'
    row = {'spell': 10, 'name': 'Example', 'connected': True, 'issues': [], 'grants': [], 'rank_chains': []}
    nodes = {10: {'present': 'true', 'edges': [], 'scripts': [], 'effects': [{'effect': 6, 'aura': 107}]}}
    assert status.inspect(row, 'mystic', nodes, {})['status'] != 'VERIFIED'
    evidence = {'mystic_slots': 'Reviewed common persistence path'}
    assert status.inspect(row, 'mystic', nodes, evidence)['status'] == 'VERIFIED'
    assert status.inspect(row, 'mystic', nodes, {**evidence, 'unmatched_modifier_spells': [10]})['status'] == 'Needs Investigation'
    row['grants'] = [{'source': 10, 'target': 11}]
    assert status.inspect(row, 'mystic', nodes, evidence)['status'] != 'VERIFIED'
    row['grants'] = []
    row['rank_chains'] = [{'server': [10, 11], 'client': [10]}]
    assert status.inspect(row, 'mystic', nodes, evidence)['status'] == 'Needs Investigation'
    row['rank_chains'] = []
    nodes[10]['edges'] = [{'target': 11}]
    nodes[11] = {'present': 'true', 'scripts': ['custom_grant'], 'edges': [{'target': 10}]}
    assert status.inspect(row, 'mystic', nodes, evidence)['status'] != 'VERIFIED'
    strings = b'\0Original @ext:Keep this detail:ext@\0'
    record = [0] * 234
    record[0], record[170] = 10, 1
    dbc = struct.pack('<4s4I', b'WDBC', 1, 234, 936, len(strings)) + struct.pack('<234I', *record) + strings
    report = {'entries': [{'spells': [10], 'status': 'VERIFIED'}]}
    first, ids = status.patch_spell_dbc(dbc, report)
    second, _ = status.patch_spell_dbc(first, report)
    pointer = struct.unpack_from('<I', second, 20 + 170 * 4)[0]
    text = second[956 + pointer:].split(b'\0')[0].decode()
    assert text.count(status.MARKER) == 1
    assert '|cff00ff00Area 52 engineering: VERIFIED|r' in text
    assert 'Original @ext:Keep this detail:ext@' in text
    assert ids == [10]
    report['entries'].append({'spells': [10], 'status': 'Confirmed Defect'})
    patched, _ = status.patch_spell_dbc(dbc, report)
    assert b'Area 52 engineering: Confirmed Defect' in patched
    report['entries'].append({'spells': [11], 'status': 'VERIFIED'})
    try:
        status.patch_spell_dbc(dbc, report)
        raise AssertionError('Missing client spell accepted')
    except ValueError:
        pass
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        (path / 'effective.json').write_text('{}')
        (path / 'evidence.json').write_text(json.dumps({'effective_sha256': 'stale'}))
        try:
            status.build(path, path / 'evidence.json')
            raise AssertionError('Stale evidence accepted')
        except ValueError:
            pass
    print('PASS: all four gates, missing evidence, grants, rank gaps, nested scripts, cycles, SHIFT labels and stale data')


if __name__ == '__main__':
    main()
