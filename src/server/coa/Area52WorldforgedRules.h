/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef AREA52_WORLDFORGED_RULES_H
#define AREA52_WORLDFORGED_RULES_H

#include <algorithm>
#include <cstdint>
#include <vector>

namespace Area52Worldforged
{
enum class EventKind { Damage, Direct, Periodic, KillingBlow };
enum class Family { Fighter, Wizard, Primal, Demonic, Draconic, Gigantic, Necrotic, ElementBound };

struct Candidate
{
    std::uint32_t enchant;
    std::uint32_t copies;
    std::uint32_t chance;
    Family family;
    EventKind required;
    bool matches;
};

struct Event
{
    bool area52;
    bool active;
    bool alive;
    bool cooldown;
    bool periodic;
    bool killingBlow;
    std::uint32_t damage;
    std::uint32_t healthBefore;
    std::uint32_t maxHealth;
};

inline bool Eligible(Candidate const& candidate, Event const& event)
{
    if (!event.area52 || !event.active || !event.alive || event.cooldown || !event.damage || !event.healthBefore ||
        !candidate.matches || !candidate.copies || !candidate.chance || candidate.chance > 100)
        return false;
    if (candidate.required == EventKind::Direct && event.periodic)
        return false;
    if (candidate.required == EventKind::Periodic && !event.periodic)
        return false;
    if (candidate.required == EventKind::KillingBlow && !event.killingBlow)
        return false;
    if (candidate.family == Family::Gigantic && (!event.maxHealth ||
        std::uint64_t(event.healthBefore) * 100 <= std::uint64_t(event.maxHealth) * 75))
        return false;
    return true;
}

template <class Random, class Roll>
std::uint32_t Select(std::vector<Candidate> candidates, Event const& event, Random& random, Roll&& roll)
{
    candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
        [&](Candidate const& candidate) { return !Eligible(candidate, event); }), candidates.end());
    std::shuffle(candidates.begin(), candidates.end(), random);
    for (Candidate const& candidate : candidates)
        if (roll(candidate.chance))
            return candidate.enchant;
    return 0;
}

inline std::uint32_t Scaled(std::uint32_t value, std::uint32_t copies,
    std::uint32_t numerator, std::uint32_t denominator)
{
    return std::uint64_t(value) * copies * numerator / denominator;
}

inline std::uint32_t DamageBonus(Family family, std::uint32_t copies,
    std::uint32_t healthBefore, std::uint32_t maxHealth)
{
    if (!maxHealth)
        return 0;
    bool const gigantic = family == Family::Gigantic &&
        std::uint64_t(healthBefore) * 100 > std::uint64_t(maxHealth) * 75;
    bool const necrotic = family == Family::Necrotic &&
        std::uint64_t(healthBefore) * 100 < std::uint64_t(maxHealth) * 35;
    return gigantic || necrotic ? 2 * copies : 0;
}
}
#endif
