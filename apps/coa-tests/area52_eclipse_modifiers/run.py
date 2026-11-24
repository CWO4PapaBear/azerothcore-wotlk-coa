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
    for id,pct,seconds in ((984417,7,1),(984418,14,2),(984419,20,3)):
        row=r[id];assert row[95:97]==(108,107) and row[110:112]==(0,11)
        assert row[80]+1==pct and row[81]-2**32+1==-1000*seconds
        for target in (954800,48505):
            assert row[208]==r[target][208] and any(a&b for a,b in zip(row[122:125],r[target][209:212]))
        assert row[125:128]==(0,0,524288)
        tests.append('assert(close(player.calc(0,100,{{108,'+str(pct)+','+str(id)+',0}}),'+str(100+pct)+'));')
        tests.append('assert(close(player.calc(11,15000,{{107,'+str(-1000*seconds)+','+str(id)+',11}}),'+str(15000-1000*seconds)+'));')
    for copies in range(4):
        for id,op,amount,base in ((916821,0,4,100),(916821,7,4,10),(916896,0,2,100),(935363,7,2,10)):
            row=r[id];effects=[i for i in range(3) if row[110+i]==op and row[95+i] in (107,108)];assert effects
            i=effects[0];assert row[80+i]+1==amount
            expected=base+copies*amount
            tests.append('assert(close(player.calc('+str(op)+','+str(base)+',{{'+str(row[95+i])+','+str(copies*amount)+','+str(id)+','+str(op)+'}}),'+str(expected)+'));')
    code=code.replace('@TESTS@',''.join(tests))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
    print('PASS complete native modifier lambda and finalizer: Starweaver 7/14/20 percent and 1/2/3-second cooldown reductions; preserved zero-to-three-copy damage and crit enchants')
if __name__=='__main__':main()
