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
    row=r[414003];assert row[95]==107 and row[110]==18 and row[80]+1==1
    mask=row[122:125]
    assert row[208]==r[33757][208] and any(x&y for x,y in zip(mask,r[33757][209:212]))
    assert not(row[208]==r[403][208] and any(x&y for x,y in zip(mask,r[403][209:212])))
    tests=[]
    for stacks in range(11):
        tests.append('assert(close(player.calc(18,20,{{107,'+str(stacks)+',414003,18}}),'+str(20+stacks)+'));')
    code=code.replace('@TESTS@',''.join(tests))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
    print('PASS native flat proc-chance modifier math: zero-to-ten Zephyr stacks add 0..10 percentage points to Windfury only; current native CalcProcChance calls this operation')
if __name__=='__main__':main()
