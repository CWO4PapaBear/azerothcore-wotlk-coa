#ifndef AREA52_DRAFT_LIFECYCLE_H
#define AREA52_DRAFT_LIFECYCLE_H
#include <cstdint>
namespace Area52Build
{
struct DraftLifecycle
{
    std::uint64_t Build = 0;
    std::uint32_t Revision = 0;
    bool Forfeited = false;
    bool Rewarded = false;
    bool EntitlementForfeited = false;
};
inline DraftLifecycle SwitchFeaturedDraft(DraftLifecycle state, std::uint32_t level,
    std::uint64_t build, std::uint32_t revision, bool featured)
{
    if (featured && level < 10)
    {
        state.Forfeited = false;
        state.EntitlementForfeited = false;
    }
    if (!featured || (level >= 10 && (state.Build != build || state.Revision != revision)))
        state.Forfeited = true;
    if (!featured)
        state.EntitlementForfeited = true;
    state.Build = featured ? build : 0;
    state.Revision = featured ? revision : 0;
    return state;
}
inline DraftLifecycle CustomizeDraft(DraftLifecycle state)
{
    state.Build = 0;
    state.Revision = 0;
    state.Forfeited = true;
    state.EntitlementForfeited = true;
    return state;
}
inline bool DraftRewardDue(DraftLifecycle const& state, std::uint32_t level, std::uint32_t maximum)
{
    return state.Build && state.Revision && !state.Forfeited && !state.Rewarded && !state.EntitlementForfeited
        && maximum >= 10 && level >= maximum;
}
}
#endif
