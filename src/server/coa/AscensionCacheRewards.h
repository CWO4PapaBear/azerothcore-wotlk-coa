/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */
#ifndef ASCENSION_CACHE_REWARDS_H
#define ASCENSION_CACHE_REWARDS_H

#include "Define.h"
#include <vector>
#include <set>

class Item;
class Player;
enum InventoryResult : uint8;
namespace Area52CacheSelection { struct Candidate; }
namespace Area52CacheAttempt { struct State; }

namespace AscensionCacheRewards
{
struct SetRelevance { bool ready = false; bool useful = false; uint32 matchingAbilities = 0; };
SetRelevance EvaluateSet(Player* player, uint32 setId);
bool TierOpen(uint32 setId, uint32 realmCap);
bool SourceOpen(uint32 itemId);
std::set<uint32> OpenSources();
struct Reward
{
    uint32 itemId;
    uint16 itemLevel;
    uint8 statType;
    uint8 armorClass;
    uint32 count = 1;
    int32 randomPropertyId = 0;
};

bool Deliver(Player* player, std::vector<Reward> const& rewards, Item* cache);
bool DeliverReserved(Player* player, Area52CacheAttempt::State& attempt, Item* cache);
bool PopulateInventoryAndBankOwnership(Player* player, std::vector<Area52CacheSelection::Candidate>& candidates);
InventoryResult CheckVisibleBagSpace(Player* player, std::vector<Reward> const& rewards);
void NotifyInsufficientBagSpace(Player* player);
}

#endif
