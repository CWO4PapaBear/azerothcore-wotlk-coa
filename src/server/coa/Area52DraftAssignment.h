#ifndef AREA52_DRAFT_ASSIGNMENT_H
#define AREA52_DRAFT_ASSIGNMENT_H
#include "Area52BuildProtocol.h"
#include <charconv>
#include <string_view>
namespace Area52Build
{
constexpr std::uint32_t FeaturedDraftCategory = 8;
struct CreationArchetype { std::uint32_t Id; std::string_view Name; };
constexpr CreationArchetype CreationArchetypes[] = {
    {19, "Blade Warden"},
    {20, "Bulwark"},
    {143, "Carnage Incarnate"},
    {22, "Relentless"},
    {21, "Swashbuckler"},
    {24, "Vanguard Sentinel"},
    {26, "Demoniac"},
    {31, "Icebound"},
    {25, "Templar of Flame"},
    {33, "Beast of Nightmare"},
    {29, "Crimson Champion"},
    {27, "Mystic Knight"},
    {32, "Plague Bearer"},
    {30, "Stormguard"},
    {34, "Thunderpaw"},
    {2, "Dawnkeeper"},
    {3, "Eternal Caretaker"},
    {10, "Grove Keeper"},
    {12, "Hierophant"},
    {1, "Naturalist"},
    {6, "Sacred Sentinel"},
    {16, "Steward of Eternity"},
    {7, "Tide Mender"},
    {13, "Cryotherapist"},
    {17, "Fire Sage"},
    {15, "Grove Ranger"},
    {14, "Tree of Wrath"},
    {18, "Vampire Lord"},
    {86, "Defias Gunslinger"},
    {78, "Dragon Warrior"},
    {129, "Gutripper"},
    {77, "Heavyweight"},
    {73, "Hoplite"},
    {79, "Son of Gruul"},
    {74, "Warden"},
    {141, "Wind Rager"},
    {85, "Bestial Ranger"},
    {140, "Dark Ranger"},
    {139, "Locust Ranger"},
    {82, "Marksman"},
    {84, "Paratrooper"},
    {101, "Ascendant"},
    {97, "Cryomancer"},
    {90, "Druid of the Eclipse"},
    {88, "Evoker"},
    {144, "Fel Apostate"},
    {95, "Shadowseer"},
    {104, "Stormcaller"},
    {132, "Crusader"},
    {136, "Divine Sentinel"},
    {75, "Druid of the Fang"},
    {134, "Firelord"},
    {131, "Hellcat"},
    {80, "Master of Shadows"},
    {142, "Storm Giant"},
    {133, "White Walker"},
};
inline std::uint32_t DraftArchetype(Entry const& entry)
{
    constexpr std::string_view marker = "###AREA52_ARCHETYPE_ID\n";
    auto const start = entry.Description.find(marker);
    if (start == std::string::npos || (start && entry.Description[start - 1] != '\n')
        || entry.Description.find(marker, start + marker.size()) != std::string::npos)
        return 0;
    auto const begin = entry.Description.data() + start + marker.size();
    auto const finish = entry.Description.data() + entry.Description.size();
    std::uint32_t id = 0;
    auto parsed = std::from_chars(begin, finish, id);
    if (parsed.ec != std::errc{} || begin == parsed.ptr)
        return 0;
    while (parsed.ptr != finish && (*parsed.ptr == '\n' || *parsed.ptr == '\r' || *parsed.ptr == ' '))
        ++parsed.ptr;
    if (parsed.ptr != finish)
        return 0;
    for (auto const& archetype : CreationArchetypes)
        if (archetype.Id == id && archetype.Name == entry.Name)
            return id;
    return 0;
}
inline bool CanSaveDraftAssignment(Entry const& entry, bool gameMaster)
{
    if (entry.Category == FeaturedDraftCategory)
        return gameMaster && DraftArchetype(entry);
    return entry.Category >= 1 && entry.Category <= 7
        && entry.Description.find("AREA52_ARCHETYPE_ID") == std::string::npos;
}
constexpr std::uint64_t CreationReferenceBase = std::uint64_t{1} << 63;
inline std::uint32_t CreationArchetypeFromReference(std::uint64_t reference)
{
    if (reference < CreationReferenceBase)
        return 0;
    for (auto const& archetype : CreationArchetypes)
        if (reference == CreationReferenceBase + archetype.Id)
            return archetype.Id;
    return 0;
}
}
#endif
