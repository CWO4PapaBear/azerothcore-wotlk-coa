#ifndef AREA52_CACHE_BAG_PLAN_H
#define AREA52_CACHE_BAG_PLAN_H

#include <algorithm>
#include <cstdint>
#include <set>
#include <vector>

namespace Area52CacheBagPlan
{
struct Stack
{
    std::uint32_t item = 0;
    std::int32_t randomProperty = 0;
    std::uint32_t count = 0;
    std::uint32_t maximum = 0;
};

struct Slot
{
    std::uint16_t position = 0;
    Stack held;
    bool available = false;
};

struct Placement
{
    std::size_t reward;
    std::uint16_t position;
    std::uint32_t count;
};

struct Result
{
    bool complete = false;
    std::vector<Placement> placements;
};

template <typename Accepts>
Result Plan(std::vector<Stack> const& rewards, std::vector<Slot> slots, Accepts accepts)
{
    Result result;
    if (rewards.empty())
        return result;
    std::set<std::uint16_t> positions;
    for (Slot const& slot : slots)
        if (!positions.insert(slot.position).second)
            return result;
    for (std::size_t index = 0; index < rewards.size(); ++index)
    {
        Stack const& reward = rewards[index];
        if (!reward.item || !reward.count || !reward.maximum)
            return {};
        std::uint32_t remaining = reward.count;
        for (unsigned pass = 0; pass < 2 && remaining; ++pass)
            for (Slot& slot : slots)
            {
                if (!remaining)
                    break;
                if (!slot.available || !accepts(slot.position, reward))
                    continue;
                bool const occupied = slot.held.count != 0;
                if (occupied != (pass == 0))
                    continue;
                if (occupied && (slot.held.item != reward.item
                    || slot.held.randomProperty != reward.randomProperty))
                    continue;
                if (occupied && slot.held.maximum != reward.maximum)
                    return {};
                if (slot.held.count >= reward.maximum)
                    continue;
                std::uint32_t const moved = std::min(remaining, reward.maximum - slot.held.count);
                if (!occupied)
                    slot.held = { reward.item, reward.randomProperty, 0, reward.maximum };
                slot.held.count += moved;
                remaining -= moved;
                result.placements.push_back({ index, slot.position, moved });
            }
        if (remaining)
            return {};
    }
    result.complete = true;
    return result;
}
}

#endif
