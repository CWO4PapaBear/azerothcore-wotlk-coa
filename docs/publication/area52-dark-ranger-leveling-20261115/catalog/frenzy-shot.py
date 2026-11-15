"""Disable the independent critical roll on the copied pet swing."""
import struct

def update(data):
    magic, count, fields, stride, strings = struct.unpack_from('<4s4I', data)
    if (magic, fields, stride) != (b'WDBC', 234, 936) or len(data) != 20 + count * stride + strings:
        raise ValueError('Unexpected Spell.dbc layout')
    matches = [p for p in range(20, 20 + count*stride, stride) if struct.unpack_from('<I', data, p)[0] == 954584]
    if len(matches) != 1:
        raise ValueError('Expected one Frenzied Cleave damage record')
    pos, = matches
    words = struct.unpack_from('<234I', data, pos)
    if words[71:74] != (2, 0, 0) or words[213] != 2:
        raise ValueError('Unexpected Frenzied Cleave damage profile')
    result = bytearray(data)
    struct.pack_into('<I', result, pos+6*4, words[6] | 0x20000000)
    return bytes(result)
