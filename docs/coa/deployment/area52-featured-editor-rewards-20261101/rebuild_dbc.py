from pathlib import Path
import json,struct,sys
def rebuild(source,changes):
    magic,count,fields,size,pool=struct.unpack_from('<4s4I',source)
    if magic!=b'WDBC' or (fields,size) not in ((28,112),(53,212),(76,308)) or len(source)!=20+count*size+pool:raise ValueError('Unexpected DBC layout')
    rows={struct.unpack_from('<I',source,20+i*size)[0]:i for i in range(count)}
    if len(rows)!=count:raise ValueError('Duplicate DBC row identity')
    result=bytearray(source)
    for change in changes:
        if change['field']>=fields or change['row'] not in rows:raise ValueError('Missing DBC row/field')
        offset=20+rows[change['row']]*size+change['field']*4
        if struct.unpack_from('<I',source,offset)[0]!=change['before']:raise ValueError('DBC baseline differs')
        struct.pack_into('<I',result,offset,change['after'])
    return bytes(result)
if __name__=='__main__':
    if len(sys.argv)!=5:raise SystemExit('Usage: rebuild_dbc.py changes.json client|server INPUT_DIR OUTPUT_DIR')
    edits=json.loads(Path(sys.argv[1]).read_text());side,source,target=sys.argv[2:];destination=Path(target);destination.mkdir(parents=True,exist_ok=True)
    for name,changes in edits.items():
        if name.startswith(side+'/'):
            name=Path(name).name;(destination/name).write_bytes(rebuild((Path(source)/name).read_bytes(),changes))
