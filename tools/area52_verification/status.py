import hashlib
import json
import re
import struct
from collections import Counter
from pathlib import Path

STATUSES = ('VERIFIED', 'Structurally Connected, Unverified', 'Needs Investigation', 'Confirmed Defect')
LEGACY_MARKER = 'Area 52 engineering: '
GATES = ('connections', 'granted_abilities', 'persistence', 'rank_upgrades')


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def values(value):
    return value if isinstance(value, list) else []


def classify(checks, defects=()):
    if defects:
        return STATUSES[3]
    if any(check['result'] == 'fail' for check in checks.values()):
        return STATUSES[2]
    if set(checks) == set(GATES) and all(check['result'] in ('pass', 'not_applicable') for check in checks.values()):
        return STATUSES[0]
    return STATUSES[1]


def inspect(entry, kind, nodes, evidence):
    roots = [entry['spell']] if kind == 'mystic' else entry['spells']
    checks = {key: {'result': 'pending', 'evidence': []} for key in GATES}
    issues = entry['issues']
    connections = [i for i in issues if i['kind'] not in ('rank_chain_difference', 'server_rank_chain_absent_client')]
    missing = [spell for spell in roots if nodes.get(spell, {}).get('present') != 'true']
    if connections or missing or (kind == 'mystic' and not entry['connected']):
        checks['connections'] = {'result': 'fail', 'evidence': connections + missing}
    else:
        checks['connections'] = {'result': 'pass', 'evidence': ['Effective effect/handler graph has no unresolved connection flags.']}
    reachable = set()
    pending = list(roots)
    while pending:
        spell = pending.pop()
        if spell in reachable:
            continue
        reachable.add(spell)
        pending.extend(int(edge['target']) for edge in values(nodes.get(spell, {}).get('edges')))
    scripted = any(values(nodes.get(spell, {}).get('scripts')) for spell in reachable)
    supported_auras = {8, 13, 22, 29, 79, 80, 85, 87, 99, 101, 107, 108, 118, 135, 137, 138,
                      189, 216, 217, 220, 268, 290, 345}
    if kind == 'mystic' and checks['connections']['result'] == 'pass':
        effects = [effect for spell in reachable for effect in values(nodes.get(spell, {}).get('effects'))]
        primitive = effects and all(int(e['effect']) == 6 and int(e['aura']) in supported_auras for e in effects)
        if not primitive or scripted:
            checks['connections'] = {'result': 'pending', 'evidence': ['Outside reviewed common native modifier profile.']}
    orphaned = set(evidence.get('unmatched_modifier_spells', [])) & reachable
    if orphaned:
        checks['connections'] = {'result': 'fail', 'evidence': ['Modifier has no reviewed target: ' + str(sorted(orphaned))]}
    grants = entry['grants']
    if not grants and not scripted and not any(i['kind'] == 'tooltip_link_requires_review' for i in issues):
        checks['granted_abilities'] = {'result': 'not_applicable', 'evidence': ['No learn/replacement grant in the reviewed graph or description.']}
    else:
        checks['granted_abilities']['evidence'] = ['Grant or custom-script ownership review required.']
    profile = 'mystic_slots' if kind == 'mystic' else 'advancement_spells'
    if checks['granted_abilities']['result'] == 'not_applicable' and not scripted and evidence.get(profile):
        checks['persistence'] = {'result': 'pass', 'evidence': [profile, evidence[profile]]}
    else:
        checks['persistence']['evidence'] = ['Special grant/replacement lifecycle not covered by the common persistence profile.']
    chains = entry['rank_chains']
    bad_chains = []
    for chain in chains:
        server = chain['server']
        talent_chain = kind == 'advancement' and entry['te'] > 0 and server == roots
        if len(set(server)) != len(server) or any(rank not in nodes for rank in server):
            bad_chains.append(chain)
        elif chain['client'] != server and not talent_chain:
            bad_chains.append(chain)
    if bad_chains:
        checks['rank_upgrades'] = {'result': 'fail', 'evidence': bad_chains}
    elif not chains:
        checks['rank_upgrades'] = {'result': 'not_applicable', 'evidence': ['No higher rank in the reviewed effective graph.']}
    elif kind == 'advancement' and entry['te'] > 0:
        checks['rank_upgrades'] = {'result': 'pass', 'evidence': ['Talent ranks match advancement selection; books must not grant talent ranks.']}
    elif evidence.get('book_ranks') and not grants and not scripted:
        checks['rank_upgrades'] = {'result': 'pass', 'evidence': ['Client/server chains agree; common selected-ability book path.', evidence['book_ranks']]}
    defects = [d for d in evidence.get('defects', []) if set(d['spells']) & set(roots)]
    tooltip_spells = set(roots)
    for chain in chains:
        tooltip_spells.update(chain['server'])
        tooltip_spells.update(chain['client'] or [])
    tooltip_spells.update(grant['target'] for grant in grants)
    return {'kind': kind, 'id': entry['spell'] if kind == 'mystic' else entry['entry'],
            'name': entry['name'], 'spells': sorted(tooltip_spells), 'checks': checks,
            'status': classify(checks, defects), 'defects': defects, 'certified': False}


def build(audit, evidence_path):
    audit = Path(audit)
    evidence_path = Path(evidence_path)
    evidence = json.loads(evidence_path.read_text())
    if digest(audit / 'effective.json') != evidence['effective_sha256']:
        raise ValueError('Effective-data evidence is stale')
    for row in evidence['sources']:
        path = (evidence_path.parent / row['path']).resolve()
        if digest(path) != row['sha256']:
            raise ValueError('Reviewed source changed: ' + row['path'])
    data = json.loads((audit / 'effective.json').read_text())
    nodes = {int(node['id']): node for node in data['spells']}
    entries = []
    for kind, filename in (('mystic', 'enchant-audit.json'), ('advancement', 'ability-audit.json')):
        entries.extend(inspect(row, kind, nodes, evidence) for row in json.loads((audit / filename).read_text()))
    return {'schema': 1, 'definition': 'VERIFIED: engineering connections, grants, persistence and ranks; CERTIFIED: separate player acceptance.',
            'scope': 'Common-path implementation and existing persistence evidence plus per-entry data checks. No per-entry combat or reconnect claim.',
            'effective_sha256': evidence['effective_sha256'], 'evidence_sha256': digest(evidence_path),
            'counts': {kind: dict(Counter(row['status'] for row in entries if row['kind'] == kind)) for kind in ('mystic', 'advancement')},
            'entries': entries}


def tooltip(status, certified=False):
    if status == 'VERIFIED':
        label, color = ('CERTIFIED' if certified else 'VERIFIED'), '00ff00'
    else:
        label, color = 'NOT VERIFIED', 'ffff80'
    return '\n@ext:|cff' + color + label + '|r:ext@'


def format_description(description, status, certified=False, spell=0, preserve_shift_blocks=False):
    legacy = r'\n@ext:\|cff[0-9a-fA-F]{6}' + re.escape(LEGACY_MARKER) + r'.*?:ext@'
    description = re.sub(legacy, '', description, flags=re.S)
    label_pattern = r'\|cff[0-9a-fA-F]{6}(?:VERIFIED|CERTIFIED|NOT VERIFIED)\|r'
    description = re.sub(r'\s*@ext:' + label_pattern + r':ext@', '', description)
    description = re.sub(r'\s*' + label_pattern + r'(?=:ext@)', '', description)
    if preserve_shift_blocks:
        return description.rstrip() + '\n' + tooltip(status, certified)
    boundary = 'Gap closer ability.'
    if spell in (100, 6178, 11578) and boundary in description and '@ext:' not in description.lower():
        visible, extra = description.split(boundary, 1)
        description = visible.rstrip() + '\n\n@ext:' + boundary + extra.rstrip() + ':ext@'
    blocks = list(re.finditer(r'@ext:(.*?):ext@', description, flags=re.S | re.I))
    label = tooltip(status, certified)[6:-5]
    unconditional = [m for m in blocks if description[:m.start()].count('[') == description[:m.start()].count(']')]
    if unconditional:
        details = '\n\n'.join(m[1].strip() for m in unconditional if m[1].strip())
        for block in reversed(unconditional):
            description = description[:block.start()] + description[block.end():]
        return description.rstrip() + '\n\n@ext:' + (details + '\n\n' if details else '') + label + ':ext@'
    return description.rstrip() + '\n' + tooltip(status, certified)


def patch_spell_dbc(data, report):
    magic, count, fields, size, string_size = struct.unpack_from('<4s4I', data)
    if magic != b'WDBC' or fields != 234 or size != fields * 4 or len(data) != 20 + count * size + string_size:
        raise ValueError('Unexpected Spell.dbc layout')
    statuses = {}
    certifications = {}
    masteries = set()
    for row in report['entries']:
        for spell in row['spells']:
            if row.get('kind') == 'advancement' and 'mastery' in row.get('name', '').lower():
                masteries.add(spell)
            current = statuses.get(spell, STATUSES[0])
            statuses[spell] = STATUSES[max(STATUSES.index(current), STATUSES.index(row['status']))]
            certifications[spell] = certifications.get(spell, True) and row.get('certified') is True
    records = bytearray(data[:20 + count * size])
    strings = bytearray(data[20 + count * size:])
    changed = []
    for index in range(count):
        offset = 20 + index * size
        spell = struct.unpack_from('<I', records, offset)[0]
        if spell not in statuses:
            continue
        field = offset + 170 * 4
        pointer = struct.unpack_from('<I', records, field)[0]
        end = strings.find(b'\0', pointer)
        if end < 0:
            raise ValueError('Unterminated description')
        description = strings[pointer:end].decode('utf-8')
        new = format_description(description, statuses[spell], certifications[spell], spell, spell in masteries)
        struct.pack_into('<I', records, field, len(strings))
        strings.extend(new.encode('utf-8') + b'\0')
        changed.append(spell)
    if set(statuses) != set(changed):
        raise ValueError('Catalog spell missing from client; cannot label this pair')
    struct.pack_into('<I', records, 16, len(strings))
    return bytes(records + strings), changed


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--audit', required=True)
    parser.add_argument('--evidence', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    result = build(args.audit, args.evidence)
    Path(args.output).write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(result['counts']))
