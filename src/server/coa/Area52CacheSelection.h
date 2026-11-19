#ifndef AREA52_CACHE_SELECTION_H
#define AREA52_CACHE_SELECTION_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <set>
#include <map>
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
    bool raidSource = false;
    bool usable = false;
    bool bonusesQualified = false;
    bool owned = false;
    bool appearanceOwned = false;
    bool improvesSlot = true;
    std::uint32_t replacementWeight = 1;
};

struct Choice
{
    std::uint32_t item;
    bool requiresAppearanceChoice;
    std::uint32_t weight = 1;
};

struct Result
{
    Outcome outcome = Outcome::NoEligibleReward;
    std::vector<Choice> choices;
};

inline bool Available(Candidate const& candidate, std::uint32_t realmCap)
{
    return candidate.item && candidate.requiredLevel == realmCap && candidate.itemLevel
        && candidate.contentUnlocked && candidate.usable && candidate.bonusesQualified;
}

inline float RewardFloor(std::vector<Candidate> const& catalog, std::uint32_t realmCap)
{
    std::uint32_t lowest = UINT32_MAX;
    for (Candidate const& candidate : catalog)
        if (Available(candidate, realmCap))
            lowest = std::min(lowest, candidate.itemLevel);
    return lowest == UINT32_MAX ? 0.0f : float(lowest);
}

inline float CallboardAverage(std::vector<Candidate> const& catalog, float equipped, std::uint32_t realmCap)
{
    return std::max(equipped, RewardFloor(catalog, realmCap));
}

struct RewardRange
{
    double minimum;
    double maximum;
};

inline RewardRange ProgressionRange(std::vector<Candidate> const& catalog, double equipped,
    std::uint32_t realmCap, std::uint32_t allowance)
{
    RewardRange range{std::max(equipped, double(RewardFloor(catalog, realmCap))), 0};
    range.maximum = range.minimum + allowance;
    std::uint32_t firstRaid = UINT32_MAX;
    for (Candidate const& candidate : catalog)
        if (Available(candidate, realmCap) && candidate.raidSource)
            firstRaid = std::min(firstRaid, candidate.itemLevel);
    if (firstRaid != UINT32_MAX && range.minimum < firstRaid)
        range.maximum = std::max(range.maximum, double(firstRaid));
    bool hasUpgrade = false;
    std::uint32_t nextLevel = UINT32_MAX;
    for (Candidate const& candidate : catalog)
        if (Available(candidate, realmCap) && !candidate.owned && candidate.improvesSlot && candidate.itemLevel > range.minimum)
        {
            if (candidate.itemLevel <= range.maximum)
                hasUpgrade = true;
            else
                nextLevel = std::min(nextLevel, candidate.itemLevel);
        }
    if (!hasUpgrade && nextLevel != UINT32_MAX)
        range.maximum = nextLevel;
    if (realmCap == 70 || realmCap == 80)
    {
        std::uint32_t ceiling = UINT32_MAX;
        for (Candidate const& candidate : catalog)
            if (Available(candidate, realmCap) && !candidate.owned && candidate.improvesSlot && candidate.itemLevel >= range.maximum)
                ceiling = std::min(ceiling, candidate.itemLevel);
        if (ceiling != UINT32_MAX)
            range.maximum = ceiling;
    }
    return range;
}

inline bool BonusFromRequest(std::uint64_t request)
{
    return request && request % 10 == 0;
}

inline RewardRange OfferRange(std::vector<Candidate> const& catalog, double equipped,
    std::uint32_t realmCap, std::uint32_t allowance, std::uint64_t request)
{
    auto range = ProgressionRange(catalog, equipped, realmCap, allowance);
    if (!BonusFromRequest(request))
        return range;
    std::uint32_t nextLevel = UINT32_MAX;
    for (Candidate const& candidate : catalog)
        if (Available(candidate, realmCap) && !candidate.owned && candidate.improvesSlot && candidate.itemLevel > range.maximum)
            nextLevel = std::min(nextLevel, candidate.itemLevel);
    if (nextLevel != UINT32_MAX)
        range = {double(nextLevel), double(nextLevel)};
    return range;
}

inline std::vector<Choice> WeightedChoices(std::vector<Candidate const*> const& candidates)
{
    using Rank = std::pair<Category, std::array<std::uint32_t, 4>>;
    auto better = [](Rank const& left, Rank const& right)
    {
        return left.first != right.first ? left.first < right.first : left.second > right.second;
    };
    std::map<Rank, std::uint32_t, decltype(better)> ranks(better);
    for (Candidate const* candidate : candidates)
        ranks.try_emplace({candidate->category, candidate->suitability}, 0);
    std::uint32_t weight = std::uint32_t(ranks.size());
    for (auto& [rank, value] : ranks)
        value = weight--;
    std::vector<Choice> choices;
    std::set<std::uint32_t> seen;
    for (Candidate const* candidate : candidates)
        if (seen.insert(candidate->item).second)
            choices.push_back({candidate->item, candidate->appearanceOwned,
                ranks.at({candidate->category, candidate->suitability})
                    * std::clamp(candidate->replacementWeight, 1u, 3u)});
    return choices;
}

inline Result Select(std::vector<Candidate> const& catalog, double average,
    double maximum, std::set<std::uint32_t> const& declined, std::uint32_t realmCap,
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
        if (!Available(candidate, realmCap))
            continue;
        anyEligible = true;
        if (candidate.owned)
            continue;
        anyUnowned = true;
        if (!candidate.improvesSlot)
            continue;
        bool const weakestSlotUpgrade = candidate.replacementWeight > 1 && average < maximum;
        if ((!weakestSlotUpgrade && candidate.itemLevel < average) || double(candidate.itemLevel) > maximum)
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
    result.choices = WeightedChoices(candidates);
    if (!result.choices.empty())
        result.outcome = Outcome::ChooseReward;
    else if (anyInRange)
        result.outcome = Outcome::AllDeclined;
    return result;
}
}

#endif
