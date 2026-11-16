#ifndef AREA52_CACHE_OWNERSHIP_H
#define AREA52_CACHE_OWNERSHIP_H

#include <cstdint>

class Player;
namespace Area52CacheOwnership
{
struct Appearance
{
    bool Ready = false;
    bool Collectible = false;
    bool Owned = false;
};

Appearance ReadAppearance(Player* player, std::uint32_t item);
}

#endif
