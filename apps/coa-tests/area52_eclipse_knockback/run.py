from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    eff=(ROOT/'src/server/game/Spells/SpellEffects.cpp').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@KNOCK@',extract(eff,'void Spell::EffectKnockBack(')).replace('@MANA@',extract(eff,'void Spell::EffectEnergizePct('))
    for tag,file,cls in [('TYPHOON','spell_druid.cpp','spell_dru_typhoon'),('THUNDER','spell_shaman.cpp','spell_sha_thunderstorm')]:
        s=(ROOT/'src/server/scripts/Spells'/file).read_text();s=s[s.index('class '+cls):]
        code=code.replace('@'+tag+'@',extract(s,'void HandleKnockBack('))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
    b=Path('/data/dbc/Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b);rows={struct.unpack_from('<I',b,20+i*z)[0]:struct.unpack_from('<234I',b,20+i*z) for i in range(n)}
    for parent,child in zip((50516,53223,53225,53226,61384),(61391,61390,61388,61387,53227)):
        r=rows[parent];c=rows[child];assert r[117]==child and r[72]==64
        assert c[71:74]==(98,2,6) and c[86:89]==(104,104,104) and c[97]==33 and c[110]==150
    for spell in (51490,59156,59158,59159):
        r=rows[spell];assert r[71:74]==(2,137,98) and r[81]+1==10 and r[111]==0 and r[112]==300
    print('PASS native knockback eligibility, interruption, reflection origin and glyph exclusion; Thunderstorm mana eligibility and amount; five Typhoon and four Thunderstorm rank records')
if __name__=='__main__':main()
