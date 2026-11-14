from pathlib import Path
import hashlib,struct,sys
from lib.mpq import MPQArchive
key=r'DBFilesClient\ItemAddon.dbc'
def stage(base,destination):
    with MPQArchive(base) as archive:
        original=archive.read_file(key)
    magic,count,fields,size,pool_size=struct.unpack_from('<4s4I',original)
    assert (magic,fields,size)==(b'WDBC',48,192)
    end=20+count*size
    assert len(original)==end+pool_size
    rows=memoryview(original)[20:end]
    pool=bytearray(original[end:])
    specs={1642945:(1999520,"Fused Quel'Serrar"),3148335:(1999521,"Dusty Draconian Deflector")}
    donors={}
    last=0
    for row in struct.iter_unpack('<48I',rows):
        assert row[0]>last
        last=row[0]
        assert row[1] not in (1999520,1999521)
        if row[1] in specs:
            assert row[1] not in donors
            donors[row[1]]=row
    assert set(donors)==set(specs)
    additions=bytearray()
    for rank,(donor,(item,name)) in enumerate(specs.items(),1):
        row=list(donors[donor])
        row[:3]=[last+rank,item,len(pool)]
        additions.extend(struct.pack('<48I',*row))
        pool.extend(name.encode()+b'\0')
    result=struct.pack('<4s4I',magic,count+2,fields,size,len(pool))+rows.tobytes()+additions+pool
    destination.write_bytes(result)
    check=destination.read_bytes()
    start=end+2*size
    assert check[20:end]==rows and check[start:start+pool_size]==original[end:]
    for row,(donor,(item,name)) in zip(struct.iter_unpack('<48I',check[end:start]),specs.items()):
        assert row[1]==item and row[3:]==donors[donor][3:] and row[36]==7
        assert check[start+row[2]:].split(b'\0',1)[0].decode()==name
    print('Verified both item IDs, names, heirloom quality and unchanged original records.')
if __name__=='__main__':
    stage(Path(sys.argv[1]),Path(sys.argv[2]))
