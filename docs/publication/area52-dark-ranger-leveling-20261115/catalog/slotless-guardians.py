"""Rebase the two slotless summons onto a matching Area 52 Spell.dbc."""
import struct

def update(data):
    magic,n,_,stride,strings=struct.unpack_from('<4s4I',data)
    if magic!=b'WDBC' or stride<114*4 or len(data)!=20+n*stride+strings:
        raise ValueError('Unexpected spell catalog layout')
    result=bytearray(data)
    expected={34433:(19668,(1561,829,713)),46585:(26125,(829,713))}
    seen=set()
    for pos in range(20,20+n*stride,stride):
        spell=struct.unpack_from('<I',data,pos)[0]
        if spell not in expected:
            continue
        field=lambda col:struct.unpack_from('<I',data,pos+col*4)[0]
        entry,allowed=expected[spell]
        if spell in seen or field(71)!=28 or field(110)!=entry or field(113) not in allowed:
            raise ValueError(f'Unexpected summon record {spell}')
        seen.add(spell)
        struct.pack_into('<I',result,pos+5*4,field(5)&~1)
        struct.pack_into('<I',result,pos+113*4,713)
    if seen!=expected.keys():
        raise ValueError('Missing summon record')
    return bytes(result)
