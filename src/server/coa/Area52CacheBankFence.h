#ifndef AREA52_CACHE_BANK_FENCE_H
#define AREA52_CACHE_BANK_FENCE_H

#include <cstdint>
#include <map>
#include <utility>

namespace Area52CacheBankFence
{
using Key = std::pair<std::uint8_t, std::uint64_t>;
class Fence
{
    struct Entry
    {
        std::uint64_t Pending = 0;
        bool Failed = false;
    };
    std::map<Key, Entry> entries;

public:
    void Begin(Key key) { ++entries[key].Pending; }
    void Complete(Key key, bool success)
    {
        auto found = entries.find(key);
        if (found == entries.end() || !found->second.Pending)
        {
            entries[key].Failed = true;
            return;
        }
        Entry& entry = found->second;
        --entry.Pending;
        entry.Failed = entry.Failed || !success;
        if (!entry.Pending && !entry.Failed)
            entries.erase(found);
    }
    bool Ready(Key key) const { return entries.find(key) == entries.end(); }
};
}
#endif
