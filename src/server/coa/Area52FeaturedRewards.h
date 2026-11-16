#ifndef AREA52_FEATURED_REWARDS_H
#define AREA52_FEATURED_REWARDS_H

#include <array>
#include <charconv>
#include <cstdint>
#include <string_view>

namespace Area52Build
{
constexpr std::array<std::uint32_t, 14> FeaturedRewardLevels{25, 10, 13, 16, 28, 36, 40, 45, 50, 22, 22, 22, 20, 60};
using FeaturedRewardItems = std::array<std::uint32_t, 14>;

inline bool ReadFeaturedRewards(std::string_view description, FeaturedRewardItems& items)
{
    items.fill(0);
    constexpr std::string_view marker = "###AREA52_FEATURED_VANITY\n";
    auto start = description.find(marker);
    if (start == std::string_view::npos)
        return true;
    if ((start && description[start - 1] != '\n')
        || description.find(marker, start + marker.size()) != std::string_view::npos)
        return false;
    start += marker.size();
    auto end = description.find('\n', start);
    auto const line = description.substr(start, end == std::string_view::npos ? end : end - start);
    if (!line.starts_with("1:"))
        return false;
    auto values = line.substr(2);
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        auto const comma = values.find(',');
        auto const value = values.substr(0, comma);
        auto parsed = std::from_chars(value.data(), value.data() + value.size(), items[i]);
        if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
            return false;
        if (i + 1 == items.size())
            return comma == std::string_view::npos;
        if (comma == std::string_view::npos)
            return false;
        values.remove_prefix(comma + 1);
    }
    return false;
}

inline bool FeaturedSlotAccepts(std::size_t slot, std::uint32_t inventoryType)
{
    constexpr std::array<std::uint32_t, 9> armor{1, 3, 5, 10, 8, 9, 6, 16, 7};
    if (slot < armor.size())
        return inventoryType == armor[slot] || (slot == 2 && inventoryType == 20);
    if (slot == 9)
        return inventoryType == 13 || inventoryType == 17 || inventoryType == 21;
    if (slot == 10)
        return inventoryType == 13 || inventoryType == 14 || inventoryType == 22 || inventoryType == 23;
    if (slot == 11)
        return inventoryType == 15 || inventoryType == 25 || inventoryType == 26;
    return slot < 14 && inventoryType == 0;
}
}
#endif
