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
    tests=[]
    cases=((929721,0,4,(1680,),3),(948488,0,5,(16914,50516,983982,983983,983984),3),(956927,0,3,(1680,23881,1464,34428,954836,954837,954838,954876,954877),3),(977725,1,40,(50516,61391,61390,61388,61387,53227),1))
    for id,index,amount,targets,limit in cases:
        row=r[id];assert row[71+index]==6 and row[95+index]==108 and row[110+index]==0 and row[80+index]+1==amount and row[49]==limit
        mask=row[122+index*3:125+index*3]
        for target in targets:
            assert row[208]==r[target][208] and any(x&y for x,y in zip(mask,r[target][209:212])),(id,target)
        assert not (row[208]==r[403][208] and any(x&y for x,y in zip(mask,r[403][209:212])))
        for copies in range(limit+1):
            tests.append('assert(close(player.calc(0,100,{{108,'+str(copies*amount)+','+str(id)+','+str(index)+'}}),'+str(100+copies*amount)+'));')
    code=code.replace('@TESTS@',''.join(tests))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
    print('PASS native modifier arithmetic and effective records: Improved Whirlwind 4%, Gale Winds 5%, Unending Fury 3%, zero-to-three copies, relevant families and unrelated spell exclusion')
if __name__=='__main__':main()
