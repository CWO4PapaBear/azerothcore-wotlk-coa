from pathlib import Path
import subprocess, tempfile, struct, resource
ROOT = Path(__file__).resolve().parents[3]
def extract(text, signature):
    a = text.index(signature)
    b = text.index('{', a) + 1
    depth = 1
    while depth:
        depth += (text[b] == '{') - (text[b] == '}')
        b += 1
    return text[a:b]
def main():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    source = (ROOT / 'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text()
    cast = (ROOT / 'src/server/game/Spells/Spell.cpp').read_text()
    selection = cast.index('aura->DiscardSuspendedWarriorStanceForForm')
    interruption = cast.index('m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CAST')
    assert selection < interruption
    assert 'if (m_spellInfo->IsSelfCast())' in cast[selection-400:selection]
    handler = extract(source, 'void AuraEffect::HandleAuraModShapeshift(')
    assert handler.index('SuspendWarriorStanceForStealth(target)') < handler.index('target->SetShapeshiftForm(form)')
    assert handler.index('RestoreWarriorStanceAfterStealth(target)') > handler.index('HandleShapeshiftBoosts(target, apply)')
    assert 'if (!apply && (mode & AURA_EFFECT_HANDLE_REAL))' in handler
    removal = handler[handler.index('        if (!target->HasShapeshiftAura())'):handler.index('        if (modelid > 0)\n            target->RestoreDisplayId();')]
    code = Path(__file__).with_name('harness.cpp').read_text()
    for tag, signature in [('SUSPEND', 'void AuraEffect::SuspendWarriorStanceForStealth('), ('RESTORE', 'void AuraEffect::RestoreWarriorStanceAfterStealth(')]:
        code = code.replace(tag, extract(source, signature))
    code = code.replace('DISCARD', extract(source, 'void AuraEffect::DiscardSuspendedWarriorStanceForForm('))
    code = code.replace('REMOVAL', removal)
    info = (ROOT / 'src/server/game/Spells/SpellInfo.cpp').read_text()
    code = code.replace('CHECK_FORM', extract(info, 'SpellCastResult SpellInfo::CheckShapeshift('))
    unit = (ROOT / 'src/server/game/Entities/Unit/Unit.cpp').read_text()
    code = code.replace('SET_FORM', extract(unit, 'void Unit::SetShapeshiftForm('))
    dbc = Path('/data/dbc/Spell.dbc').read_bytes()
    _, count, fields, size, _ = struct.unpack_from('<4s4I', dbc)
    pool = dbc[20 + count * size:]
    rows = []
    for i in range(count):
        row = struct.unpack_from('<' + str(fields) + 'I', dbc, 20 + i * size)
        if row[0] in (2457, 71, 2458):
            assert all(not row[71+j] or row[86+j] == 1 for j in range(3))
            assert 36 in row[95:98]
        offset = row[136]
        name = pool[offset:pool.find(b'\0', offset)].decode(errors='replace')
        if name in ('Pistol Shot', 'Grape Shot'):
            rows.append(row)
    assert rows and any(r[0] == 998110 for r in rows)
    cases = '\n'.join('checkShot(%d,%dULL,%dULL,%d,%d);' % (r[0], r[12] | r[13] << 32, r[14] | r[15] << 32, r[4], r[6]) for r in rows)
    code = code.replace('SHOT_CASES', cases)
    forms = Path('/data/dbc/SpellShapeshiftForm.dbc').read_bytes()
    _, count, fields, size, _ = struct.unpack_from('<4s4I', forms)
    records = [struct.unpack_from('<' + str(fields) + 'I', forms, 20 + i * size) for i in range(count)]
    assert all(not any(r[27:35]) for r in records if r[0] in (17,18,19,30))
    code = code.replace('FORM_CASES', '\n'.join('sSpellShapeshiftFormStore.forms[%d] = {%d};' % (r[0], r[19]) for r in records))

    with tempfile.TemporaryDirectory() as tmp:
        cpp = Path(tmp) / 'test.cpp'
        cpp.write_text(code)
        exe = Path(tmp) / 'test'
        subprocess.run(['c++', '-std=c++20', str(cpp), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
        selection_baseline = code.replace(extract(source, 'void AuraEffect::DiscardSuspendedWarriorStanceForForm('), 'void AuraEffect::DiscardSuspendedWarriorStanceForForm(Unit*,uint32)const {}')
        cpp.write_text(selection_baseline)
        subprocess.run(['c++', '-std=c++20', str(cpp), '-o', str(exe)], check=True)
        result = subprocess.run([str(exe)], capture_output=True)
        assert result.returncode != 0 and b'u.casts==1' in result.stderr
        baseline = code.replace(extract(source, 'void AuraEffect::SuspendWarriorStanceForStealth('), 'void AuraEffect::SuspendWarriorStanceForStealth(Unit*)const {}')
        cpp.write_text(baseline)
        subprocess.run(['c++', '-std=c++20', str(cpp), '-o', str(exe)], check=True)
        result = subprocess.run([str(exe)], capture_output=True)
        assert result.returncode != 0 and b'u.effects.size()==1' in result.stderr

    print('PASS extracted native suspension/restoration and removal branch: all three stances, repeated cycles, zero stance, replacement form, overlapping stealth, death, logout, removed ownership, foreign caster and other-mode isolation.')
    print('PASS native CheckShapeshift for all %d named Pistol/Grape Shot records after every restored stance; no learned spells or action slots changed.' % len(rows))
if __name__ == '__main__':
    main()
