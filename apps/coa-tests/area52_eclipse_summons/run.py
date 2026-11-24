from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def rows(name):
    b=(Path('/data/dbc')/(name+'.dbc')).read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b)
    return {struct.unpack_from('<I',b,20+i*z)[0]:struct.unpack_from('<'+str(z//4)+'I',b,20+i*z) for i in range(n)}
def main():
    source=(ROOT/'src/server/game/Entities/Totem/Totem.cpp').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@INIT@',extract(source,'void Totem::InitSummon(')).replace('@UPDATE@',extract(source,'void Totem::Update('))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
    spells=rows('Spell');properties=rows('SummonProperties');dur=rows('SpellDuration')
    for id,creature in zip((30706,57720,57721,57722),(17539,30652,30653,30654)):
        r=spells[id];assert r[71]==28 and r[110]==creature and r[113]==63 and r[80]+1==5 and dur[r[40]][1]==300000
    assert properties[63][3:5]==(4,1)
    assert spells[55342][117]==58832 and spells[55342][118]==58836 and dur[spells[55342][40]][1]==30000
    assert spells[58832][116:119]==(58833,58831,58834)
    for id in (58833,58831,58834):assert spells[id][71]==28 and spells[id][110]==31216 and spells[id][113]==1021
    assert properties[1021][4]==0
    print('PASS actual Totem initialization and lifetime, both passive buffs on all four ranks, five-health/five-minute summon data; Mirror Image three-image helper graph and slotless properties')
if __name__=='__main__':main()
