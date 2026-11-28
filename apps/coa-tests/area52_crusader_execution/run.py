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
    source = (ROOT / "src/server/coa/AscensionHeroPaladin.cpp").read_text()
    ranks = source[source.index("struct ExecutionSentenceRank"):source.index("constexpr int32 FANATICISM_PCT")]
    active = block(source.split("class spell_hero_paladin_execution_sentence :", 1)[1], "void HandleDummy(")
    removal = block(source.split("class aura_hero_paladin_execution_sentence_burst :", 1)[1], "void HandleRemove(")
    code = Path(__file__).with_name("harness.cpp").read_text().replace("@RANKS@", ranks).replace("@ACTIVE@", active).replace("@REMOVAL@", removal)
    with tempfile.TemporaryDirectory() as temporary:
        cpp = Path(temporary) / "check.cpp"
        cpp.write_text(code)
        executable = Path(temporary) / "check"
        subprocess.run(["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print("PASS extracted Execution Sentence rank lookup, all seven ally/enemy routes and fourteen burst expiry/dispel callbacks with cleanup exclusions")


if __name__ == "__main__":
    main()
