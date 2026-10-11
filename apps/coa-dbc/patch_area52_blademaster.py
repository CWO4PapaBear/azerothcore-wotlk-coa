import argparse
from pathlib import Path
import struct

VERIFIED_SPELLS = {965862, 12295, 12676, 12677, 53385, *range(54762, 54768)}
NUMERIC = {
    965862: {72: 0, 73: 0},
    990042: {49: 3, 95: 4},
    965865: {49: 1},
    965867: {49: 1},
    965869: {49: 1},
    20231: {72: 0},
    20232: {72: 0, 73: 0, 95: 23, 98: 1000},
}


def patch(raw):
    magic, count, fields, size, pool_size = struct.unpack_from('<4s4I', raw)
    if magic != b'WDBC' or fields < 226 or size != fields * 4 or len(raw) != 20 + count * size + pool_size:
        raise ValueError('Unsupported Spell.dbc layout')
    rows = bytearray(raw[:20 + count * size])
    pool = bytearray(raw[20 + count * size:])
    seen = set()
    for i in range(count):
        offset = 20 + i * size
        spell = struct.unpack_from('<I', rows, offset)[0]
        if spell not in VERIFIED_SPELLS | NUMERIC.keys() | {965863}:
            continue
        seen.add(spell)
        for field, value in NUMERIC.get(spell, {}).items():
            struct.pack_into('<I', rows, offset + 4 * field, value)
        if spell in {12295, 12676, 12677}:
            struct.pack_into('<I', rows, offset + 4 * 80, 0xFFFFFFFF)
        if spell == 20232:
            attr = struct.unpack_from('<I', rows, offset + 4 * 8)[0]
            struct.pack_into('<I', rows, offset + 4 * 8, attr | 0x4000)
        pointer = struct.unpack_from('<I', rows, offset + 4 * 170)[0]
        description = pool[pointer:pool.index(0, pointer)].decode('utf-8')
        if spell in VERIFIED_SPELLS:
            updated = description.replace('|cffff0000NOT VERIFIED|r', '|cffffff00VERIFIED|r')
            if 'VERIFIED|r' not in updated and 'CERTIFIED|r' not in updated:
                raise ValueError(f'Missing existing status marker: {spell}')
            if updated != description:
                struct.pack_into('<I', rows, offset + 4 * 170, len(pool))
                pool.extend(updated.encode('utf-8') + b'\0')
        if spell == 965863:
            struct.pack_into('<I', rows, offset + 4 * 187, pointer)
    required = VERIFIED_SPELLS | NUMERIC.keys() | {965863}
    if required - seen:
        raise ValueError(f'Missing Blademaster records: {sorted(required - seen)}')
    struct.pack_into('<I', rows, 16, len(pool))
    return bytes(rows) + pool


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if args.source.resolve() == args.output.resolve():
        parser.error('Use a separate output file')
    args.output.write_bytes(patch(args.source.read_bytes()))
