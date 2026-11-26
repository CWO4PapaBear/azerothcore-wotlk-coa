from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    s=(ROOT/'src/server/game/Entities/Player/Player.cpp').read_text();s=s[s.index('void Player::ApplyTargetedSpellModTrackingDamage('):]
    a=s.index('    if (op == SPELLMOD_CASTING_TIME || op == SPELLMOD_DURATION)');end=s[a:s.index('\n}',a)]
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@LAMBDA@',extract(s,'auto calculateSpellMod =')+';').replace('@END@',end)
    b=Path('/data/dbc/Spell.dbc').read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b);r={struct.unpack_from('<I',b,20+i*z)[0]:struct.unpack_from('<234I',b,20+i*z) for i in range(n)}
    row=r[997084];assert row[95]==108 and row[110]==0 and row[80]+1==10 and row[49]==3
    for id in [997083,*range(997085,997098)]:
        assert row[208]==r[id][208] and any(x&y for x,y in zip(row[122:125],r[id][209:212]))
    doom=r[983974];assert doom[95]==107 and doom[110]==18 and doom[80]+1==20
    assert doom[208]==r[33757][208] and any(x&y for x,y in zip(doom[122:125],r[33757][209:212]))
    tests=[]
    for stacks in range(4):
        tests.append('assert(close(player.calc(0,100,{{108,'+str(stacks*10)+',997084,0}}),'+str(100+stacks*10)+'));')
    for bonus in (10,20):
        tests.append('assert(close(player.calc(18,20,{{107,'+str(bonus)+',983974,18}}),'+str(20+bonus)+'));')
    code=code.replace('@TESTS@',''.join(tests))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
    print('PASS actual native spell modifier arithmetic: Desolate 0..3 stacks +0..30% damage on all14 bolt ranks; Doom adds20 or10 percentage points to Windfury only')
if __name__=='__main__':main()
