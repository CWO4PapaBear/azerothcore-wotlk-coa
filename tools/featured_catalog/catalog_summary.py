"""Render the enUS spell summary while retaining all other native catalog fields."""
import re
import struct

MARKER='###AREA52_CREATION_SUMMARY'
RECORD=struct.Struct('<157I')
PREFIX=re.compile(rb'^\|cFF00f3ffUnlocks at level (\d+) \|r\r\n\r\n',re.I)

def selection(description):
    lines=description.split('\n')
    hits=[i for i,line in enumerate(lines) if line.startswith(MARKER)]
    if not hits:return None
    if len(hits)!=1 or lines[hits[0]]!=MARKER:raise ValueError('Invalid summary section')
    line=lines[hits[0]+1] if hits[0]+1<len(lines) else ''
    return parse(line)

def parse(line):
    if not line.startswith('1;'):raise ValueError('Unknown summary version')
    result=[]
    for item in line[2:].split(';'):
        m=re.fullmatch(r'([1-9][0-9]{0,8}):([1-9][0-9]?):([0-9a-f]{0,2048})',item)
        if not m:raise ValueError('Invalid summary row')
        spell,level=int(m[1]),int(m[2]);body=bytes.fromhex(m[3])
        body.decode('utf8')
        if level>80 or any(b<32 and b not in (10,13) for b in body) or b'\x7f' in body:raise ValueError('Invalid summary text or level')
        result.append((spell,level,body))
    if len(result)>5 or len({s for s,_,_ in result})!=len(result):raise ValueError('Duplicate or excessive summary abilities')
    return result

def transform(data, overrides):
    magic,n,k,w,size=struct.unpack_from('<4s4I',data)
    end=20+n*w
    if (magic,k,w)!=(b'WDBC',157,628) or end+size!=len(data):raise ValueError('Archetype catalog layout changed')
    records=bytearray(data[20:end]);pool=bytearray(data[end:]);defaults={}
    for i in range(n):
        row=list(RECORD.unpack_from(records,i*w));archetype=row[0];old=row[:];slots={};rows=[]
        for j,spell in enumerate(row[3:8]):
            if not spell:continue
            start=70+j*17;off=row[start];text=bytes(pool[off:pool.index(0,off)]);m=PREFIX.match(text)
            if not m or spell in slots:raise ValueError('Unsupported source summary')
            slots[spell]=(start,text);rows.append((spell,int(m[1]),text[m.end():]))
        defaults[archetype]=rows
        chosen=overrides.get(archetype)
        if chosen is None or chosen==rows:continue
        if len(chosen)!=len(slots) or len({s for s,_,_ in chosen})!=len(chosen):raise ValueError('Invalid summary positions')
        for j,(spell,level,body) in enumerate(chosen):
            dest=70+j*17;row[3+j]=spell
            if spell in slots:
                start,text=slots[spell];row[dest:dest+17]=old[start:start+17]
            else:
                text=b'|cFF00f3ffUnlocks at level 1 |r\r\n\r\n'
                row[dest:dest+17]=[0]*17
            match=PREFIX.match(text)
            new=text[:match.start(1)]+str(level).encode()+text[match.end(1):match.end()]+body
            if new!=text:
                row[dest]=len(pool);pool.extend(new+b'\0')
        RECORD.pack_into(records,i*w,*row)
    if set(overrides)-defaults.keys():raise ValueError('Missing summary archetype')
    return data[:16]+struct.pack('<I',len(pool))+records+pool,defaults
