import argparse
from pathlib import Path
import struct


def patch(raw):
    magic, count, fields, size, pool_size = struct.unpack_from('<4s4I', raw)
    if magic != b'WDBC' or fields < 226 or size != fields * 4 or len(raw) != 20 + count * size + pool_size:
        raise ValueError('Unsupported Spell.dbc layout')
    rows = bytearray(raw[:20 + count * size])
    pool = bytearray(raw[20 + count * size:])
    for i in range(count):
        offset = 20 + i * size
        if struct.unpack_from('<I', rows, offset)[0] != 84445:
            continue
        pointer = struct.unpack_from('<I', rows, offset + 4 * 170)[0]
        description = pool[pointer:pool.index(0, pointer)].decode('utf-8')
        old = 'Your Kick ability now knocks players back.'
        new = 'Your Kick ability now knocks enemies back.'
        if new in description:
            return raw
        if old not in description:
            raise ValueError('Unexpected Spartan Kick tooltip; reconcile before patching')
        updated = description.replace(old, new, 1)
        struct.pack_into('<I', rows, offset + 4 * 170, len(pool))
        pool.extend(updated.encode('utf-8') + b'\0')
        struct.pack_into('<I', rows, 16, len(pool))
        return bytes(rows) + pool
    raise ValueError('Missing Spartan Kick record')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if args.source.resolve() == args.output.resolve():
        parser.error('Use a separate output file')
    args.output.write_bytes(patch(args.source.read_bytes()))
