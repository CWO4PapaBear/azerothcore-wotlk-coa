import struct

def correct_mark(data):
    magic,count,fields,stride,strings=struct.unpack_from('<4s4I',data)
    if magic!=b'WDBC' or stride<119*4 or len(data)!=20+count*stride+strings:
        raise ValueError('Unexpected Spell.dbc layout')
    offsets=[pos for pos in range(20,20+count*stride,stride)
             if struct.unpack_from('<I',data,pos)[0]==9931034]
    if len(offsets)!=1:
        raise ValueError('Dark Arrow mark missing or ambiguous')
    pos=offsets[0]
    def field(col):
        return struct.unpack_from('<I',data,pos+col*4)[0]
    if (field(71),field(86),field(95))!=(6,6,4):
        raise ValueError('Unexpected mark effect or target')
    staged=bytearray(data)
    struct.pack_into('<I',staged,pos+7*4,field(7)&~0x00100000)
    return bytes(staged)
