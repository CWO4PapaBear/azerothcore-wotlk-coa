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
    paladin = (ROOT / "src/server/scripts/Spells/spell_paladin.cpp").read_text()
    immunity = paladin.split("class spell_pal_immunities :", 1)[1]
    banish = (ROOT / "src/server/scripts/Spells/spell_warlock.cpp").read_text().split("class spell_warl_banish :", 1)[1]
    seal = paladin.split("class spell_pal_seal_of_command_aura :", 1)[1]
    blinding = (ROOT / "src/server/coa/Area52CrusaderBlinding.inc").read_text()
    substitutions = {
        "@CHECK@": block(immunity,"SpellCastResult CheckCast("),
        "@DEBUFFS@": block(immunity,"void TriggerDebuffs("),
        "@BANISH@": block(banish,"void HandleBanish("),
        "@SEAL_CHECK@": block(seal,"bool CheckProc("),
        "@SEAL_PROC@": block(seal,"void HandleProc("),
        "@BLINDING@": "\n".join(block(blinding,name) for name in ["bool Area52Caster(","bool Cleric(","void HandleControl(","void HandleHeal("])
    }
    code = Path(__file__).with_name("harness.cpp").read_text()
    for key,value in substitutions.items():
        code = code.replace(key,value)
    with tempfile.TemporaryDirectory() as temporary:
        cpp = Path(temporary) / "check.cpp"
        cpp.write_text(code)
        executable = Path(temporary) / "check"
        subprocess.run(["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print("PASS native immunity restrictions/debuffs, Banish caster-owned cancellation, Seal proc/cleave routing and Blinding Light enchant/primary-stat admission")


if __name__ == "__main__":
    main()
