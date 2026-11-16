#pragma once
#include "DatabaseEnv.h"
#include <utility>
#include <vector>
class Player;
struct Area52MilestoneCollection
{
    std::vector<std::pair<uint32,uint32>> Appearances;
    std::vector<uint32> Items;
};
void AppendArea52MilestoneUnlock(Player*, uint32, CharacterDatabaseTransaction&, Area52MilestoneCollection&);
void ConfirmArea52MilestoneUnlock(Player*, Area52MilestoneCollection const&);
bool Area52MilestoneCatalogReady();
