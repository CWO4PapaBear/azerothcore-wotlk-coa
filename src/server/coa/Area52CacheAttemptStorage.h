#ifndef AREA52_CACHE_ATTEMPT_STORAGE_H
#define AREA52_CACHE_ATTEMPT_STORAGE_H

#include "Area52CacheAttempt.h"
#include <bit>
#include <limits>
#include <optional>

namespace Area52CacheAttempt
{
inline bool Valid(State const& state)
{
    if (!state.owner || !state.cache || !state.request || state.revision == UINT64_MAX
        || unsigned(state.phase) > unsigned(Phase::Delivered) || state.declined.size() > 10000
        || state.tokens.size() > 64 || (state.phase != Phase::Choosing && !state.offered)
        || (state.offered && state.declined.contains(state.offered)) || state.declined.contains(0)
        || (!state.offered && state.offeredRandomProperty) || !state.offeredCount || state.offeredCount > 10000)
        return false;
    for (Token const& token : state.tokens)
        if (!token.item || !token.count || token.count > 10000)
            return false;
    return true;
}

inline std::optional<std::vector<std::uint8_t>> Encode(State const& state)
{
    if (!Valid(state))
        return std::nullopt;
    std::vector<std::uint8_t> bytes;
    auto put = [&](std::uint64_t value, unsigned size)
    {
        for (unsigned index = 0; index < size; ++index)
            bytes.push_back(std::uint8_t(value >> (index * 8)));
    };
    put(0x33414341, 4);
    put(state.owner, 8);
    put(state.cache, 8);
    put(state.request, 8);
    put(state.revision, 8);
    put(state.offered, 4);
    put(std::bit_cast<std::uint32_t>(state.offeredRandomProperty), 4);
    put(unsigned(state.phase), 1);
    put(state.declined.size(), 4);
    for (auto item : state.declined)
        put(item, 4);
    put(state.tokens.size(), 4);
    for (Token const& token : state.tokens)
    {
        put(token.item, 4);
        put(token.count, 4);
    }
    put(state.offeredCount, 4);
    put(state.copper, 4);
    return bytes;
}

inline std::optional<State> Decode(std::vector<std::uint8_t> const& bytes)
{
    if (bytes.size() > 41000)
        return std::nullopt;
    std::size_t position = 0;
    bool good = true;
    auto get = [&](unsigned size)
    {
        std::uint64_t value = 0;
        if (bytes.size() - position < size)
        {
            good = false;
            return value;
        }
        for (unsigned index = 0; index < size; ++index)
            value |= std::uint64_t(bytes[position++]) << (index * 8);
        return value;
    };
    if (get(4) != 0x33414341)
        return std::nullopt;
    State state;
    state.owner = get(8);
    state.cache = get(8);
    state.request = get(8);
    state.revision = get(8);
    state.offered = std::uint32_t(get(4));
    state.offeredRandomProperty = std::bit_cast<std::int32_t>(std::uint32_t(get(4)));
    auto phase = get(1);
    if (!good || phase > unsigned(Phase::Delivered))
        return std::nullopt;
    state.phase = Phase(phase);
    auto declined = get(4);
    if (!good || declined > 10000)
        return std::nullopt;
    for (std::uint64_t index = 0; index < declined; ++index)
        if (!state.declined.insert(std::uint32_t(get(4))).second)
            return std::nullopt;
    auto tokens = get(4);
    if (!good || tokens > 64)
        return std::nullopt;
    for (std::uint64_t index = 0; index < tokens; ++index)
    {
        std::uint32_t item = std::uint32_t(get(4));
        std::uint32_t count = std::uint32_t(get(4));
        state.tokens.push_back({ item, count });
    }
    state.offeredCount = std::uint32_t(get(4));
    state.copper = std::uint32_t(get(4));
    if (!good || position != bytes.size() || !Valid(state))
        return std::nullopt;
    return state;
}
}

#endif
