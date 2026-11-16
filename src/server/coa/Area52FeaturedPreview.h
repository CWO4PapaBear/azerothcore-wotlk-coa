#ifndef AREA52_FEATURED_PREVIEW_H
#define AREA52_FEATURED_PREVIEW_H

#include "Area52FeaturedRewards.h"
#include <array>
#include <charconv>
#include <cstdint>
#include <string_view>

namespace Area52Build
{
constexpr std::uint16_t FeaturedPreviewRequest = 0x7F10;
constexpr std::uint16_t FeaturedPreviewResponse = 0x7F11;
constexpr std::uint32_t FeaturedPreviewVersion = 1;
constexpr std::uint32_t FeaturedPreviewDefault = UINT32_MAX;
using FeaturedPreviewEffects = std::array<std::uint32_t, 4>;

inline bool ReadFeaturedPreview(std::string_view description, FeaturedPreviewEffects& effects)
{
    effects.fill(FeaturedPreviewDefault);
    constexpr std::string_view marker = "###AREA52_FEATURED_PREVIEW\n";
    auto start = description.find(marker);
    if (start == std::string_view::npos)
        return true;
    if ((start && description[start - 1] != '\n')
        || description.find(marker, start + marker.size()) != std::string_view::npos)
        return false;
    start += marker.size();
    auto const end = description.find('\n', start);
    auto values = description.substr(start, end == std::string_view::npos ? end : end - start);
    if (!values.starts_with("1:"))
        return false;
    values.remove_prefix(2);
    for (std::size_t i = 0; i < effects.size(); ++i)
    {
        auto const comma = values.find(',');
        auto const value = values.substr(0, comma);
        auto const parsed = std::from_chars(value.data(), value.data() + value.size(), effects[i]);
        if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
            return false;
        if (i + 1 == effects.size())
            return comma == std::string_view::npos;
        if (comma == std::string_view::npos)
            return false;
        values.remove_prefix(comma + 1);
    }
    return false;
}
}
#endif
