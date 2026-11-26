from pathlib import Path
import subprocess,tempfile,re
ROOT=Path(__file__).resolve().parents[3]
def extract(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
def main():
 source=(ROOT/'src/server/game/Spells/Spell.cpp').read_text()
 expr=source.split('bool canEffectTrigger = ',1)[1].split(';',1)[0]
 end=source.index('caster->ToPlayer()->CastItemCombatSpell',source.index('SPELL_ATTR4_SUPPRESS_WEAPON_PROCS'))
 begin=source.rfind('if (',0,end)+4
 weapon=source[begin:end].strip();assert weapon.endswith(')');weapon=weapon[:-1]

 code=Path(__file__).with_name('harness.cpp').read_text().replace('@PROC@',expr).replace('@WEAPON@',weapon).replace('@CONFIG@',extract((ROOT/'src/server/coa/Area52WindTalents.inc').read_text(),'void ConfigureArea52WindTalents('))
 with tempfile.TemporaryDirectory() as tmp:
  cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
 print('PASS native hit caster-proc and weapon-enchant predicates suppress offhand only; original main hand and other-realm metadata retained')
if __name__=='__main__':main()
