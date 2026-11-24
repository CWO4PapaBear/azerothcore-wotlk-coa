from pathlib import Path
import struct,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    druid=(ROOT/'src/server/scripts/Spells/spell_druid.cpp').read_text()
    amount=extract(druid[druid.index('class spell_dru_innervate'):],'void CalculateAmount(')
    tick=extract((ROOT/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text(),'void AuraEffect::HandlePeriodicEnergizeAuraTick(')
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@AMOUNT@',amount).replace('@TICK@',tick)
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    data=Path('/data/dbc/Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',data)
    row=next(struct.unpack_from('<234I',data,20+i*z) for i in range(n) if struct.unpack_from('<I',data,20+i*z)[0]==29166)
    assert row[80]+1==150 and row[98]==1000 and row[95]==24 and row[110]==0 and row[29]==120000
    dur=Path('/data/dbc/SpellDuration.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',dur)
    duration=next(struct.unpack_from('<4i',dur,20+i*z) for i in range(n) if struct.unpack_from('<I',dur,20+i*z)[0]==row[40])
    assert duration[1]==10000
    print('PASS Innervate actual amount/tick functions; 150 percent caster base mana over ten ticks, target eligibility, power clamp adapter and threat based on actual gain; 120-second record')
if __name__=='__main__':main()
