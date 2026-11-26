from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/scripts/Spells/spell_rogue.cpp').read_text().split('class spell_rog_killing_spree_aura :',1)[1]
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@PERIODIC@',extract(source,'void HandleEffectPeriodic('))
    aura=(ROOT/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text()
    code=code.replace('@TOTAL@',extract(aura,'int32 AuraEffect::GetTotalTicks(')).replace('@UPDATE@',extract(aura,'void AuraEffect::Update('))
    code=code.replace('@CONFIG@',extract((ROOT/'src/server/coa/Area52WindTalents.inc').read_text(),'void ConfigureArea52WindTalents('))
    import struct
    b=Path('/data/dbc/Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b)
    r=next(struct.unpack_from('<'+str(f)+'I',b,20+i*z) for i in range(n) if struct.unpack_from('<I',b,20+i*z)[0]==51690)
    assert r[98]==500 and r[9]&512 and r[40]==39
    b=Path('/data/dbc/SpellDuration.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b)
    duration=next(struct.unpack_from('<'+str(f)+'I',b,20+i*z) for i in range(n) if struct.unpack_from('<I',b,20+i*z)[0]==39)
    assert duration[1]==2000
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS native five-assault periodic scheduling, 500ms boundary, invalid/stealth/invisible target cleanup, teleport-before-hit and Area52-only offhand suppression metadata; no gameplay')
if __name__=='__main__':main()
