#ifndef AREA52_DRAFT_VANITY_H
#define AREA52_DRAFT_VANITY_H
#include <cstdint>
#include <utility>
#include <vector>
class Player;
using Area52DraftAppearanceRewards = std::vector<std::pair<std::uint32_t, std::uint32_t>>;
bool PrepareArea52DraftVanityReward(Player* player, std::uint64_t build,
    std::uint32_t revision, Area52DraftAppearanceRewards& rewards);
void ConfirmArea52DraftVanityReward(Player* player, Area52DraftAppearanceRewards const& rewards);
#endif
