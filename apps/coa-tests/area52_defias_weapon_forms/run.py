from pathlib import Path
import subprocess, tempfile
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
    source = (ROOT / 'src/server/coa/AscensionMysticEnchant.cpp').read_text()
    gate = extract(source, 'bool DefiasWeaponReady(')
    unit = (ROOT / 'src/server/game/Entities/Unit/Unit.h').read_text()
    feral = extract(unit, 'bool IsInFeralForm() const')
    storage = (ROOT / 'src/server/game/Entities/Player/PlayerStorage.cpp').read_text()
    weapon = extract(storage, 'Item* Player::GetWeaponForAttack(')
    header = (ROOT / 'src/server/game/Entities/Unit/UnitDefines.h').read_text()
    forms = extract(header, 'enum ShapeshiftForm') + ';'
    code = Path(__file__).with_name('harness.cpp').read_text()
    code = code.replace('FORM_ENUM', forms).replace('FERAL_METHOD', feral).replace('WEAPON_METHOD', weapon).replace('\nGATE\n', '\n' + gate + '\n')
    old = gate.replace('!player->HasAura(997047) ||', '!player->HasAura(997047) || player->GetShapeshiftForm() != FORM_NONE ||').replace('DefiasWeaponReady(', 'BaselineReady(')
    code = code.replace('BASELINE_GATE', old)
    with tempfile.TemporaryDirectory() as tmp:
        cpp = Path(tmp) / 'forms.cpp'
        cpp.write_text(code)
        exe = Path(tmp) / 'forms'
        subprocess.run(['c++', '-std=c++20', str(cpp), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
    assert 'return DefiasWeaponReady(GetCaster()->ToPlayer()) ? SPELL_CAST_OK : SPELL_FAILED_CASTER_AURASTATE;' in source
    assert 'return DefiasWeaponReady(player) && IsDefiasPistol' in source
    assert 'return DefiasWeaponReady(player) && IsDefiasGrape' in source
    print('PASS actual shared gate and native weapon accessor; failing old-form baseline; native feral forms; weapon and ownership preservation. Player inventory adapted; no gameplay performed.')

if __name__ == '__main__':
    main()
