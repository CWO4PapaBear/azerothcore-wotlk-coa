import struct

def link_references(data, archetypes):
    magic, count, fields, size, strings = struct.unpack_from('<4s4I', data)
    if (magic, fields, size) != (b'WDBC', 157, 628) or len(data) != 20 + count * size + strings:
        raise ValueError('Unsupported character creation layout')
    offsets = {struct.unpack_from('<I', data, 20+i*size)[0]:20+i*size for i in range(count)}
    if len(offsets) != count or len(set(archetypes)) != len(archetypes):
        raise ValueError('Duplicate archetypes')
    output = bytearray(data)
    pool = 20 + count * size
    for archetype in archetypes:
        if archetype not in offsets or not 0 < archetype < 2**32:
            raise ValueError('Unknown archetype')
        field = offsets[archetype] + 8
        offset = struct.unpack_from('<I', output, field)[0]
        if offset >= strings:
            raise ValueError('Invalid string offset')
        existing = data[pool+offset:data.index(b'\0', pool+offset)]
        reference = str(2**63 + archetype).encode('ascii')
        if existing != reference:
            struct.pack_into('<I', output, field, len(output)-pool)
            output.extend(reference+b'\0')
    struct.pack_into('<I', output, 16, len(output)-pool)
    return bytes(output)
