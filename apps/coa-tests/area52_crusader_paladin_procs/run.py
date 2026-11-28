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
    source = (ROOT / "src/server/scripts/Spells/spell_paladin.cpp").read_text()
    wise = block(source.split("class spell_pal_judgements_of_the_wise :", 1)[1], "void HandleProc(")
    vengeance = block(source.split("class spell_pal_righteous_vengeance :", 1)[1], "void HandleProc(")
    effects = (ROOT / "src/server/game/Spells/SpellEffects.cpp").read_text().split("void Spell::EffectEnergize(", 1)[1]
    mana = effects[effects.index("    int level_multiplier = 0;"):effects.index("    if (level_diff > 0)")]
    code = Path(__file__).with_name("harness.cpp").read_text().replace("@WISE@", wise).replace("@VENGEANCE@", vengeance).replace("@MANA@", mana)
    with tempfile.TemporaryDirectory() as temporary:
        cpp = Path(temporary) / "check.cpp"
        cpp.write_text(code)
        executable = Path(temporary) / "check"
        subprocess.run(["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print("PASS extracted Judgements routing/base-mana calculation and Righteous Vengeance amount/ownership with all ranks and primary-stat transitions")


if __name__ == "__main__":
    main()
