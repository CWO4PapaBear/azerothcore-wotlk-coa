/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef COA_CALLBOARD_HUNT_ROTATION_H
#define COA_CALLBOARD_HUNT_ROTATION_H
#include <algorithm>
#include <cstdint>
#include <iterator>
class Player;
namespace CallboardHunts
{
struct Zone { unsigned tier, id; char const* name; };
inline constexpr Zone Zones[] = {
    { 60, 618, "Winterspring" },
    { 60, 361, "Felwood" },
    { 60, 139, "Eastern Plaguelands" },
    { 60, 28, "Western Plaguelands" },
    { 60, 16, "Azshara" },
    { 60, 1377, "Silithus" },
    { 60, 490, "Un'Goro Crater" },
    { 60, 46, "Burning Steppes" },
    { 70, 3483, "Hellfire Peninsula" },
    { 70, 3521, "Zangarmarsh" },
    { 70, 3519, "Terokkar Forest" },
    { 70, 3518, "Nagrand" },
    { 70, 3522, "Blade's Edge Mountains" },
    { 70, 3523, "Netherstorm" },
    { 70, 3520, "Shadowmoon Valley" },
    { 80, 3537, "Borean Tundra" },
    { 80, 495, "Howling Fjord" },
    { 80, 65, "Dragonblight" },
    { 80, 394, "Grizzly Hills" },
    { 80, 66, "Zul'Drak" },
    { 80, 3711, "Sholazar Basin" },
    { 80, 67, "The Storm Peaks" },
    { 80, 210, "Icecrown" },
};
struct Entry { unsigned quest, zone; };
inline constexpr Entry Quests[] = {
    { 17838, 3522 },
    { 17851, 3522 },
    { 17862, 3522 },
    { 17869, 3518 },
    { 17870, 3518 },
    { 17874, 3518 },
    { 17878, 3518 },
    { 17884, 3518 },
    { 17898, 3523 },
    { 17906, 3523 },
    { 17907, 3523 },
    { 17921, 3520 },
    { 17926, 618 },
    { 17930, 618 },
    { 18005, 618 },
    { 18016, 618 },
    { 18017, 361 },
    { 18018, 361 },
    { 18019, 361 },
    { 18021, 361 },
    { 18027, 361 },
    { 18030, 139 },
    { 18033, 139 },
    { 18034, 139 },
    { 18036, 139 },
    { 18038, 139 },
    { 18044, 139 },
    { 18045, 139 },
    { 18046, 139 },
    { 18052, 28 },
    { 18054, 28 },
    { 18055, 28 },
    { 18056, 28 },
    { 18057, 28 },
    { 18062, 28 },
    { 18065, 490 },
    { 18067, 490 },
    { 18072, 490 },
    { 18078, 46 },
    { 18082, 46 },
    { 18087, 46 },
    { 18089, 16 },
    { 18090, 16 },
    { 18091, 16 },
    { 18093, 16 },
    { 18103, 1377 },
    { 18104, 1377 },
    { 18105, 1377 },
    { 18106, 1377 },
    { 18108, 1377 },
    { 18109, 1377 },
    { 18110, 1377 },
    { 18111, 1377 },
    { 18114, 139 },
    { 354000, 3523 },
    { 354001, 3523 },
    { 354005, 3520 },
    { 354007, 3520 },
    { 354008, 3520 },
    { 354012, 3522 },
    { 354013, 3522 },
    { 354016, 3518 },
    { 354018, 3518 },
    { 354020, 3519 },
    { 354022, 3519 },
    { 354023, 3519 },
    { 354024, 3519 },
    { 354028, 3521 },
    { 354030, 3483 },
    { 354034, 3483 },
    { 354047, 3519 },
    { 354051, 3519 },
    { 354061, 3521 },
    { 354062, 3521 },
    { 354065, 3521 },
    { 354080, 3521 },
    { 354081, 3521 },
    { 354082, 3521 },
    { 354091, 3483 },
    { 354107, 3483 },
    { 354115, 3520 },
    { 354117, 3520 },
    { 354120, 3520 },
    { 354121, 3520 },
    { 354127, 3520 },
    { 9080000, 495 },
    { 9080001, 3537 },
    { 9080002, 65 },
    { 9080003, 394 },
    { 9080004, 3711 },
    { 9080005, 3711 },
    { 9080006, 67 },
    { 9080007, 210 },
    { 9080008, 66 },
    { 9080100, 3537 },
    { 9080101, 3537 },
    { 9080102, 3537 },
    { 9080103, 495 },
    { 9080104, 495 },
    { 9080105, 65 },
    { 9080106, 65 },
    { 9080107, 394 },
    { 9080108, 3711 },
    { 9080109, 67 },
    { 9080110, 67 },
    { 9080111, 210 },
    { 9080112, 210 },
    { 9080200, 3537 },
    { 9080201, 495 },
    { 9080202, 65 },
    { 9080203, 394 },
    { 9080204, 66 },
    { 9080205, 3711 },
    { 9080206, 67 },
    { 9080207, 210 },
};
inline Zone const* Active(unsigned tier, std::int64_t day)
{
    auto first = std::find_if(std::begin(Zones), std::end(Zones), [tier](Zone z){ return z.tier == tier; });
    if (first == std::end(Zones) || day < 0) return nullptr;
    auto end = std::find_if(first, std::end(Zones), [tier](Zone z){ return z.tier != tier; });
    return first + day % (end - first);
}
inline unsigned ZoneFor(unsigned quest)
{
    auto row = std::lower_bound(std::begin(Quests), std::end(Quests), quest,
        [](Entry entry, unsigned id){ return entry.quest < id; });
    return row != std::end(Quests) && row->quest == quest ? row->zone : 0;
}
inline bool Offered(unsigned quest, unsigned tier, std::int64_t day)
{
    unsigned const expected = ZoneFor(quest);
    if (!expected) return true;
    auto zone = Active(tier, day);
    return zone && zone->id == expected;
}
std::int64_t ResetDay();
bool CanOffer(Player const* player, unsigned quest);
char const* Today(Player const* player);
}
#endif
