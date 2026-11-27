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
    def signed(x):return x if x<2**31 else x-2**32
    for id,effect,op,target in ((95276,0,11,21169),(916184,0,11,21169),(916184,1,8,21169),(16184,0,11,21169),(16184,1,8,21169),(16209,0,11,21169),(16209,1,8,21169)):
        row=r[id];other=r[target];assert row[71+effect]==6 and row[95+effect]==107 and row[110+effect]==op
        assert row[208]==other[208] and any(x&y for x,y in zip(row[122+effect*3:125+effect*3],other[209:212]))
    for id,index,amount in ((95276,0,-90000),(916184,0,-60000),(916184,1,5),(16184,0,-600000),(16184,1,20),(16209,0,-1200000),(16209,1,40)):
        assert signed(r[id][80+index])+signed(r[id][74+index])==amount
    assert r[21169][29]==1800000
    for rank,bonus,cd in ((0,0,0),(16184,20,-600000),(16209,40,-1200000)):
        for hasty in range(4):
            for improved in range(4):
                mods=[(107,-90000*hasty,95276,11),(107,-60000*improved,916184,11)]
                if rank:mods.append((107,cd,rank,11))
                def literal(values):return '{'+','.join('{'+','.join(map(str,v))+'}' for v in values)+'}'
                expected=1800000+cd-90000*hasty-60000*improved
                tests.append('assert(close(player.calc(11,1800000,'+literal(mods)+'),'+str(expected)+'));')
                mods=[(107,improved*5,916184,8)]
                if rank:mods.append((107,bonus,rank,8))
                tests.append('assert(close(player.calc(8,20,'+literal(mods)+'),'+str(20+bonus+improved*5)+'));')
    for enchant,target,operation,amount in [(946908,12292,11,-10),(956927,1464,0,3),(956927,23881,0,3),(956927,34428,0,3),(956927,1680,0,3),(964332,1464,13,-10)]:
        row=r[enchant];other=r[target];assert row[95]==108 and row[110]==operation
        assert signed(row[80])+row[74]==amount
        assert row[208]==other[208] and any(x&y for x,y in zip(row[122:125],other[209:212]))
        for copies in range(1,4 if enchant==956927 else 2):
            tests.append('assert(close(player.calc('+str(operation)+',100,{{108,'+str(amount*copies)+','+str(enchant)+','+str(operation)+'}}),'+str(100+amount*copies)+'));')
    for talent in [46908,46909,56924]:
        assert talent in r
        row=r[talent]
        assert row[95]==108 and row[110]==11
        amount=signed(row[80])+row[74]
        tests.append('assert(close(player.calc(11,100,{{108,-10,946908,11},{108,'+str(amount)+','+str(talent)+',11}}),'+str(90+amount)+'));')
    code=code.replace('@TESTS@',''.join(tests))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
    print('PASS actual native spell-modifier arithmetic and DBC family masks: Reincarnation plus 0-2 talent ranks and 0-3 copies of both enchants, 48 combinations, Intensify Rage enchant/talent cooldown stacking, Unending Fury1-3 copies and Battering Ram armor modifier; helper effects verified separately.')
if __name__=='__main__':main()
