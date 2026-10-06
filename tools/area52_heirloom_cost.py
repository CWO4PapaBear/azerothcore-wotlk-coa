CLI_DESCRIPTION = 'Add the Area 52 discounted heirloom cache cost to an existing ItemExtendedCost.dbc.'

import argparse
from pathlib import Path
import struct


def patch(data):
    if len(data) < 20 or data[:4] != b'WDBC':
        raise ValueError('Expected a WDBC cost table')
    count, fields, size, strings = struct.unpack_from('<4I', data, 4)
    if fields != 16 or size != 64 or len(data) != 20 + count * size + strings:
        raise ValueError('Unexpected ItemExtendedCost layout')
    rows = [(900052, 0, 0, 0, 375250, 0, 0, 0, 0, 3600, 0, 0, 0, 0, 0, 0),
            (900053, 0, 0, 0, 375250, 0, 0, 0, 0, 500, 0, 0, 0, 0, 0, 0),
            (900054, 0, 0, 0, 375250, 0, 0, 0, 0, 1000, 0, 0, 0, 0, 0, 0)]
    existing = {struct.unpack_from('<16I', data, 20 + index * size)[0]:
                struct.unpack_from('<16I', data, 20 + index * size) for index in range(count)}
    missing = []
    for row in rows:
        if row[0] in existing:
            if existing[row[0]] != row:
                raise ValueError(f'Cost ID {row[0]} is already used for another price')
        else:
            missing.append(row)
    end = 20 + count * size
    return (data[:4] + struct.pack('<4I', count + len(missing), fields, size, strings)
            + data[20:end] + b''.join(struct.pack('<16I', *row) for row in missing) + data[end:])



def main():
    parser = argparse.ArgumentParser(description=CLI_DESCRIPTION)
    parser.add_argument('input', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if args.input.resolve() == args.output.resolve():
        parser.error('Use a separate staged output file')
    args.output.write_bytes(patch(args.input.read_bytes()))


if __name__ == '__main__':
    main()
