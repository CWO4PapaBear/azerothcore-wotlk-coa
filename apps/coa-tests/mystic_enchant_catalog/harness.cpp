/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionMysticEnchantCatalog.h"
#include <cstdio>

using namespace AscensionMysticEnchant;
int main(int argc, char** argv)
{
    if (argc != 2)
        return 2;
    int failed = 0;
    auto check = [&](bool pass, char const* label)
    {
        std::printf("%s: %s\n", pass ? "PASS" : "FAIL", label);
        failed += !pass;
    };
    Catalog catalog;
    check(LoadCatalog(catalog, std::string(argv[1]) + "/MysticEnchant.dbc"), "load effective live Hero catalog");
    check(catalog.BySpell.size() == 3822, "live Hero catalog matches reviewed client data");
    for (auto const& [spell, entry] : catalog.BySpell)
        if (entry.Item && catalog.ByItem.at(entry.Item) != spell)
            ++failed;
    check(!LoadCatalog(catalog, std::string(argv[1]) + "/absent-mystic.dbc") && catalog.BySpell.empty(),
        "failed reload cannot retain stale entries");
    std::map<std::uint32_t, SpellNode> spells = {{1, {true, {2}}}, {2, {true, {}}}, {3, {false, {}}}};
    check(ResolveNative(1, spells), "native dependency resolves");
    check(!ResolveNative(99, spells), "missing spell is rejected");
    spells.at(2).Dependencies.insert(3);
    check(!ResolveNative(1, spells), "unsupported nested payload is rejected");
    spells.at(2).Dependencies = {1};
    check(ResolveNative(1, spells), "native cyclic graph terminates");
    spells.at(2).Dependencies.insert(99);
    check(!ResolveNative(1, spells), "missing dependency within cycle is rejected");
    Catalog synthetic;
    std::set<std::uint32_t> owned;
    std::map<std::uint32_t, std::uint32_t> limits;
    for (std::uint32_t id = 1; id <= 19; ++id)
    {
        Entry entry;
        entry.Spell = id;
        entry.Quality = id <= 13 ? 2 + id % 2 : id <= 16 ? 4 : id == 17 ? 5 : 6;
        synthetic.BySpell.emplace(id, entry);
        owned.insert(id);
        limits[id] = 1;
    }
    Slots slots;
    for (std::uint32_t i = 0; i < slots.size(); ++i)
        slots[i] = i + 1;
    check(ValidateSlots(synthetic, slots, 1, owned, limits), "13 mixed Rare/Uncommon, 3 Epic, 1 Legendary");
    slots[16] = 18;
    check(!ValidateSlots(synthetic, slots, 1, owned, limits), "Artifact cannot occupy Legendary slot");
    slots[16] = 17;
    synthetic.BySpell.at(13).Quality = 4;
    check(!ValidateSlots(synthetic, slots, 1, owned, limits), "four Epics plus a Legendary are rejected");
    synthetic.BySpell.at(13).Quality = 3;
    synthetic.BySpell.at(14).Quality = 2;
    check(ValidateSlots(synthetic, slots, 1, owned, limits), "lower rarity can fill an Epic capacity");
    synthetic.BySpell.at(14).Quality = 4;
    owned.erase(1);
    check(!ValidateSlots(synthetic, slots, 1, owned, limits), "unowned enchant is rejected");
    owned.insert(1);
    slots[1] = 1;
    check(!ValidateSlots(synthetic, slots, 1, owned, limits), "duplicate beyond stack limit is rejected");
    slots[1] = 2;
    for (auto& [id, entry] : synthetic.BySpell)
        entry.Quality = id % 2 ? 2 : 3;
    check(ValidateSlots(synthetic, slots, 1, owned, limits), "seventeen Rare or Uncommon enchants are accepted");
    for (std::uint32_t id = 1; id <= 4; ++id)
        synthetic.BySpell.at(id).Quality = 4;
    check(ValidateSlots(synthetic, slots, 1, owned, limits), "four Epics without a Legendary are accepted");
    synthetic.BySpell.at(5).Quality = 4;
    check(!ValidateSlots(synthetic, slots, 1, owned, limits), "five Epics are rejected");
    synthetic.BySpell.at(5).Quality = 3;
    synthetic.BySpell.at(1).Quality = 5;
    check(ValidateSlots(synthetic, slots, 1, owned, limits), "one Legendary with three Epics is accepted");
    synthetic.BySpell.at(2).Quality = 5;
    check(!ValidateSlots(synthetic, slots, 1, owned, limits), "two Legendaries are rejected");
    Slots saved{978217, 782246, 782246, 782246};
    auto removed = saved;
    removed[1] = 0;
    check(IsRemovalOnly(saved, removed), "remove one rejected copy without accepting new enchants");
    check(!IsRemovalOnly(saved, saved), "unchanged setup is not a removal");
    removed[4] = 84445;
    check(!IsRemovalOnly(saved, removed), "removal cannot bypass validation for an addition");
    removed = saved;
    removed[4] = removed[1];
    removed[1] = 0;
    check(!IsRemovalOnly(saved, removed), "moving a rejected enchant is not a removal");
    removed.fill(0);
    check(IsRemovalOnly(saved, removed), "all rejected slots can be cleared");
    return failed ? 1 : 0;
}
