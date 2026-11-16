/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef COA_CALLBOARD_REALM_TIER_H
#define COA_CALLBOARD_REALM_TIER_H
#include <string_view>
class Quest;
class Player;
namespace CallboardRealmTier
{
constexpr unsigned PlayerTier(unsigned level, unsigned realmMaximum)
{
    unsigned const effective = level < realmMaximum ? level : realmMaximum;
    if (effective >= 80) return 80;
    if (effective >= 70) return 70;
    if (effective >= 60) return 60;
    return 0;
}
unsigned ForPlayer(Player const* player);
constexpr unsigned RequiredCap(std::string_view type)
{
    if (type.starts_with("OpenWorldVanilla") || type.starts_with("Vanilla")) return 60;
    if (type.starts_with("TBC")) return 70;
    if (type.starts_with("Northrend")) return 80;
    return 0;
}
constexpr bool Available(std::string_view type, unsigned cap)
{
    unsigned const required = RequiredCap(type);
    return cap && (!required || required == cap);
}
constexpr bool DungeonMatches(unsigned cap, unsigned expansion)
{
    return (cap == 60 && expansion == 0) || (cap == 70 && expansion == 1) || (cap == 80 && expansion == 2);
}
bool CanUse(Player const* player, Quest const* quest);
}
#endif
