#ifndef AREA52_CACHE_RUNTIME_H
#define AREA52_CACHE_RUNTIME_H
#include "Define.h"
#include "Area52CacheAttempt.h"
#include <optional>
class Player;
class Item;
namespace Area52CacheRuntime
{
struct Options
{
    bool prestige = false;
    uint32 allowance = 6;
    uint32 goldMin = 0;
    uint32 goldMax = 0;
    uint32 tokenChance = 25;
    uint32 secondTokenChance = 5;
};
bool QueueOpen(Player* player, Item* item, Options options);
// Catalog adapters are authoritative on every initial selection/reselection.
bool LoadCatalog();
std::optional<Area52CacheAttempt::State> SelectAttempt(Player* player, Item* item,
    Options options, std::optional<Area52CacheAttempt::State> previous);
bool StillEligible(Player* player, Area52CacheAttempt::State const& state, Options options);
}
void AddArea52CacheRuntime();
#endif
