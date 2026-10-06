import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]


def block(text, marker):
    start = text.index(marker)
    opening = text.index('{', start)
    end, depth = opening + 1, 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def main():
    source = (ROOT / 'src/server/coa/AscensionMysticKnightEpics.cpp').read_text()
    code = (Path(__file__).parent / 'harness.cpp').read_text()
    for tag, marker in [('REPLACEMENT', 'void Replacement('), ('SYNCHRONIZE', 'void Synchronize('),
                        ('CONTRACT', 'void OnLoadSpellCustomAttr('), ('DURATION', 'int32 DeconstructionDuration(')]:
        code = code.replace(tag, block(source, marker).replace(' override', ''))
    hook = source[source.index('class spell_area52_flesh_hook'):]
    decon = source[source.index('class spell_area52_deconstruct'):]
    for tag, text, marker in [('HOOK_CHECK', hook, 'SpellCastResult Check('),
                              ('HOOK_DAMAGE', hook, 'void Damage('),
                              ('DECON_DAMAGE', decon, 'void Damage('), ('DECON_HIT', decon, 'void Hit(')]:
        code = code.replace(tag, block(text, marker))
    compiler = shutil.which(os.environ.get('CXX', 'c++'))
    assert compiler
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        (path / 'test.cpp').write_text(code)
        subprocess.run([compiler, '-std=c++20', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
        subprocess.run([str(path / 'test')], check=True)
    assert 'GetUnitOwner() && Applies(GetUnitOwner()->ToPlayer())' in source
    print('PASS: rank replacement, temporary ownership, removal, realm gate, Holy Ground conflict, coefficient isolation')


if __name__ == '__main__':
    main()
