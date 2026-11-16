/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef AREA52_WIZARD_FROSTBOLT_RULES_H
#define AREA52_WIZARD_FROSTBOLT_RULES_H
#include <cstdint>
namespace Area52WizardFrostbolt
{
constexpr std::uint32_t Enchant = 781474, Companion = 781475, Power = 781300, Mana = 781301, Cooldown = 780999;
inline std::uint32_t SpellPower(std::uint32_t level, std::uint32_t copies)
{
    return level * 2 * copies / 5;
}
inline std::uint32_t ManaReturn(std::uint32_t baseMana, std::uint32_t copies)
{
    return std::uint64_t(baseMana) * 4 * copies / 100;
}
inline bool Eligible(bool area52, bool active, bool alive, bool triggered, bool matching,
    bool damage, bool missed, bool cooldown, bool rolled)
{
    return area52 && active && alive && !triggered && matching && damage && !missed && !cooldown && !rolled;
}
}
#endif
