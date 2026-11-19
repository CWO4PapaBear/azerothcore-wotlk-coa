#ifndef AREA52_CALLBOARD_CATEGORY_POLICY_H
#define AREA52_CALLBOARD_CATEGORY_POLICY_H

#include <algorithm>
#include <cstdint>
#include <vector>

namespace Area52CallboardCategory
{
struct Pool
{
    std::uint32_t item;
    std::uint32_t cap;
    std::uint32_t map;
    std::uint32_t difficulty;
    std::uint32_t minimum;
    std::uint32_t maximum;
};

inline bool MythicAvailable(std::uint32_t cap)
{
    return cap == 60;
}

inline std::uint32_t Choose(std::vector<Pool> const& pools, std::uint32_t cap,
    double equipped, std::uint32_t allowance)
{
    Pool const* lowest = nullptr;
    Pool const* chosen = nullptr;
    for (auto const& pool : pools)
    {
        if (pool.cap != cap || !pool.minimum || !pool.maximum
            || (!pool.map && pool.difficulty == 2 && !MythicAvailable(cap)))
            continue;
        if (!lowest || pool.minimum < lowest->minimum)
            lowest = &pool;
        if (pool.maximum <= equipped + allowance && (!chosen || pool.maximum >= chosen->maximum))
            chosen = &pool;
    }
    Pool const* result = chosen ? chosen : lowest;

    if (cap > 60 && result && result->maximum <= equipped)
    {
        Pool const* next = nullptr;
        for (auto const& pool : pools)
            if (pool.cap == cap && pool.minimum && pool.maximum > result->maximum
                && pool.minimum <= equipped + allowance
                && (pool.map || pool.difficulty != 2 || MythicAvailable(cap))
                && (!next || pool.maximum < next->maximum))
                next = &pool;
        if (next) result = next;
    }
    return result ? result->item : 0;
}

inline std::uint32_t DungeonItem(std::uint32_t cap, std::uint32_t difficulty)
{
    if (difficulty != 1 && difficulty != 2)
        return 0;
    if (difficulty == 2 && !MythicAvailable(cap))
        return 0;
    return cap == 60 ? 1615000 + difficulty : cap == 70 ? 1999599 + difficulty
        : cap == 80 ? 1999610 + difficulty : 0;
}
}

#endif
