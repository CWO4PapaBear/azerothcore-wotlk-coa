/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionFreePick.h"
#include "Area52StarterEquipment.h"
#include "Area52BuildSelection.h"
#include "DBCStores.h"
#include <cstdio>
#include <string_view>

using namespace AscensionFreePick;

namespace
{
int Failures = 0;
void Check(bool pass, char const* name)
{
    std::printf("%s: %s\n", pass ? "PASS" : "FAIL", name);
    Failures += !pass;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
        return 2;
    DbcDirectory = std::string(argv[1]) + "/";
    for (auto race : {1, 2, 3, 4, 5, 6, 7, 8, 10, 11})
    {
        auto const kit = Area52StarterEquipment::Items(race);
        Check(kit[3] == 519243 && kit[9] == 2092 && kit[10] == 39202, "shared chest, dagger and polearm");
        Check(std::count(kit.begin(), kit.end(), 20904) == 0, "Blood Elf mail excluded");
    }
    Catalog data;
    Check(LoadCatalog(data), "load effective catalog");
    for (auto const& [id, spell] : std::map<std::uint32_t, std::uint32_t>{
        {1149, 84864}, {1150, 84865}, {1151, 84866}, {1152, 84867}, {18149, 129243}})
    {
        auto const found = data.Entries.find(id);
        Check(found != data.Entries.end() && found->second.Spells == std::vector<std::uint32_t>{spell},
            "primary path maps to its exact spell");
        Check(bool(Validate(data, 1, {{id, 1}})), "primary path is available at level one");
        if (found != data.Entries.end())
            Check(!found->second.AE && !found->second.TE, "primary path costs no essence");
    }
    Check(!Validate(data, 1, {{1149, 1}, {18149, 1}}), "two primary paths are rejected");
    Check(bool(Validate(data, 1, {{18149, 1}}, {{1149, 1}})), "switch to Duality is valid");
    Check(!data.Entries.count(7), "seasonal Heroic Strike cannot be selected on live");
    Check(!data.Entries.count(6664), "CoA Cultist Eldritch Obelisk is excluded");
    Check(!Validate(data, 80, {{6664,1}}), "forged CoA search selection is refused");
    Check(data.Entries.count(2661) && data.Entries.at(2661).Spells.front() == 78,
        "live Heroic Strike maps to spell 78");
    Check(data.Budgets.at(1).AE == 9 && data.Budgets.at(1).TE == 0, "level one budget");
    Check(data.Budgets.at(9).AE == 9 && data.Budgets.at(9).TE == 0, "level nine budget");
    Check(data.Budgets.at(10).AE == 10 && data.Budgets.at(10).TE == 1, "level ten budget");
    Check(data.Budgets.at(80).AE == 80 && data.Budgets.at(80).TE == 71, "level eighty budget");
    Check(bool(Validate(data, 1, { {2661, 1} })), "learn live Heroic Strike at level one");
    Check(!Validate(data, 1, { {7, 1} }), "reject seasonal entry");
    Check(!Validate(data, 1, { {2661, 1}, {2661, 1} }), "reject duplicate entries");
    Check(!Validate(data, 1, { {2661, 99} }), "reject excessive rank");
    Check(!Validate(data, 1, { {2661, 0} }), "reject zero rank");
    Check(!Validate(data, 1, { {0xffffffff, 1} }), "reject unknown entry");
    Check(!Validate(data, 1, { {2965, 1} }), "reject high level ability");
    Catalog synthetic;
    synthetic.Budgets[1] = {9, 0};
    synthetic.Budgets[10] = {10, 1};
    synthetic.Budgets[15] = {15, 6};
    for (std::uint32_t id = 1; id <= 20; ++id)
    {
        Entry entry;
        entry.Id = id;
        entry.Spells = {100 + id};
        entry.AE = 2;
        synthetic.Entries[id] = entry;
    }
    Check(bool(Validate(synthetic, 1, {{1,1},{2,1},{3,1},{4,1}})), "eight AE fits nine AE");
    Check(!Validate(synthetic, 1, {{1,1},{2,1},{3,1},{4,1},{5,1}}), "ten AE exceeds nine AE");
    auto& talent = synthetic.Entries.at(1);
    talent.Level = 10;
    talent.AE = 0;
    talent.TE = 1;
    talent.Spells = {101,102,103};
    Check(!Validate(synthetic, 9, {{1,1}}), "tier one is unavailable before ten");
    Check(bool(Validate(synthetic, 10, {{1,1}})), "tier one is available at ten");
    Check(!Validate(synthetic, 10, {{1,2}}), "talent ranks each consume TE");
    talent.Level = 15;
    Check(!Validate(synthetic, 14, {{1,1}}), "tier two is unavailable before fifteen");
    Check(bool(Validate(synthetic, 15, {{1,3}})), "tier two ranks work at fifteen");
    synthetic.Entries.at(2).Required[0] = 3;
    Check(!Validate(synthetic, 15, {{2,1}}), "missing prerequisite is refused");
    Check(bool(Validate(synthetic, 15, {{2,1},{3,1}})), "upload order does not bypass prerequisites");
    synthetic.Entries.at(3).Required[0] = 2;
    Check(!Validate(synthetic, 15, {{2,1},{3,1}}), "cyclic prerequisites are refused");
    synthetic.Entries.at(3).Required[0] = 0;
    synthetic.Entries.at(4).Group = 2;
    synthetic.Entries.at(5).Group = 2;
    Check(!Validate(synthetic, 15, {{4,1},{5,1}}), "exclusive group cannot be duplicated");
    synthetic.Entries.at(4).Group = 0;
    synthetic.Entries.at(5).Group = 0;
    for (std::uint32_t quality = 1; quality <= 4; ++quality)
    {
        auto& entry = synthetic.Entries.at(4);
        entry.Quality = quality;
        entry.QualityCost = QualityLimits[quality - 1];
        Check(bool(Validate(synthetic, 15, {{4,1}})), "rarity budget boundary is allowed");
        ++entry.QualityCost;
        Check(!Validate(synthetic, 15, {{4,1}}), "rarity overspend is refused");
    }
    synthetic.Entries.at(4).Masteries[0] = 5;
    Check(bool(Validate(synthetic, 15, {{4,1},{5,1}})), "mastery exempts authored gem cost");
    Check(bool(Validate(synthetic, 15, {})), "empty selection is valid");
    Check(SameSelection({{1,1},{2,2}}, {{2,2},{1,1}}), "reordered selection is a no-op");
    Check(!SameSelection({{1,1},{2,2}}, {{1,1},{2,1}}), "rank change is not a no-op");
    Check(!SameSelection({{1,1},{2,2}}, {{1,1},{1,1}}), "duplicate cannot impersonate selection");
    synthetic.Entries.at(5).Flags |= 0x80000;
    Check(!Validate(synthetic, 15, {{5,1}}), "hidden unowned entry cannot be purchased");
    Check(bool(Validate(synthetic, 15, {{5,1}}, {{5,1}})), "owned hidden entry can be retained");
    synthetic.Entries.at(3).Flags |= 4;
    Check(!Validate(synthetic, 15, {}, {{3,1}}), "protected ability cannot be removed");
    Check(!Validate(synthetic, 15, {{4,1}}, {{4,1},{5,1}}), "retained ability protects its mastery");
    Catalog plans;
    plans.Budgets[1] = {9, 0};
    plans.Budgets[10] = {10, 1};
    plans.Budgets[15] = {15, 6};
    Entry ability;
    ability.Id = 1;
    ability.Spells = {100};
    ability.AE = 3;
    plans.Entries[1] = ability;
    Entry planTalent;
    planTalent.Id = 2;
    planTalent.Spells = {200, 201, 202};
    planTalent.Level = 10;
    planTalent.TE = 1;
    plans.Entries[2] = planTalent;
    Area52Build::Entry plan;
    plan.Spells = {{100, 1}, {200, 10}, {202, 15}};
    std::vector<KnownEntry> selected;
    Check(bool(Area52Build::Select(plans, plan, 1, selected)) && SameSelection(selected, {{1, 1}}),
        "active plan maps spell IDs and delays future talents");
    Check(bool(Area52Build::Select(plans, plan, 10, selected)) && SameSelection(selected, {{1, 1}, {2, 1}}),
        "active plan adds eligible talent at level ten");
    Check(bool(Area52Build::Select(plans, plan, 15, selected)) && SameSelection(selected, {{1, 1}, {2, 3}}),
        "active plan merges talent ranks without duplicate essence charges");
    Check(bool(Area52Build::Select(plans, plan, 1, selected, {{2, 1}}, true)) == false,
        "retained selections still respect level and budget validation");
    Check(bool(Area52Build::Select(plans, plan, 15, selected, {{2, 3}}, true))
        && SameSelection(selected, {{1, 1}, {2, 3}}), "progression retains existing talent ranks");
    auto earlyTalent = plan;
    earlyTalent.Spells[1].Level = 1;
    Check(bool(Area52Build::Select(plans, earlyTalent, 15, selected, {}, true, 9))
        && SameSelection(selected, {{1, 1}}), "beginner book uses catalog level as well as authored level");
    Check(bool(Area52Build::Select(plans, plan, 15, selected, {{2, 3}}, true, 9))
        && SameSelection(selected, {{1, 1}, {2, 3}}), "beginner book retains previously learned higher-level talents");
    auto const unchanged = selected;
    plan.Spells.push_back({999, 80});
    Check(!Area52Build::Select(plans, plan, 1, selected) && SameSelection(selected, unchanged),
        "unknown future spell rejects the plan without partial output");
    plan.Spells.pop_back();
    plan.Spells[1].Level = 1;
    Check(!Area52Build::Select(plans, plan, 1, selected), "authored level cannot bypass talent gate");
    plan.Spells[1].Level = 10;
    plans.Entries[1].AE = 10;
    Check(!Area52Build::Select(plans, plan, 1, selected), "active plan cannot exceed essence budget");
    plans.Entries[1].AE = 3;
    plans.Entries[3] = ability;
    plans.Entries[3].Id = 3;
    Check(!Area52Build::Select(plans, plan, 1, selected), "ambiguous spell mapping fails closed");
    plans.Entries.erase(3);
    for (auto id : {1149, 1150, 1151, 1152, 18149})
        plans.Entries[id] = data.Entries.at(id);
    for (auto const& [stat, id] : std::map<std::uint32_t, std::uint32_t>{
        {1, 1149}, {2, 1150}, {3, 1151}, {4, 1152}, {6, 18149}})
    {
        plan.PrimaryStat = stat;
        Check(bool(Area52Build::Select(plans, plan, 15, selected, {}, false, 80, false))
            && SameSelection(selected, {{id, 1}}), "manual activation applies only the build primary stat");
        Check(bool(Area52Build::Select(plans, plan, 15, selected, {{1150, 1}, {1, 1}}, true, 80, false))
            && SameSelection(selected, {{id, 1}, {1, 1}}), "manual restore changes stat and preserves learned abilities");
        Check(bool(Area52Build::Select(plans, plan, 15, selected, {{1150, 1}, {1, 1}}, true))
            && SameSelection(selected, {{id, 1}, {1, 1}, {2, 3}}), "auto enable catches up all eligible plan ranks");
        Check(bool(Area52Build::Select(plans, plan, 1, selected))
            && SameSelection(selected, {{id, 1}, {1, 1}}), "auto activation excludes future plan ranks");
    }
    plan.PrimaryStat = 5;
    auto const priorOutput = selected;
    Check(!Area52Build::Select(plans, plan, 15, selected) && SameSelection(selected, priorOutput),
        "unsupported Stamina primary stat fails without changing selection");
    plan.PrimaryStat = 99;
    Check(!Area52Build::Select(plans, plan, 15, selected), "invalid primary stat is refused");
    plan.PrimaryStat = 1;
    plans.Entries.erase(1149);
    Check(!Area52Build::Select(plans, plan, 15, selected), "missing primary path fails closed");
    return Failures ? 1 : 0;
}
