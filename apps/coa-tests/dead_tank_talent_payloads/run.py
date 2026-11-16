CLI_DESCRIPTION = """Check Area 52 Eye for an Eye rank bindings and Perforating Shots payloads against Spell.dbc.
Also guard against importing the incompatible Frost Presence replacement or overwriting native tank threat.
No database, server build or game client is needed.
"""

import argparse
from pathlib import Path
import re
import struct
import sys

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE.parent))
from client_data import dbc_dir  # noqa: E402

SHAPES = ROOT / "src/server/coa/AscensionStockScriptShapes.h"
PRESENCE = ROOT / "src/server/scripts/Spells/spell_dk.cpp"
PERFORATING = ROOT / "src/server/coa/AscensionPerforatingShots.cpp"
LOADER = ROOT / "src/server/coa/CoAScriptLoader.cpp"
SQL = ROOT / "data/sql/updates/pending_db_world/rev_20261004_10_coa_dead_tank_talent_payloads.sql"

DURATION, EFFECT, BASE, TARGET_A, AURA, MISC, TRIGGER, DESCRIPTION = 40, 71, 80, 86, 95, 110, 116, 170
APPLY_AURA, TARGET_UNIT_CASTER, TARGET_UNIT_TARGET_ENEMY = 6, 1, 6
DUMMY, PERIODIC_DAMAGE, MOD_DAMAGE_PERCENT_TAKEN, MOD_PERCENT_STAT, MOD_BASE_RESISTANCE_PCT = 4, 3, 87, 80, 142
ASCENSION_MOD_IGNORE_ARMOR_PCT = 338
FROST_PRESENCE, FROST_PRESENCE_BONUS, TANK_THREAT = 48263, 48905, 57340
EYE_FOR_AN_EYE = (9799, 25988, 89799)
PERFORATING_SHOTS = {853236: 853239, 853237: 853240, 853238: 853241}
PIERCING_SHOTS = 63468

failures = 0


def check(value, name):
    global failures
    failures += not value
    print(("PASS" if value else "FAIL") + ": " + name)


def load_spells(directory):
    raw = (directory / "Spell.dbc").read_bytes()
    count, fields, size, _ = struct.unpack_from("<IIII", raw, 4)
    strings = raw[20 + count * size:]
    rows = {row[0]: row for row in struct.iter_unpack(f"<{fields}i", raw[20:20 + count * size])}
    return rows, lambda offset: strings[offset:strings.index(b"\0", offset)].decode("utf-8", "replace")


def read(path):
    return path.read_text(encoding="utf-8") if path.exists() else ""


def main():
    parser = argparse.ArgumentParser(description=CLI_DESCRIPTION)
    parser.add_argument("--dbc-dir", type=Path)
    args = parser.parse_args()
    spells, text = load_spells(args.dbc_dir or dbc_dir())

    def slot(spell, index):
        row = spells[spell]
        return row[EFFECT + index], row[AURA + index], row[TARGET_A + index]

    check(slot(TANK_THREAT, 0)[:2] == (APPLY_AURA, 10),
          "Area 52 tank threat already has its native threat aura")
    check("SPELL_DK_FROST_PRESENCE_BONUS_EFFECTS" not in read(PRESENCE),
          "incompatible Frost Presence payload is not imported")

    sql = read(SQL)
    check(all(slot(spell, 0)[:2] == (APPLY_AURA, DUMMY) for spell in EYE_FOR_AN_EYE) and
          [spells[spell][BASE] for spell in EYE_FOR_AN_EYE] == [4, 9, 14],
          "Eye for an Eye has three dummy ranks at 5/10/15%")
    ranks = re.findall(r"\((9799), (\d+), (\d)\)", sql)
    check([(int(spell), int(rank)) for _, spell, rank in ranks] == [(9799, 1), (25988, 2), (89799, 3)],
          "the SQL chains Eye for an Eye's ranks to 9799")

    piercing = spells[PIERCING_SHOTS]
    check(all(slot(talent, 0)[:2] == (APPLY_AURA, DUMMY) and spells[talent][TRIGGER] == tear and
              slot(tear, 0) == (APPLY_AURA, ASCENSION_MOD_IGNORE_ARMOR_PCT, TARGET_UNIT_TARGET_ENEMY) and
              spells[tear][DURATION] == piercing[DURATION] for talent, tear in PERFORATING_SHOTS.items()),
          "each Perforating Shots rank names a target armor tear that lasts as long as Piercing Shots")
    check(slot(PIERCING_SHOTS, 0) == (APPLY_AURA, PERIODIC_DAMAGE, TARGET_UNIT_TARGET_ENEMY),
          "Piercing Shots' first effect is the bleed on the target")
    source = read(PERFORATING)
    check(all(f"= {talent}" in source for talent in PERFORATING_SHOTS) and
          "Effects[EFFECT_0].TriggerSpell" in source and "SPELL_AURA_PERIODIC_DAMAGE" in source and
          "AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK" in source,
          "the script casts the rank's own armor tear each time the bleed lands or refreshes")
    registered = set(re.findall(r"RegisterSpellScript\((\w+)\)", source))
    bound = set(re.findall(r"\(63468, '(\w+)'\)", sql))
    check(registered and registered == bound and "AddSC_AscensionPerforatingShots();" in read(LOADER),
          "the SQL binds Piercing Shots to the registered and loaded script")
    raise SystemExit(1 if failures else 0)


if __name__ == "__main__":
    main()
