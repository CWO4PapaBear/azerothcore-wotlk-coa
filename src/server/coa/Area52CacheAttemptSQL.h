#ifndef AREA52_CACHE_ATTEMPT_SQL_H
#define AREA52_CACHE_ATTEMPT_SQL_H

#include "Area52CacheAttemptStorage.h"
#include <algorithm>
#include <string>

namespace Area52CacheAttempt
{
inline std::string Hex(std::vector<std::uint8_t> const& bytes)
{
    constexpr char digits[] = "0123456789ABCDEF";
    std::string result;
    result.reserve(bytes.size() * 2);
    for (auto byte : bytes)
    {
        result += digits[byte >> 4];
        result += digits[byte & 15];
    }
    return result;
}

inline std::optional<std::vector<std::string>> Mutation(std::optional<State> const& previous, State const& next)
{
    auto bytes = Encode(next);
    if (!bytes)
        return std::nullopt;
    std::string const payload = "X'" + Hex(*bytes) + "'";
    auto number = [](auto value) { return std::to_string(value); };
    if (!previous)
    {
        if (next.revision || (next.phase != Phase::Choosing && next.phase != Phase::Ready))
            return std::nullopt;
        return std::vector<std::string>{
            "INSERT INTO mod_area52_cache_attempt (cache_guid,owner_guid,request_id,revision,phase,payload) VALUES ("
            + number(next.cache) + "," + number(next.owner) + "," + number(next.request) + ",0,"
            + number(unsigned(next.phase)) + "," + payload + ")"};
    }
    auto before = Encode(*previous);
    if (!before || previous->owner != next.owner || previous->cache != next.cache
        || previous->request != next.request || next.revision != previous->revision + 1
        || previous->tokens != next.tokens || previous->copper != next.copper || previous->phase == Phase::Delivered
        || (previous->offered == next.offered && (previous->offeredRandomProperty != next.offeredRandomProperty
            || previous->offeredCount != next.offeredCount)))
        return std::nullopt;
    if (previous->phase == Phase::Choosing)
    {
        if (next.phase != Phase::Choosing && next.phase != Phase::Ready)
            return std::nullopt;
        if (!previous->offered)
        {
            if (!next.offered || next.declined != previous->declined)
                return std::nullopt;
        }
        else if (next.phase == Phase::Ready)
        {
            if (next.offered != previous->offered || next.declined != previous->declined)
                return std::nullopt;
        }
        else
        {
            auto declined = previous->declined;
            declined.insert(previous->offered);
            if (next.offered || next.declined != declined)
                return std::nullopt;
        }
    }
    else if (next.offered != previous->offered || next.declined != previous->declined
        || (previous->phase == Phase::Ready && next.phase != Phase::Committing)
        || (previous->phase == Phase::Committing && next.phase != Phase::Delivered))
        return std::nullopt;
    std::string const request = number(next.request);
    return std::vector<std::string>{
        "UPDATE mod_area52_cache_attempt SET revision=" + number(next.revision) + ",phase="
        + number(unsigned(next.phase)) + ",payload=" + payload + " WHERE cache_guid=" + number(next.cache)
        + " AND owner_guid=" + number(next.owner) + " AND request_id=" + request + " AND revision="
        + number(previous->revision) + " AND payload=X'" + Hex(*before) + "'",
        "INSERT INTO mod_area52_cache_attempt_guard (request_id,matched_rows) VALUES (" + request + ",ROW_COUNT())",
        "DELETE FROM mod_area52_cache_attempt_guard WHERE request_id=" + request};
}
}
#endif
