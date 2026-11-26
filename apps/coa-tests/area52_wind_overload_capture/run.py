from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    effects=(ROOT/'src/server/game/Spells/SpellEffects.cpp').read_text()
    method=extract(effects,'void Spell::EffectSchoolDMG(')
    a=method.index('            if (Player* player = m_originalCaster->ToPlayer(); AscensionFreePick::Applies(player)')
    b=method.index('m_damageBeforeTakenMods += damage;',a)+len('m_damageBeforeTakenMods += damage;')
    capture=method[a:b]
    assert method.index('damage = m_originalCaster->SpellDamageBonusDone',0,a)<a<method.index('damage = unitTarget->SpellDamageBonusTaken',a)
    source=(ROOT/'src/server/game/Spells/Spell.cpp').read_text()
    method=extract(source,'void Spell::DoAllEffectOnLaunchTarget(')
    a=method.index('                if (Player* player = m_originalCaster ? m_originalCaster->ToPlayer() : nullptr;')
    b=method.index('m_damageBeforeTakenMods = int32(m_damageBeforeTakenMods * m_damageMultipliers[i]);',a)+len('m_damageBeforeTakenMods = int32(m_damageBeforeTakenMods * m_damageMultipliers[i]);')
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@CAPTURE@',capture).replace('@BOUNCE@',method[a:b])
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual damage snapshot and bounce arithmetic: LB/CL/replacements capture after caster scaling and before target modifiers; unrelated spells/realms remain unchanged')
if __name__=='__main__':main()
