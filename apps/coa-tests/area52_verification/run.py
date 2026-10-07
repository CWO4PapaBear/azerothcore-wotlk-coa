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
    strings = b'\0Original @ext:Keep this detail:ext@\n@ext:|cff00ff00Area 52 engineering: VERIFIED|r\nEngineering review only; gameplay certification is separate.:ext@\0'
    record = [0] * 234
    record[0], record[170] = 10, 1
    dbc = struct.pack('<4s4I', b'WDBC', 1, 234, 936, len(strings)) + struct.pack('<234I', *record) + strings
    report = {'entries': [{'spells': [10], 'status': 'VERIFIED'}]}
    first, ids = status.patch_spell_dbc(dbc, report)
    second, _ = status.patch_spell_dbc(first, report)
    pointer = struct.unpack_from('<I', second, 20 + 170 * 4)[0]
    text = second[956 + pointer:].split(b'\0')[0].decode()
    assert text.count('|cff00ff00VERIFIED|r:ext@') == 1
    assert text.count('@ext:') == 1
    assert status.LEGACY_MARKER not in text
    assert 'certification is separate' not in text
    assert text == 'Original\n\n@ext:Keep this detail\n\n|cff00ff00VERIFIED|r:ext@'
    charge = 'Charge damage.\n\nMax 2 charges.\n\nGap closer ability.\n\nUsable while shapeshifted.'
    formatted = status.format_description(charge, 'VERIFIED', spell=11578)
    assert formatted == 'Charge damage.\n\nMax 2 charges.\n\n@ext:Gap closer ability.\n\nUsable while shapeshifted.\n\n|cff00ff00VERIFIED|r:ext@'
    assert status.format_description(formatted, 'VERIFIED', spell=11578) == formatted
    conditional = '$?s123[@ext:First branch:ext@][@ext:Second branch:ext@]'
    fixed = status.format_description(conditional, 'VERIFIED')
    assert fixed.endswith(']\n\n@ext:|cff00ff00VERIFIED|r:ext@')
    assert 'First branch:ext@' in fixed and 'Second branch:ext@' in fixed
    mastery = 'Mastery description.\n\n@ext:@s:168:0@~@s:168:-1@:ext@\n@ext:@s:6117:0@~@s:6117:-1@:ext@'
    fixed_mastery = status.format_description(mastery, 'Needs Investigation', preserve_shift_blocks=True)
    assert fixed_mastery == mastery + '\n\n@ext:|cffffff80NOT VERIFIED|r:ext@'
    assert status.format_description(fixed_mastery, 'Needs Investigation', preserve_shift_blocks=True) == fixed_mastery
    assert ids == [10]
    report['entries'].append({'spells': [10], 'status': 'Confirmed Defect'})
    patched, _ = status.patch_spell_dbc(dbc, report)
    pointer = struct.unpack_from('<I', patched, 20 + 170 * 4)[0]
    assert b'NOT VERIFIED|r:ext@' in patched[956 + pointer:].split(b'\0')[0]
    for label in status.STATUSES[1:]:
        assert status.tooltip(label) == '\n@ext:|cffffff80NOT VERIFIED|r:ext@'
    assert status.tooltip('VERIFIED', True) == '\n@ext:|cff00ff00CERTIFIED|r:ext@'
    assert 'NOT VERIFIED' in status.tooltip('Confirmed Defect', True)
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
