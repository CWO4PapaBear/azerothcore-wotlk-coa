CLI_DESCRIPTION = """Check the mystic enchant rules the server shares with the client, without a server.

Compiles the mystic enchant rule code against the client DBC set the server loads (--dbc-dir), then checks the
catalog, the realm, class and quality columns, slot validity, the per-quality caps, and the result each apply,
destroy, scroll purchase, inspect and preset request receives.
No database, server build or game client is needed.
"""

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE.parent))
from coa_talent_catalog import STUBS  # noqa: E402
from client_data import dbc_dir  # noqa: E402

MAIN = r"""
#include "AscensionMysticEnchantRules.h"
#include "DBCStores.h"
#include <cstdio>
#include <string>

using namespace AscensionMysticEnchant;

namespace
{
int failures = 0;
void Check(bool value, char const* name)
{
    failures += !value;
    std::printf("%s: %s\n", value ? "PASS" : "FAIL", name);
}

constexpr std::uint32_t WARRIOR = 1;
constexpr std::uint32_t COA_CLASS = 12;
constexpr std::uint32_t RUNIC_CONTAGION = 288975;
constexpr std::uint32_t RUNIC_CONTAGION_SCROLL = 404438;
constexpr std::uint32_t CREMATION_SCROLL = 404764;
constexpr std::uint32_t HOLY_POWER_SCROLL = 1175658;
constexpr std::uint32_t RARE_SCROLLS[] = { 404438, 404440, 404441, 404442, 405207, 405213 };

Character Hero(ClientConfig const& config, std::uint32_t level)
{
    Character character;
    character.Class = HERO_CLASS;
    character.Level = level;
    character.Money = 100000;
    character.AltarNearby = true;
    character.UnlockTokens = 1;
    character.RealmGates = { false, true, false, false, false };
    character.Config = &config;
    character.StackLimit = [](std::uint32_t) { return 1u; };
    return character;
}

StagedApply Stage(std::uint32_t slot, std::uint32_t item, std::uint32_t key)
{
    return { slot, item, key, true };
}

std::vector<StagedApply> RareScrolls(std::size_t count)
{
    std::vector<StagedApply> staged;
    for (std::size_t index = 0; index < count; ++index)
        staged.push_back(Stage(std::uint32_t(index + 1), RARE_SCROLLS[index], std::uint32_t(index + 1)));
    return staged;
}
}

int main(int, char** argv)
{
    DbcDirectory = std::string(argv[1]) + "/";
    Catalog catalog;
    Check(LoadCatalog(catalog), "MysticEnchant.dbc loads");
    std::size_t seasonal = 0;
    for (Enchant const& enchant : catalog.Rows)
        seasonal += enchant.Realms[1];
    Check(catalog.Rows.size() > 7000 && seasonal == 1560, "the catalog holds the 1,560 seasonal enchants");

    Enchant const* contagion = catalog.FindSpell(RUNIC_CONTAGION);
    Check(contagion && catalog.FindItem(RUNIC_CONTAGION_SCROLL) == contagion, "an enchant is found by spell and item");
    if (!contagion)
        return 1;
    Check(!catalog.FindItem(UNTARNISHED_MYSTIC_SCROLL), "the untarnished scroll carries no enchant");
    Check(QualityOf(*contagion, HERO_CLASS) == QUALITY_RARE && QualityOf(*contagion, WARRIOR) == QUALITY_POOR,
        "a stock class reads the reborn quality column");
    Check(ClassAllowed(*contagion, HERO_CLASS) && !ClassAllowed(*contagion, WARRIOR) && !ClassAllowed(*contagion, 0),
        "the class mask admits the Hero only");
    Enchant const* holyPower = catalog.FindItem(HOLY_POWER_SCROLL);
    Check(holyPower && !RealmAllows(*holyPower, { false, true, false, false, false }) &&
        RealmAllows(*holyPower, { true, false, false, false, false }), "a live-only enchant is refused on a seasonal realm");

    Check(!SlotValid(0, true) && SlotValid(1, true) && SlotValid(15, true) && !SlotValid(16, true),
        "class fusion uses slots 2 to 16 only");
    Check(SlotValid(0, false) && SlotValid(16, false) && !SlotValid(17, false), "otherwise all 17 slots exist");

    ClientConfig plain;
    Character hero = Hero(plain, 80);
    Character warrior = hero;
    warrior.Class = WARRIOR;
    Check(QualityCap(hero, QUALITY_UNCOMMON) == 17 && QualityCap(hero, QUALITY_RARE) == 17 &&
        QualityCap(hero, QUALITY_EPIC) == 3 && QualityCap(hero, QUALITY_LEGENDARY) == 1 &&
        QualityCap(warrior, QUALITY_EPIC) == 5 && QualityCap(hero, QUALITY_POOR) == 0,
        "without configs the caps are the client's defaults");

    ClientConfig fusion;
    fusion.Booleans["CONFIG_CLASS_FUSION_ENABLED"] = true;
    for (int index = 1; index <= 5; ++index)
        fusion.Integers["CONFIG_RARE_RANDOM_ENCHANT_UNLOCK_LEVEL_" + std::to_string(index)] = index * 10;
    Check(Hero(fusion, 80).Fusion() && Hero(fusion, 35).Fusion(), "a Hero on a class fusion realm uses fusion");
    Check(QualityCap(Hero(fusion, 35), QUALITY_RARE) == 3 && QualityCap(Hero(fusion, 80), QUALITY_RARE) == 5 &&
        QualityCap(Hero(fusion, 80), QUALITY_EPIC) == 4, "fusion unlocks slots by level, up to 6/5/4");
    Character wildcard = Hero(fusion, 80);
    wildcard.GameModes = WILDCARD_MODE;
    Check(!wildcard.Fusion(), "the wildcard game mode is not fusion");

    Slots empty{};
    Character fused = Hero(fusion, 80);
    Check(CheckSaveApply(catalog, fused, empty, RareScrolls(5)) == APPLY_OK, "five rare scrolls fit a fusion Hero");
    Check(CheckSaveApply(catalog, fused, empty, RareScrolls(6)) == APPLY_RARE_LIMIT,
        "a sixth rare scroll is RARE_LIMIT");
    Check(CheckSaveApply(catalog, fused, empty, {}) == APPLY_UNKNOWN, "an empty apply is UNKNOWN");
    std::vector<StagedApply> const twice = { Stage(1, RUNIC_CONTAGION_SCROLL, 7), Stage(2, RUNIC_CONTAGION_SCROLL, 7) };
    Check(CheckSaveApply(catalog, fused, empty, twice) == APPLY_STACK_LIMIT &&
        CheckApplySlot(catalog, fused, empty, twice, twice[0]).back() == APPLY_DUPLICATE_BAG_SLOT_PAIR,
        "one scroll staged into two slots also reports DUPLICATE_BAG_SLOT_PAIR");
    Check(CheckSaveApply(catalog, fused, empty, { Stage(1, RUNIC_CONTAGION_SCROLL, 7),
        Stage(2, RUNIC_CONTAGION_SCROLL, 8) }) == APPLY_STACK_LIMIT, "an enchant twice past its stacks is STACK_LIMIT");
    Check(CheckSaveApply(catalog, fused, empty, { Stage(1, RARE_SCROLLS[0], 1), Stage(1, RARE_SCROLLS[1], 2) }) ==
        APPLY_DUPLICATE_SLOT, "fusion refuses one slot staged twice");
    Slots holding{};
    holding[1] = RUNIC_CONTAGION;
    Check(CheckSaveApply(catalog, fused, holding, { Stage(2, RUNIC_CONTAGION_SCROLL, 7) }) == APPLY_STACK_LIMIT,
        "an enchant already slotted counts toward its stacks");
    Check(CheckSaveApply(catalog, fused, empty, { Stage(0, RUNIC_CONTAGION_SCROLL, 7) }) == APPLY_BAD_SLOT,
        "fusion refuses slot 1");
    Check(CheckSaveApply(catalog, Hero(fusion, 30), empty, { Stage(1, CREMATION_SCROLL, 7) }) ==
        APPLY_TOO_LOW_LEVEL, "a level 35 enchant is refused at level 30");
    Check(CheckSaveApply(catalog, wildcard, empty, { Stage(1, RUNIC_CONTAGION_SCROLL, 7) }) ==
        APPLY_DISABLED_IN_WILDCARD, "the wildcard game mode refuses mystic enchanting");
    Check(CheckSaveApply(catalog, fused, empty, { Stage(1, UNTARNISHED_MYSTIC_SCROLL, 7) }) ==
        APPLY_NOT_MYSTIC_SCROLL, "an untarnished scroll cannot be applied");
    Check(CheckSaveApply(catalog, fused, empty, { { 1, 0, 7, false } }) == APPLY_BAD_ITEM,
        "a scroll the character does not hold is BAD_ITEM");
    Check(CheckSaveApply(catalog, fused, empty, { Stage(1, HOLY_POWER_SCROLL, 7) }) == APPLY_BAD_REALM,
        "a live-only enchant is BAD_REALM on a seasonal realm");
    Character coa = hero;
    coa.Class = COA_CLASS;
    Check(CheckSaveApply(catalog, coa, empty, { Stage(1, RUNIC_CONTAGION_SCROLL, 7) }) == APPLY_BAD_CLASS,
        "a class outside the mask is BAD_CLASS");
    Character casting = fused;
    casting.Casting = true;
    Check(CheckSaveApply(catalog, casting, empty, { Stage(1, RUNIC_CONTAGION_SCROLL, 7) }) ==
        APPLY_NOT_WHILE_CASTING, "nothing is applied mid-cast");

    Enchant gated;
    gated.ClassTypes = { 1, 3, 1 };
    gated.Tabs = { 1, 6, 1 };
    gated.RequiredAE = { 0, 9, 0 };
    Character investor = hero;
    Check(InvestmentMet(investor, *contagion, false), "an enchant with no requirement needs no investment");
    Check(!InvestmentMet(investor, gated, false), "an AE requirement refuses an empty build");
    investor.Invested = [](std::uint32_t classType, std::uint32_t tab, bool talent)
    {
        return classType == 3 && tab == 6 && !talent ? 9u : 0u;
    };
    Check(InvestmentMet(investor, gated, false) && InvestmentMet(investor, gated, true),
        "nine AE in the named tab meets it, and no TE is asked for");

    Check(CheckDestroy(fused, empty, 1) == DESTROY_NO_ENCHANT_APPLIED, "an empty slot has nothing to destroy");
    Check(CheckDestroy(fused, holding, 0) == DESTROY_BAD_SLOT, "fusion slot 1 does not exist to destroy");
    Check(CheckDestroy(casting, holding, 1) == DESTROY_NOT_WHILE_CASTING && CheckDestroy(fused, holding, 1) ==
        DESTROY_OK, "a held enchant is destroyed when not casting");
    Character coaHolder = fused;
    coaHolder.Class = COA_CLASS;
    Check(CheckDestroy(coaHolder, holding, 1) == DESTROY_BAD_CLASS, "a Conquest of Azeroth class may not destroy");

    Character shopper = hero;
    Check(CheckPurchaseScroll(shopper, true, 1000) == PURCHASE_OK, "a scroll is bought at an altar");
    Check(CheckPurchaseScroll(shopper, false, 1000) == PURCHASE_ITEM_NOT_FOUND, "a missing scroll item is reported");
    shopper.Money = 999;
    Check(CheckPurchaseScroll(shopper, true, 1000) == PURCHASE_NOT_ENOUGH_MONEY, "a short purse is NOT_ENOUGH_MONEY");
    shopper.Money = 1000;
    shopper.AltarNearby = false;
    Check(CheckPurchaseScroll(shopper, true, 1000) == PURCHASE_NO_MYSTIC_ALTAR, "scrolls are sold only at an altar");
    shopper.AltarNearby = true;
    shopper.BagSpace = false;
    Check(CheckPurchaseScroll(shopper, true, 1000) == PURCHASE_NOT_ENOUGH_SPACE, "full bags are NOT_ENOUGH_SPACE");

    Check(CheckInspect(false, false, HERO_CLASS) == INSPECT_PLAYER_NOT_FOUND &&
        CheckInspect(true, false, HERO_CLASS) == INSPECT_PLAYER_NOT_IN_MAP &&
        CheckInspect(true, true, COA_CLASS) == INSPECT_BAD_CLASS && CheckInspect(true, true, HERO_CLASS) == INSPECT_OK,
        "inspect needs a present non-CoA player on the same map");

    Check(CheckPresetUnlock(hero, 1) == PRESET_UNLOCK_OK && CheckPresetUnlock(hero, MAX_PRESETS) ==
        PRESET_UNLOCK_MAX_VALUE_REACHED, "presets unlock up to 100");
    Character tokenless = hero;
    tokenless.UnlockTokens = 0;
    Check(CheckPresetUnlock(tokenless, 1) == PRESET_UNLOCK_NO_MONEY, "an unlock needs its token");
    Check(CheckPresetActivate(hero, 1, 2) == PRESET_SET_ACTIVE_OK && CheckPresetActivate(hero, 2, 2) ==
        PRESET_SET_ACTIVE_UNKNOWN, "only an unlocked preset activates");
    Check(CheckPresetActivate(casting, 0, 1) == PRESET_SET_ACTIVE_NOT_WHILE_CASTING &&
        CheckPresetSave(coa) == PRESET_SAVE_BAD_CLASS, "casting and CoA classes are refused");

    return failures ? 1 : 0;
}
"""


def main():
    parser = argparse.ArgumentParser(description=CLI_DESCRIPTION)
    parser.add_argument("--dbc-dir", type=Path)
    args = parser.parse_args()
    args.dbc_dir = args.dbc_dir or dbc_dir()

    compiler = shutil.which(os.environ.get("CXX", "cl.exe" if os.name == "nt" else "c++"))
    assert compiler, "Enable a C++20 compiler (VS Developer PowerShell on Windows)."
    with tempfile.TemporaryDirectory(prefix="coa-mystic-enchant-rules-") as directory:
        out = Path(directory)
        for name, text in STUBS.items():
            (out / name).write_text(text, encoding="utf-8")
        (out / "main.cpp").write_text(MAIN, encoding="utf-8")
        includes = [out, ROOT / "src/server/coa", ROOT / "src/server/shared/DataStores", ROOT / "src/common"]
        sources = [out / "main.cpp", ROOT / "src/server/coa/AscensionMysticEnchantRules.cpp",
                   ROOT / "src/server/shared/DataStores/ClientDBC.cpp"]
        executable = out / ("rules.exe" if os.name == "nt" else "rules")
        if Path(compiler).stem.lower() == "cl":
            flags = ["/nologo", "/std:c++20", "/EHsc", "/utf-8", "/D_CRT_SECURE_NO_WARNINGS",
                     *["/I" + str(p) for p in includes], *map(str, sources), "/Fe" + str(executable)]
        else:
            flags = ["-std=c++20", "-Wall", "-Wextra", *["-I" + str(p) for p in includes], *map(str, sources),
                     "-o", str(executable)]
        build = subprocess.run([compiler, *flags], cwd=out, capture_output=True, text=True, errors="replace")
        if build.returncode:
            raise SystemExit("Mystic enchant rules harness did not compile:\n" + build.stdout + build.stderr)
        raise SystemExit(subprocess.run([str(executable), str(args.dbc_dir.resolve())], text=True).returncode)


if __name__ == "__main__":
    main()
