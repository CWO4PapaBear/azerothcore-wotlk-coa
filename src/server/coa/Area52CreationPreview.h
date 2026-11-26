#ifndef AREA52_CREATION_PREVIEW_H
#define AREA52_CREATION_PREVIEW_H

#include "Area52FeaturedPreview.h"
#include "Area52FeaturedIncarnations.h"
#include "Area52CreationPreviewIncarnations.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <string_view>

namespace Area52Build
{
using CreationPreviewRow = std::array<std::uint32_t, 28>;
constexpr std::uint16_t CreationPreviewOpcode = 0x6E3;

struct CreationSelection
{
    std::uint32_t Stance = FeaturedPreviewDefault;
    std::uint32_t Weapons = 0;
    bool operator==(CreationSelection const&) const = default;
};

inline bool ReadCreationNumber(std::string_view description, std::string_view marker, std::uint32_t& value)
{
    auto start = description.find(marker);
    if (start == std::string_view::npos)
        return true;
    if ((start && description[start - 1] != '\n')
        || description.find(marker, start + marker.size()) != std::string_view::npos)
        return false;
    start += marker.size();
    auto const end = description.find('\n', start);
    auto line = description.substr(start, end == std::string_view::npos ? end : end - start);
    if (!line.starts_with("1:"))
        return false;
    line.remove_prefix(2);
    auto const parsed = std::from_chars(line.data(), line.data() + line.size(), value);
    return !line.empty() && parsed.ec == std::errc{} && parsed.ptr == line.data() + line.size();
}

inline bool ReadCreationSelection(std::string_view description, CreationSelection& selection)
{
    selection = {};
    if (!ReadCreationNumber(description, "###AREA52_FEATURED_STANCE\n", selection.Stance)
        || !ReadCreationNumber(description, "###AREA52_FEATURED_WEAPONS\n", selection.Weapons))
        return false;
    constexpr std::array<std::uint32_t, 10> stances{FeaturedPreviewDefault, 0, 25, 26, 27, 28, 29, 48, 51, 52};
    return std::find(stances.begin(), stances.end(), selection.Stance) != stances.end() && selection.Weapons <= 3;
}

inline bool ProjectCreationPreview(CreationPreviewRow& row, std::string_view description)
{
    CreationSelection selection;
    FeaturedRewardItems items;
    FeaturedPreviewEffects preview;
    FeaturedPreviewEffects incarnations;
    if (!ReadCreationSelection(description, selection) || !ReadFeaturedRewards(description, items)
        || !ReadFeaturedPreview(description, preview) || !ReadFeaturedIncarnations(description, incarnations))
        return false;
    auto const current = std::find_if(CreationIncarnations.begin(), CreationIncarnations.end(),
        [&](auto const& value) { return value[2] == row[14]; });
    if (current != CreationIncarnations.end())
    {
        auto const category = std::find(FeaturedIncarnationCategories.begin(), FeaturedIncarnationCategories.end(),
            (*current)[1]);
        if (category != FeaturedIncarnationCategories.end())
        {
            auto const appearance = incarnations[category - FeaturedIncarnationCategories.begin()];
            if (appearance == 0)
                row[14] = 0;
            else if (appearance != FeaturedPreviewDefault)
            {
                auto const selected = std::find_if(CreationIncarnations.begin(), CreationIncarnations.end(),
                    [&](auto const& value) { return value[0] == appearance && value[1] == (*current)[1]; });
                if (selected == CreationIncarnations.end())
                    return false;
                row[14] = (*selected)[2];
            }
        }
    }
    if (description.find("###AREA52_FEATURED_VANITY\n") != std::string_view::npos)
        std::copy_n(items.begin(), 12, row.begin() + 16);
    if (selection.Stance != FeaturedPreviewDefault)
        row[12] = selection.Stance;
    if (selection.Weapons == 2 || selection.Weapons == 3)
        row[25] = row[26] = 0;
    if (selection.Weapons == 1 || selection.Weapons == 3)
        row[27] = 0;
    if (preview[0] != FeaturedPreviewDefault)
        row[13] = preview[0];
    return true;
}
}

class WorldSession;
void SendArea52CreationPreview(WorldSession* session);
#endif
