#pragma once

#include <algorithm>

namespace Area52TemporaryPhysicalDamage
{
inline bool IsExclusiveBonus(unsigned spellId)
{
    switch (spellId)
    {
        case 31665:
        case 19574:
        case 277875:
        case 886395:
        case 992391:
        case 1436395:
            return true;
        default:
            return false;
    }
}

template<class Range, class Predicate>
float Normalization(Range const& effects, Predicate const& eligible)
{
    float product = 1.0f;
    float strongest = 1.0f;
    for (auto const* effect : effects)
    {
        if (!IsExclusiveBonus(effect->GetId()) || effect->GetAmount() <= 0 || !eligible(effect))
            continue;
        float const multiplier = 1.0f + float(effect->GetAmount()) / 100.0f;
        product *= multiplier;
        strongest = std::max(strongest, multiplier);
    }
    return strongest / product;
}
}
