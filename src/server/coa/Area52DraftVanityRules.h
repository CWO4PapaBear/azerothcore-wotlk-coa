#ifndef AREA52_DRAFT_VANITY_RULES_H
#define AREA52_DRAFT_VANITY_RULES_H

#include <cstdint>
#include <map>
#include <set>

namespace Area52Build
{
struct DraftVanityLease
{
    std::uint64_t Build = 0;
    std::uint32_t Revision = 0;
    std::map<std::uint32_t, std::uint32_t> Applied;
    std::map<std::uint32_t, std::uint32_t> Previous;

    bool Contains(std::uint32_t appearance) const
    {
        for (auto const& [category, id] : Applied)
            if (id == appearance)
                return true;
        return false;
    }

    template<class Appearances, class Ownership>
    bool Synchronize(std::uint64_t build, std::uint32_t revision,
        std::map<std::uint32_t, std::uint32_t> const& desired,
        Appearances& active, Ownership const& permanent)
    {
        if (Build == build && Revision == revision && Applied == desired)
            return false;
        for (auto const& [category, appearance] : Applied)
            if (active[category] == appearance && !permanent.contains(appearance))
            {
                auto const previous = Previous.at(category);
                active[category] = permanent.contains(previous) ? previous : 0;
            }
        Applied = desired;
        Previous.clear();
        Build = build;
        Revision = revision;
        for (auto const& [category, appearance] : Applied)
        {
            Previous[category] = active[category];
            active[category] = appearance;
        }
        return true;
    }

    template<class Ownership>
    std::uint32_t PersistentAppearance(std::uint32_t category,
        std::uint32_t active, Ownership const& permanent) const
    {
        if (!active || permanent.contains(active))
            return active;
        auto const applied = Applied.find(category);
        if (applied == Applied.end() || applied->second != active)
            return 0;
        auto const previous = Previous.at(category);
        return permanent.contains(previous) ? previous : 0;
    }
};
}
#endif
