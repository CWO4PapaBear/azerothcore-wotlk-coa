from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
with tempfile.TemporaryDirectory() as directory:
    executable = Path(directory) / "cache-floor"
    subprocess.run(["g++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
                    "-I" + str(root / "src/server/coa"),
                    str(Path(__file__).with_name("harness.cpp")), "-o", str(executable)], check=True)
    subprocess.run([str(executable)], check=True)
print("Cache floor: realm tiers, heirloom average, ownership, release gates, fractional bounds and progression passed")

def function(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]

catalog = (root / "src/server/coa/Area52CacheCatalog.inc").read_text()
cap = function(catalog, "uint32 RewardCap(")
requirements = catalog[catalog.index("ItemTemplate rewardRequirements = *item;"):]
requirements = requirements[:requirements.index("auto [set, inserted]")]
storage = (root / "src/server/game/Entities/Player/PlayerStorage.cpp").read_text()
use = function(storage, "InventoryResult Player::CanUseItem(ItemTemplate const* proto) const")
fixture = Path(__file__).with_name("requirements.cpp.in").read_text()
fixture = fixture.replace("@CAP@", cap).replace("@REQUIREMENTS@", requirements).replace("@USE@", use)
with tempfile.TemporaryDirectory() as directory:
    source = Path(directory) / "requirements.cpp"
    executable = Path(directory) / "requirements"
    source.write_text(fixture)
    subprocess.run(["g++", "-std=c++20", "-Wall", "-Wextra", str(source), "-o", str(executable)], check=True)
    subprocess.run([str(executable)], check=True)
print("Extracted native item requirements: lower-level receipt, unchanged equip gate, faction/class/race/skill/spell/event/script restrictions and Prestige preserved")
