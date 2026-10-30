from pathlib import Path
import struct

OLD = 94986
NEW = 1294986
ITEM = 1177836


def repair_catalog(raw):
    magic, count, fields, size, strings = struct.unpack_from('<4s4I', raw)
    if (magic, fields, size) != (b'WDBC', 31, 124) or len(raw) != 20 + count * size + strings:
        raise ValueError('Unexpected MysticEnchant layout')
    rows = [struct.unpack_from('<31I', raw, 20 + i * size) for i in range(count)]
    targets = [(i, row) for i, row in enumerate(rows) if row[0] == 1902 or row[6] == ITEM]
    if len(targets) != 1:
        raise ValueError('Ambiguous enchant identity')
    index, row = targets[0]
    if row[0] != 1902 or row[6] != ITEM or row[1] not in (OLD, NEW) or not row[8] or not row[13] & 512:
        raise ValueError('Catalog baseline changed')
    if any(r[1] == NEW and i != index for i, r in enumerate(rows)):
        raise ValueError('Destination spell already mapped')
    output = bytearray(raw)
    struct.pack_into('<I', output, 20 + index * size + 4, NEW)
    return bytes(output)


def repair_status(document):
    import copy
    output = copy.deepcopy(document)
    entries = output if isinstance(output, list) else output.get('entries', [])
    old = [r for r in entries if r.get('kind') == 'mystic' and r.get('id') == OLD]
    new = [r for r in entries if r.get('kind') == 'mystic' and r.get('id') == NEW]
    if len(old) == 0 and len(new) == 1:
        return output
    if len(old) != 1 or new:
        raise ValueError('Status identity conflict')
    row = old[0]
    if row.get('certified') or row.get('status') == 'CERTIFIED':
        raise ValueError('Certified identity requires explicit reconciliation')
    row['id'] = NEW
    if 'name' in row:
        row['name'] = 'Critical Judgement of Righteousness'
    if 'spells' in row:
        row['spells'] = [NEW if value == OLD else value for value in row['spells']]
    return output


if __name__ == '__main__':
    import hashlib, json
    root = Path(__file__).resolve().parent
    target = root/'scroll-candidate'
    target.mkdir(exist_ok=True)
    before = (root/'scroll-baseline-MysticEnchant.dbc').read_bytes()
    after = repair_catalog(before)
    (target/'MysticEnchant.dbc').write_bytes(after)
    records = json.loads((root/'scroll-original-area52.json').read_text())['records']
    record = records[str(NEW)]
    assert record['name'] == 'Critical Judgement of Righteousness'
    assert record['values'][80] == 1 and record['values'][74] == 1
    result = {'old_spell': OLD, 'new_spell': NEW, 'item': ITEM,
              'before_sha256': hashlib.sha256(before).hexdigest(),
              'after_sha256': hashlib.sha256(after).hexdigest(),
              'status': 'Needs Investigation', 'scope': 'Catalog identity and collection ownership; no gameplay promotion'}
    (target/'mapping.json').write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps(result, indent=2))
