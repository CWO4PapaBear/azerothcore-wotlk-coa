from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]


def block(source, signature):
    start = source.index(signature)
    cursor = source.index("{", start) + 1
    depth = 1
    while depth:
        depth += (source[cursor] == "{") - (source[cursor] == "}")
        cursor += 1
    return source[start:cursor]


def main():
    info = (ROOT / "src/server/game/Spells/SpellInfo.cpp").read_text()
    spell = (ROOT / "src/server/game/Spells/Spell.cpp").read_text()
    creatures = block(info,"bool SpellInfo::CheckTargetCreatureType(")
    start = info.index("    if (AscensionFreePick::Enabled() && mod->spellId == 954879")
    end = info.index("    if (AscensionFreePick::Enabled() && !mod->mask)",start)
    rank = info[start:end]
    start = spell.index("    if (m_spellInfo->Speed > 0.0f && m_caster != target)")
    end = spell.index("        targetInfo.timeDelay = 0LL;",start)+len("        targetInfo.timeDelay = 0LL;")
    delay = spell[start:end]
    group = spell.split("case TARGET_CHECK_RAID:")[-1].split("case TARGET_CHECK_CORPSE:")[0]
    group = group[:group.rindex("break;")]
    code = Path(__file__).with_name("harness.cpp").read_text()
    for key,value in {"@CREATURE@":creatures,"@RANK@":rank,"@DELAY@":delay,"@GROUP@":group}.items():
        code = code.replace(key,value)
    with tempfile.TemporaryDirectory() as temporary:
        cpp = Path(temporary) / "check.cpp"
        cpp.write_text(code)
        executable = Path(temporary) / "check"
        subprocess.run(["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", "-I", str(ROOT / "src/server/coa"), str(cpp), "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print("PASS native Halo missile delay and rank modifier admission, party/raid target predicate, and Banish creature-type mask")


if __name__ == "__main__":
    main()
