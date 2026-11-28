from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]


def body(source, signature):
    start = source.index(signature)
    cursor = source.index("{", start) + 1
    depth = 1
    while depth:
        depth += (source[cursor] == "{") - (source[cursor] == "}")
        cursor += 1
    return source[start:cursor]


def main():
    priest = (ROOT / "src/server/coa/AscensionHeroPriest.cpp").read_text()
    unit = (ROOT / "src/server/game/Entities/Unit/Unit.cpp").read_text()
    metadata = body(priest, "void ApplyArea52CrusaderAbilityContracts(")
    healing = body(unit, "[caster, spellProto](AuraEffect const* aurEff)\n        {\n            if (aurEff->GetId() == 271315")
    assert "ApplyArea52CrusaderAbilityContracts(spellInfo, Area52MysticRealm::UsesFreePickEngine());" in priest
    code = Path(__file__).with_name("harness.cpp").read_text().replace("@METADATA@", metadata).replace("@HEALING@", healing)
    with tempfile.TemporaryDirectory() as temporary:
        cpp = Path(temporary) / "check.cpp"
        cpp.write_text(code)
        executable = Path(temporary) / "check"
        subprocess.run(["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", "-I", str(ROOT / "src/server/coa"), str(cpp), "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print("PASS extracted metadata and healing predicates: shield schools, five Halo ranks, caster ownership, Holy school, raid target metadata, repeated initialization and unaffected mechanics")


if __name__ == "__main__":
    main()
