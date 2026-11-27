#ifndef AREA52_DRAGON_REVIEW_RULES_H
#define AREA52_DRAGON_REVIEW_RULES_H
#include <algorithm>
#include <cstdint>
namespace Area52DragonReview
{
inline float BloodthirstBleedMultiplier(std::uint32_t ownedBleeds)
{
    return 1.0f + 0.08f * std::min(ownedBleeds, 5u);
}
inline bool BurningRushAllowsMovement(bool area52, bool active, std::uint32_t school)
{
    return area52 && active && (school & 4u);
}
inline std::uint32_t BurningRushDrain(std::uint32_t maximumHealth)
{
    return std::uint64_t(maximumHealth) * 5u / 100u;
}
}
#endif
