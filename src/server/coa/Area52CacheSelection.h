#ifndef AREA52_CACHE_SELECTION_H
#define AREA52_CACHE_SELECTION_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <set>
#include <vector>

namespace Area52CacheSelection
{
enum class Category : std::uint8_t { Weapon, Armor, Accessory };
enum class Outcome : std::uint8_t { ChooseReward, AllOwned, NoEligibleReward, AllDeclined };

struct Candidate
{
    std::uint32_t item = 0;
    std::uint32_t itemLevel = 0;
    std::uint32_t requiredLevel = 0;
    std::uint32_t tier = 0;
    Category category = Category::Accessory;
    std::array<std::uint32_t, 4> suitability{};
    bool contentUnlocked = false;
    bool usable = false;
    bool bonusesQualified = false;
    bool owned = false;
    bool appearanceOwned = false;
};

struct Choice
{
    std::uint32_t item;
    bool requiresAppearanceChoice;
};

struct Result
{
    Outcome outcome = Outcome::NoEligibleReward;
    std::vector<Choice> choices;
};

inline Result Select(std::vector<Candidate> const& catalog, std::uint32_t average,
    std::uint32_t allowance, std::set<std::uint32_t> const& declined, std::uint32_t realmCap,
    bool catalogAndOwnershipComplete = false)
{
    Result result;
    if (realmCap != 60 && realmCap != 70 && realmCap != 80)
        return result;
    bool anyEligible = false;
    bool anyUnowned = false;
    bool anyInRange = false;
    std::vector<Candidate const*> candidates;
    for (Candidate const& candidate : catalog)
    {
        if (!candidate.item || candidate.requiredLevel != realmCap || !candidate.contentUnlocked
            || !candidate.usable || !candidate.bonusesQualified)
            continue;
        anyEligible = true;
        if (candidate.owned)
            continue;
        anyUnowned = true;
        if (candidate.itemLevel < average || std::uint64_t(candidate.itemLevel) > std::uint64_t(average) + allowance)
            continue;
        anyInRange = true;
        if (!declined.contains(candidate.item))
            candidates.push_back(&candidate);
    }
    if (anyEligible && !anyUnowned)
    {
        result.outcome = catalogAndOwnershipComplete ? Outcome::AllOwned : Outcome::NoEligibleReward;
        return result;
    }
    if (!candidates.empty())
    {
        auto better = [](Candidate const* left, Candidate const* right)
        {
            if (left->category != right->category)
                return left->category < right->category;
            return left->suitability > right->suitability;
        };
        Candidate const* best = *std::min_element(candidates.begin(), candidates.end(), better);
        std::set<std::uint32_t> seen;
        for (Candidate const* candidate : candidates)
            if (candidate->category == best->category && candidate->suitability == best->suitability
                && seen.insert(candidate->item).second)
                result.choices.push_back({ candidate->item, candidate->appearanceOwned });
    }
    if (!result.choices.empty())
        result.outcome = Outcome::ChooseReward;
    else if (anyInRange)
        result.outcome = Outcome::AllDeclined;
    return result;
}
}

#endif
