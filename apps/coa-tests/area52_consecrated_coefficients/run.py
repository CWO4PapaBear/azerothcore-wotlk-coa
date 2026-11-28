from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[3]
def block(s,start):
 i=s.index('{',start)+1;n=1
 while n:n+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[start:i]
s=(r/'src/server/game/Entities/Unit/Unit.cpp').read_text();start=s.index('    SpellBonusEntry consecratedGroundBonus{};');end=s.index('    if (bonus)',start);override=s[start:end]
a=s.index('        if (damagetype == DOT)',end);dot=block(s,a)
sp=s[s.index('        float contribution = DoneAdvertisedBenefit * coeff * factorMod;',a):s.index('        DoneTotal += int32(contribution);',a)+len('        DoneTotal += int32(contribution);')]
code=Path(__file__).with_name('harness.cpp').read_text().replace('@OVERRIDE@',override).replace('@DOT@',dot).replace('@SP@',sp)
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS extracted native scopedground coefficient override and DOTAP/SP contribution:4percent each,originalplayer ownership,otherrealm/spell/effect/direct isolation and preserved existing coefficients; general resistance/modifier pipeline remains native')
