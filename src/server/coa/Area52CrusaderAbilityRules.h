#pragma once

#include <cstdint>

namespace Area52CrusaderAbility
{
constexpr bool IsHaloRecipient(std::uint32_t spell)
{
    return spell == 954875 || (spell >= 955004 && spell <= 955008) ||
        (spell >= 955013 && spell <= 955016);
}

constexpr bool IsHaloMissile(std::uint32_t spell)
{
    return spell == 954874 || (spell >= 955017 && spell <= 955020);
}

constexpr bool IsHaloDistanceTarget(std::uint32_t spell)
{
    return IsHaloMissile(spell) || spell == 954875 || (spell >= 955013 && spell <= 955016);
}

constexpr bool HaloHealingApplies(bool area52, bool sameCaster, std::uint32_t schoolMask)
{
    return area52 && sameCaster && (schoolMask & 2);
}
}
