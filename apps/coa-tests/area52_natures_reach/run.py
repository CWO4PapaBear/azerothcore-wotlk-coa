from pathlib import Path
import struct, subprocess, tempfile
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
    source = (ROOT / 'src/server/game/Entities/Player/Player.cpp').read_text()
    eligibility = extract(source, 'if (mod->spellId == 17322 ||')
    aggregate = extract(source, 'if (op == SPELLMOD_RANGE && AscensionFreePick::Applies(this))')
    code = Path(__file__).with_name('harness.cpp').read_text()
    code = code.replace('ELIGIBILITY', eligibility).replace('AGGREGATE', aggregate)
    druid = (ROOT / 'src/server/coa/AscensionHeroDruid.cpp').read_text()
    code = code.replace('SPREAD', extract(druid, 'void HandleProc('))
    with tempfile.TemporaryDirectory() as tmp:
        cpp = Path(tmp) / 'test.cpp'
        cpp.write_text(code)
        exe = Path(tmp) / 'test'
        subprocess.run(['c++', '-std=c++20', str(cpp), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
    data = Path('/data/dbc/Spell.dbc').read_bytes()
    _, count, fields, size, _ = struct.unpack_from('<4s4I', data)
    rows = {struct.unpack_from('<I', data, 20+i*size)[0]: struct.unpack_from('<234I', data, 20+i*size) for i in range(count)}
    for spell, amount in ((16819, 2), (16820, 4), (816819, 6)):
        row = rows[spell]
        assert row[95] == 107 and row[110] == 3 and row[80]+1 == amount
    print('PASS Nature Reach native eligibility, 2/4/6 yards, strongest shared range, caster-owned spread and realm isolation')
if __name__ == '__main__':
    main()
