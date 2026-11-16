/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef AREA52_CRYOTHERAPIST_RULES_H
#define AREA52_CRYOTHERAPIST_RULES_H

#include <algorithm>
#include <array>
#include <cstdint>

namespace Area52Cryotherapist
{
inline constexpr std::uint32_t Enchant = 86576;
inline constexpr std::uint32_t FrozenLight = 92752;
inline constexpr std::uint32_t LesserFrozenLight = 300024;
inline constexpr std::array<std::uint32_t, 3> HealingTimesMs = {0, 1000, 2000};

inline std::uint32_t BaseHealingPulse(std::uint32_t damage)
{
    return static_cast<std::uint32_t>(std::uint64_t(damage) * 2 / 100);
}

inline std::uint32_t ConsumedHealingStacks(std::uint32_t available)
{
    return std::min<std::uint32_t>(available, 2);
}

inline std::uint32_t CastTime(std::uint32_t originalMs, std::uint32_t available)
{
    auto reduction = std::min<std::uint32_t>(100, 53 * ConsumedHealingStacks(available));
    return static_cast<std::uint32_t>(std::uint64_t(originalMs) * (100 - reduction) / 100);
}

inline std::uint32_t ManaCost(std::uint32_t originalCost, std::uint32_t available)
{
    return static_cast<std::uint32_t>(std::uint64_t(originalCost) * (100 - 10 * ConsumedHealingStacks(available)) / 100);
}

inline std::uint32_t RemainingHealingStacks(std::uint32_t available)
{
    return available - ConsumedHealingStacks(available);
}
}

#endif
