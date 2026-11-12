"""Transform one previously validated DBC row; input and output remain caller-owned."""
import struct
def corrected_catalog(raw):
    magic,n,fields,size,strings=struct.unpack_from('<4s4I',raw)
    assert (magic,fields,size)==(b'WDBC',234,936) and len(raw)==20+n*size+strings
    offsets={}; wanted={84561,84562}
    for pos in range(20,20+n*size,size):
        sid,=struct.unpack_from('<I',raw,pos)
        if sid in wanted:
            assert sid not in offsets
            offsets[sid]=pos
    assert offsets.keys()==wanted
    sibling=struct.unpack_from('<234I',raw,offsets[84561]); before=struct.unpack_from('<234I',raw,offsets[84562])
    assert (sibling[71:73],sibling[80],sibling[95],sibling[110])in {((6,0),0xfffffffb,72,17),((6,6),0xffffffff,107,3)}, (sibling[71:73],sibling[80],sibling[95],sibling[110])
    assert (before[71:73],before[74],before[80:82],before[95:97],before[110:112],before[116:118],before[49])==((6,6),1,(0xffffffff,0xffffffff),(107,107),(3,3),(1,32),1)
    out=bytearray(raw)
    for field,value in {72:0,80:0xfffffffb,95:72,96:0,110:33,111:0,116:0,117:0}.items():
        struct.pack_into('<I',out,offsets[84562]+4*field,value)
    return out
