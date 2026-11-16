#ifndef AREA52_FEATURED_SET_H
#define AREA52_FEATURED_SET_H

#include "Area52BuildProtocol.h"
#include "Area52FeaturedRewards.h"

namespace Area52Build
{
constexpr char FeaturedSetQuery[] =
    "SELECT COALESCE((SELECT JSON_UNQUOTE(JSON_EXTRACT(r.document,'$.wire')) "
    "FROM area52_build_archetype a JOIN area52_build b ON b.id=a.build_id AND b.archived=0 "
    "JOIN area52_build_revision r ON r.build_id=a.build_id AND r.revision=a.revision AND r.category=8 "
    "JOIN area52_build_featured f ON f.build_id=a.build_id AND f.revision=a.revision "
    "WHERE a.archetype_id={}), '')";

inline int ReadFeaturedSet(std::string_view hex, FeaturedRewardItems& items)
{
    if (hex.empty())
        return 0;
    if (hex.size() % 2 || hex.size() > MaxPacketBytes * 2)
        return -1;
    std::vector<std::uint8_t> bytes;
    bytes.reserve(hex.size() / 2);
    for (std::size_t i = 0; i < hex.size(); i += 2)
    {
        unsigned value = 0;
        auto const result = std::from_chars(hex.data() + i, hex.data() + i + 2, value, 16);
        if (result.ec != std::errc{} || result.ptr != hex.data() + i + 2)
            return -1;
        bytes.push_back(std::uint8_t(value));
    }
    Entry entry;
    if (!Decode(bytes, entry) || entry.Category != 8 || !ReadFeaturedRewards(entry.Description, items))
        return -1;
    return entry.Description.find("###AREA52_FEATURED_VANITY\n") == std::string::npos ? 0 : 1;
}
}
#endif
