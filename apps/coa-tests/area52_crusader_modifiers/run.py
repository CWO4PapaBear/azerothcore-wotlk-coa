from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]

def extract(source, signature):
    start = source.index(signature)
    end = source.index("{", start) + 1
    depth = 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]

def signed(value):
    return value if value < 2**31 else value - 2**32

def main():
    source = (ROOT / "src/server/game/Entities/Player/Player.cpp").read_text()
    source = source[source.index("void Player::ApplyTargetedSpellModTrackingDamage("):]
    start = source.index("    if (op == SPELLMOD_CASTING_TIME || op == SPELLMOD_DURATION)")
    end = source[start:source.index("\n}", start)]
    code = Path(__file__).with_name("harness.cpp").read_text()
    code = code.replace("@LAMBDA@", extract(source, "auto calculateSpellMod =") + ";").replace("@END@", end)
    data = Path("/data/dbc/Spell.dbc").read_bytes()
    magic, count, fields, size, _ = struct.unpack_from("<4s4I", data)
    assert magic == b"WDBC" and fields == 234 and size == 936
    records = {struct.unpack_from("<I", data, 20 + i * size)[0]:
        struct.unpack_from("<234I", data, 20 + i * size) for i in range(count)}
    tests = []
    contracts = [
        (920224, 0, 0, 5, (20424, 278746)),
        (932043, 0, 22, 2, (26573, 954818)),
        (932043, 1, 0, 2, (879, 35395, 53385)),
        (965395, 0, 7, 5, (53385,)),
        (965395, 1, 12, 15, (53385,)),
    ]
    for enchant, effect, op, amount, targets in contracts:
        row = records[enchant]
        aura = 107 if op == 7 else 108
        assert row[71 + effect] == 6 and row[95 + effect] == aura and row[110 + effect] == op
        assert signed(row[80 + effect]) + row[74 + effect] == amount and row[49] == 3
        mask = row[122 + effect * 3:125 + effect * 3]
        for target in targets:
            target_row = records[target]
            assert row[208] == target_row[208] and any(a & b for a, b in zip(mask, target_row[209:212])), (enchant, target, mask, target_row[209:212])
        assert not (row[208] == records[133][208] and any(a & b for a, b in zip(mask, records[133][209:212])))
        for copies in range(4):
            base = 10 if op == 7 else 8 if op == 12 else 100
            expected = base + amount * copies if op == 7 else base * (100 + amount * copies) / 100
            tests.append(f"assert(close(player.calc({op},{base},{{{{{aura},{amount*copies},{enchant},{op}}}}}),{expected}));")
    for copies in range(4):
        tests.append(f"assert(close(player.calc(0,100,{{{{108,{5*copies},920224,0}},{{108,{2*copies},932043,0}}}}),{100*(1+.05*copies)*(1+.02*copies)}));")
    code = code.replace("@TESTS@", "\n".join(tests))
    aura_source = (ROOT / "src/server/game/Entities/Unit/Unit.cpp").read_text()
    multiplier = extract(aura_source, "float Unit::GetTotalAuraMultiplier(AuraType auraType, std::function")
    handler = extract((ROOT / "src/server/game/Spells/Auras/SpellAuraEffects.cpp").read_text(), "void AuraEffect::HandleModTotalPercentStat(")
    stat_code = Path(__file__).with_name("stats.cpp").read_text().replace("@MULTIPLIER@", "int Area52ConsecratedGroundAmount(AuraEffect const* effect, Unit const*){assert(effect->GetId()!=965903);return 0;}\n" + multiplier).replace("@HANDLER@", handler)
    assert records[931866][95:98] == (0,79,0) and records[931866][111] == 2 and records[931866][81] + records[931866][75] == 2 and records[931866][49] == 1
    assert records[920264][95] == 137 and records[920264][110] == 0 and records[920264][80] + records[920264][74] == 1 and records[920264][49] == 3
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        for name, content in (("modifiers",code),("stats",stat_code)):
            cpp = path / (name + ".cpp")
            cpp.write_text(content)
            subprocess.run(["c++", "-std=c++20", str(cpp), "-o", str(path / name)], check=True)
            subprocess.run([str(path / name)], check=True)
    print("PASS: extracted native modifiers for Seals of the Pure, Sanctity of Battle and Vanquishing Storm0..3copies; Crusade Holy aura multiplier; Divine Strength native stat handler/recalculation. Metadata masks are structural evidence; dispatcher/login qualified separately.")

if __name__ == "__main__":
    main()
