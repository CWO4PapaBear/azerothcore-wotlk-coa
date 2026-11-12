"""Apply the two Auto Shot modal followups to a validated Area 52 client Spell.dbc."""
import struct
def corrected_followups(raw):
    magic,n,fields,size,strings=struct.unpack_from('<4s4I',raw)
    assert (magic,fields,size)==(b'WDBC',234,936) and len(raw)==20+n*size+strings
    data=bytearray(raw)
    found=[]
    for off in range(20,20+n*size,size):
        spell,=struct.unpack_from('<I',raw,off)
        if spell in (997517,997526):
            assert struct.unpack_from('<I',raw,off+192)==(0,)
            struct.pack_into('<I',data,off+192,75)
            found.append(spell)
    assert sorted(found)==[997517,997526]
    return bytes(data)
