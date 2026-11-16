import struct

def update(data):
    magic,count,fields,stride,pool_size=struct.unpack_from('<4s4I',data)
    if magic!=b'WDBC' or stride<171*4 or len(data)!=20+count*stride+pool_size:
        raise ValueError('Unexpected Spell.dbc')
    pool_start=20+count*stride
    result=bytearray(data)
    found=set()
    for pos in range(20,pool_start,stride):
        field=lambda col:struct.unpack_from('<I',data,pos+4*col)[0]
        spell=field(0)
        if spell not in (81279,9931034):
            continue
        if spell in found:
            raise ValueError('Duplicate spell')
        found.add(spell)
        if spell==9931034:
            if field(40) not in (29,31) or field(95)!=4:
                raise ValueError('Unexpected mark')
            struct.pack_into('<I',result,pos+40*4,29)
            struct.pack_into('<I',result,pos+7*4,field(7)&~0x100000)
            continue
        if field(35) not in (10,20) or field(116)!=9931034:
            raise ValueError('Unexpected enchant')
        struct.pack_into('<I',result,pos+35*4,20)
        offset=pool_start+field(170)
        old=data[offset:data.index(b'\0',offset)]
        new=old.replace(b'10% chance',b'20% chance').replace(b'dark magic on the target,', b'dark magic on the target for 12 seconds,')
        if b'20% chance' not in new or b'12 seconds' not in new:
            raise ValueError('Unrecognized Dark Arrow tooltip: '+repr(old))
        if new!=old:
            struct.pack_into('<I',result,pos+170*4,len(result)-pool_start)
            result.extend(new+b'\0')
    if found!={81279,9931034}:
        raise ValueError('Missing Dark Arrow records')
    struct.pack_into('<I',result,16,len(result)-pool_start)
    return bytes(result)
