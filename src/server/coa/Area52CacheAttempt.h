#ifndef AREA52_CACHE_ATTEMPT_H
#define AREA52_CACHE_ATTEMPT_H

#include <cstdint>
#include <set>
#include <vector>

namespace Area52CacheAttempt
{
enum class Phase : std::uint8_t { Choosing, Ready, Committing, Delivered };
enum class Result : std::uint8_t { Rejected, AwaitingSpace, CommitRequired, AlreadyDelivered, Changed };

struct Token
{
    std::uint32_t item = 0;
    std::uint32_t count = 0;
    bool operator==(Token const&) const = default;
};

struct State
{
    std::uint64_t owner = 0;
    std::uint64_t cache = 0;
    std::uint64_t request = 0;
    std::uint64_t revision = 0;
    std::uint32_t offered = 0;
    std::int32_t offeredRandomProperty = 0;
    std::uint32_t offeredCount = 1;
    std::uint32_t copper = 0;
    Phase phase = Phase::Choosing;
    std::set<std::uint32_t> declined;
    std::vector<Token> tokens;
};

inline bool Matches(State const& state, std::uint64_t owner, std::uint64_t cache,
    std::uint64_t request, std::uint64_t revision)
{
    return owner && cache && request && state.owner == owner && state.cache == cache
        && state.request == request && state.revision == revision;
}

inline Result Answer(State& state, std::uint64_t owner, std::uint64_t cache, std::uint64_t request,
    std::uint64_t revision, std::uint32_t offered, bool accept)
{
    if (!Matches(state, owner, cache, request, revision) || state.phase != Phase::Choosing
        || !offered || offered != state.offered)
        return Result::Rejected;
    if (accept)
        state.phase = Phase::Ready;
    else
    {
        state.declined.insert(offered);
        state.offered = 0;
        state.offeredRandomProperty = 0;
        state.offeredCount = 1;
    }
    ++state.revision;
    return Result::Changed;
}

inline Result BeginDelivery(State& state, std::uint64_t owner, std::uint64_t cache,
    std::uint64_t request, std::uint64_t revision, bool eligible, bool bagSpace)
{
    if (!Matches(state, owner, cache, request, revision))
        return Result::Rejected;
    if (state.phase == Phase::Delivered)
        return Result::AlreadyDelivered;
    if (state.phase != Phase::Ready || !state.offered || !eligible)
        return Result::Rejected;
    if (!bagSpace)
        return Result::AwaitingSpace;
    state.phase = Phase::Committing;
    ++state.revision;
    return Result::CommitRequired;
}

inline Result ConfirmCommit(State& state, std::uint64_t request, std::uint64_t revision)
{
    if (!request || request != state.request || revision != state.revision || state.phase != Phase::Committing)
        return Result::Rejected;
    state.phase = Phase::Delivered;
    ++state.revision;
    return Result::Changed;
}
}

#endif
