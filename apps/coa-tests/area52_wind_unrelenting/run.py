from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/coa/Area52Unrelenting.inc').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text()
    for tag,sig in (('@RANK@','uint32 Area52UnrelentingRank('),('@ELIGIBLE@','bool Area52UnrelentingCast('),('@CONFIG@','void ConfigureArea52Unrelenting('),('@CAST@','void OnSpellBeforeEffects('),('@DAMAGE@','void OnSpellCalculatedTarget('),('@HIT@','void OnSpellHitResult('),('@REMOVE@','void OnAuraRemove(')):
        code=code.replace(tag,extract(source,sig).replace(' override',''))
    b=Path('/data/dbc/Spell.dbc').read_bytes();_,n,fields,size,_=struct.unpack_from('<4s4I',b)
    wanted={30664,30665,30666,30675,30678,30679,412000,412001,412002,412004,412005}
    rows={}
    for i in range(n):
        row=struct.unpack_from('<234I',b,20+i*size)
        if row[0] in wanted:rows[row[0]]=row
    assert len(rows)==len(wanted)
    for rank in range(1,4):
        row=rows[411999+rank]
        assert row[49]==3 and row[71:73]==(6,6) and row[95:97]==(65,138)
        assert row[80:82]==(rank-1,rank-1)
    assert rows[412004][72]==6 and rows[412004][96]==87 and rows[412004][81]==9 and rows[412004][111]==126
    duration=Path('/data/dbc/SpellDuration.dbc').read_bytes();_,dn,df,ds,_=struct.unpack_from('<4s4I',duration)
    durations={struct.unpack_from('<I',duration,20+i*ds)[0]:struct.unpack_from('<i',duration,24+i*ds)[0] for i in range(dn)}
    for id in (412000,412001,412002):assert durations[rows[id][40]]==8000
    assert durations[rows[412004][40]]==12000 and durations[rows[412005][40]]==3500
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS native Unrelenting cast, capstone snapshot/damage, Shock consumption and self-owned removal; actual three-rank haste/stack/duration metadata')
if __name__=='__main__':main()
