/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef COA_ASCENSION_CALLBOARD_CONTRACTS_H
#define COA_ASCENSION_CALLBOARD_CONTRACTS_H
#include "DatabaseEnv.h"
class Player;
class Quest;
namespace AscensionCallboardContracts
{
CharacterDatabaseTransaction CurrentRewardTransaction();
bool CanReward(Player const* player, Quest const* quest);
void RecordReward(Player* player, Quest const* quest);
void AppendSave(Player* player, CharacterDatabaseTransaction transaction);
}
#endif
