#ifndef AREA52_ARCANIST_RULES_H
#define AREA52_ARCANIST_RULES_H

#include <array>
#include <cstdint>

namespace Area52Arcanist
{
constexpr std::uint32_t Enchant = 984117;
constexpr std::uint32_t Meteor = 760040;
constexpr std::uint32_t Potential = 984138;
constexpr std::array<std::uint32_t, 4> Schools = {64, 4, 16, 8};
constexpr std::array<std::uint32_t, 4> Meteors = {984126, 984118, 984122, 984130};
constexpr std::array<std::uint32_t, 4> Motes = {984141, 984139, 984140, 984142};
constexpr std::array<std::uint32_t, 4> Resonances = {984408, 984406, 984407, 984409};
constexpr std::array<std::uint32_t, 4> Cooldowns = {984267, 984394, 984266, 984268};

inline int Element(std::uint32_t schools)
{
    for (int index : {1, 3, 2, 0})
        if (schools & Schools[index])
            return index;
    return -1;
}

inline float PvpCompensation(float bonus)
{
    return (100.0f + bonus * 0.5f) / (100.0f + bonus);
}

template <typename Owner>
int CurrentMote(Owner* owner)
{
    for (int index = 0; index < 4; ++index)
        if (owner->HasAura(Motes[index], owner->GetGUID()))
            return index;
    return -1;
}

template <typename Owner>
void Clear(Owner* owner)
{
    owner->RemoveAurasDueToSpell(Potential, owner->GetGUID());
    for (auto const& group : {Motes, Resonances, Cooldowns})
        for (auto id : group)
            owner->RemoveAurasDueToSpell(id, owner->GetGUID());
}

template <typename Owner>
void Advance(Owner* owner, unsigned stacks)
{
    if (stacks < 4)
        return;
    int const index = CurrentMote(owner);
    if (index < 0)
        return;
    owner->RemoveAurasDueToSpell(Potential, owner->GetGUID());
    owner->RemoveAurasDueToSpell(Motes[index], owner->GetGUID());
    owner->CastSpell(owner, Resonances[index], true);
    owner->CastSpell(owner, Motes[(index + 1) % 4], true);
}
}

#endif
