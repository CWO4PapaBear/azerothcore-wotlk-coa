from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    s=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text()
    a=s.index('uint32 Unit::SpellDamageBonusTaken(Unit* caster, SpellInfo const* spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack, float*')
    damage=extract(s[a:],'if (Player* player = caster ? caster->ToPlayer() : nullptr;')
    crit=extract(s[s.index('float Unit::SpellTakenCritChance('):],'if (!spellProto->IsPositive() && (schoolMask & SPELL_SCHOOL_MASK_MAGIC) &&')
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@DAMAGE@',damage).replace('@CRIT@',crit)
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual Celestial Bane native blocks: all ranks, own/foreign DOTs, Sunfire inheritance, both schools, strongest vulnerability and removal')
if __name__=='__main__':main()
