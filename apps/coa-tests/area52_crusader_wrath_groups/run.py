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
    source = (ROOT / "src/server/game/Spells/SpellMgr.cpp").read_text()
    policy = block(source, "bool SpellMgr::IsEffectInSameEffectStackGroup(")
    admission = block(source, "bool SpellMgr::AddSameEffectStackRuleSpellGroups(")
    code = Path(__file__).with_name("harness.cpp").read_text().replace("@POLICY@", policy).replace("@ADMISSION@", admission)
    with tempfile.TemporaryDirectory() as temporary:
        cpp = Path(temporary) / "check.cpp"
        cpp.write_text(code)
        executable = Path(temporary) / "check"
        subprocess.run(["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print("PASS native strongest-effect group admission: Avenging Wrath and seven group members, insertion orders, unrelated aura and distinct raid personal effects")


if __name__ == "__main__":
    main()
