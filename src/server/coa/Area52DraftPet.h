#ifndef AREA52_DRAFT_PET_H
#define AREA52_DRAFT_PET_H
#include <cstdint>
class Pet;
class Player;
bool Area52DraftPetSavePending(Pet const* pet);
bool Area52DraftPetTracked(Pet const* pet);
bool Area52WardrobePetReserved(Player const* player, std::uint32_t number);
bool Area52DraftPetBusy(Player const* player);
bool Area52DraftPetAuthorized(Player const* player, std::uint32_t number);
void UpdateArea52DraftPet(Player* player);
bool Area52DraftPetRewardReady(Player* player, std::uint64_t build, std::uint32_t revision);
void ConfirmArea52DraftPetReward(Player* player, std::uint64_t build, std::uint32_t revision);
#endif
