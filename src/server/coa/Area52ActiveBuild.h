#ifndef AREA52_ACTIVE_BUILD_H
#define AREA52_ACTIVE_BUILD_H

#include <cstdint>
#include <array>
#include <optional>
#include <string>
class Player;
class Creature;
namespace Area52Build
{
struct Entry;
struct CacheBuildContext
{
    bool Ready = false;
    std::string Build;
    std::uint32_t Revision = 0;
    std::uint32_t Roles = 0;
    std::uint32_t PrimaryStat = 0;
};
CacheBuildContext GetCacheBuildContext(Player* player);
struct DraftVanityContext
{
    std::uint64_t Build = 0;
    std::uint32_t Revision = 0;
    std::uint32_t Archetype = 0;
    bool Ready = false;
    std::array<std::uint32_t, 4> Incarnations{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
    std::uint32_t Pet = UINT32_MAX;
};
std::optional<std::uint32_t> GetReadyFeaturedArchetype(Player* player);
DraftVanityContext GetDraftVanityContext(Player* player);
void PauseAutoLearnForReset(Player* player);
bool BeforeManualDraftChange(Player* player);
bool IsFeaturedDraft(Player* player);
bool CanSwitchSpecialization(Player* player);
void RestoreSpecialization(Player* player);
std::uint32_t Train(Player* player, Creature* trainer, std::uint32_t maximumLevel = 80);
}
void SendArea52Advancement(Player* player);
void SendArea52Specialization(Player* player);
#endif
