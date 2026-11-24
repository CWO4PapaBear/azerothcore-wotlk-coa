from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    aura=(ROOT/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text()
    effects=(ROOT/'src/server/game/Spells/SpellEffects.cpp').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@CHANCE@',extract(aura,'float Aura::CalcProcChance(')).replace('@HEAL@',extract(effects,'void Spell::EffectAscensionRestoreBaseHealthPct('))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    data=Path('/client-staging/DBFilesClient/Spell.dbc').read_bytes()
    _,n,fields,size,_=struct.unpack_from('<4s4I',data)
    pool=data[20+n*size:]
    found=set()
    for i in range(n):
        row=struct.unpack_from('<234I',data,20+i*size)
        if row[0] in (28176,28189,47892,47893):
            assert row[72]==6 and row[96]==42 and row[117]==828176 and row[99]==0 and row[34]==69904 and row[35]==10
            description=pool[row[170]:pool.index(0,row[170])].decode()
            assert '10% chance' in description and '3% of your base health' in description and '5 percentage points' in description and '2 percentage points' in description
            found.add(row[0])
        if row[0]==828176:
            assert row[71]==181 and row[95]==0 and row[80]==2 and row[6]&0x20000000
            found.add(row[0])
        if row[0] in (30143,30144,30145):
            description=pool[row[170]:pool.index(0,row[170])].decode()
            assert str(2*(row[0]-30142))+' percentage points' in description
            found.add(row[0])
    assert len(found)==8
    print('PASS Fel Armor actual native chance/base-health handler: all ranks, talent ranks0-3, live/dead/absent/non-demon pets, direct/periodic/auto/zero-damage/foreign actor, rank removal, other modes, healing reduction and guards')
if __name__=='__main__':main()
