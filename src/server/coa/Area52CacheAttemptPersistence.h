#ifndef AREA52_CACHE_ATTEMPT_PERSISTENCE_H
#define AREA52_CACHE_ATTEMPT_PERSISTENCE_H

#include "Area52CacheAttemptStorage.h"
#include "DatabaseEnvFwd.h"

class QueryCallback;

namespace Area52CacheAttempt
{
enum class ReadStatus { Unavailable, Missing, Found };
struct StoredRead
{
    ReadStatus status = ReadStatus::Unavailable;
    std::optional<State> state;
};

QueryCallback LoadStored(std::uint64_t cache);
StoredRead DecodeStored(QueryResult const& rows, std::uint64_t owner, std::uint64_t cache);
bool AppendStoredMutation(CharacterDatabaseTransaction transaction, std::optional<State> const& previous,
    State const& next);
}
#endif
