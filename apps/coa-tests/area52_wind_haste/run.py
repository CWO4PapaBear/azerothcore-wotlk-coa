from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@HASTE@',extract(source,'void AuraEffect::HandleModMeleeSpeedPct('))
    code=code.replace('@TIMER@',extract((ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text(),'void Unit::ApplyAttackTimePercentMod('))
    import struct
    b=Path('/data/dbc/Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b);rows={r[0]:r for r in (struct.unpack_from('<'+str(f)+'I',b,20+i*z) for i in range(n))}
    for id,percent in ((16257,4),(16277,6),(16278,10)):
        r=rows[id];assert r[95]==138 and r[80]+1==percent and r[36]==3
    r=rows[414003];assert r[96]==138 and r[81]+1==1
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS native melee-haste apply/remove and attack timer fraction preservation for three Flurry ranks and Zephyr stacks; ranged unaffected; record-derived amounts')
if __name__=='__main__':main()
