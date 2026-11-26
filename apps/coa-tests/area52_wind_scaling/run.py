from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def main():
    source=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text()
    a=source.index('    float coeff = spellProto->Effects[effIndex].BonusMultiplier;',source.index('uint32 Unit::SpellDamageBonusDone('))
    b=source.index('    if (agilityDamage)\n        *agilityDamage *= DoneTotalMod;',a)
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@NATIVE@',source[a:b])
    effects=(ROOT/'src/server/game/Spells/SpellEffects.cpp').read_text()
    a=effects.index('        if (effIndex == EFFECT_0 && AscensionFreePick::Applies(m_caster->ToPlayer()))',effects.index('void Spell::EffectSchoolDMG('))
    b=effects.index('        if (m_originalCaster /*',a)
    code=code.replace('@ADMISSION@',effects[a:b])
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20','-I'+str(ROOT/'src/server/coa'),str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS native SpellDamageBonusDone coefficient consumer for26 strike helpers,14 Desolate ranks,8 Tornado ticks, independent AP/SP, coefficient mods, foreign realm fallback; actual EffectSchoolDMG offhand/Tornado admission')
if __name__=='__main__':main()
