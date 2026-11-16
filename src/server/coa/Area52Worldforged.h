/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef AREA52_WORLDFORGED_H
#define AREA52_WORLDFORGED_H
#include <cstdint>
class Unit;
class Player;
class Aura;
class SpellInfo;
namespace Area52Worldforged
{
void RefreshAccess();
bool OwnerReady(Player* player);
void Damage(Unit* attacker, Unit* target, SpellInfo const* spell, std::uint32_t damage,
    std::uint32_t healthBefore, std::uint32_t maxHealth, bool periodic, bool killingBlow);
std::uint32_t Bonus(Unit* attacker, Unit* target, SpellInfo const* spell, std::uint32_t damage);
std::int32_t PrimaryStat(Aura* aura, std::int32_t fallback);
}
#endif
