#pragma once
#include <cstdint>
namespace AscensionFreePick
{
constexpr bool SharesDefensiveCooldown(std::uint32_t firstRank)
{
    return firstRank == 19263 || firstRank == 31224 || firstRank == 642 ||
        firstRank == 1022 || firstRank == 45438;
}
}
