#ifndef AREA52_FEATURED_PETS_H
#define AREA52_FEATURED_PETS_H
#include <cstdint>
namespace Area52Build
{
inline std::uint32_t FeaturedHunterPet(std::uint32_t selection, std::uint32_t archetype)
{
    if (selection == UINT32_MAX)
    {
        switch (archetype)
        {
            case 17: return 1042;
            case 84: return 24747;
            case 85: return 35189;
            case 139: return 9906554;
            default: return 0;
        }
    }
    switch (selection)
    {
        case 1042: return 1042;
        case 24747: return 24747;
        case 35189: return 35189;
        case 6554: return 9906554;
        default: return 0;
    }
}
inline std::uint32_t FeaturedPetAppearance(std::uint32_t selection)
{
    switch (selection)
    {
        case 74491: return 61374;
        case 16941: return 47677;
        case 18683: return 51229;
        case 80854: return 51230;
        default: return 0;
    }
}
inline bool ValidFeaturedPet(std::uint32_t selection)
{
    return selection == 0 || selection == UINT32_MAX
        || FeaturedHunterPet(selection, 0) || FeaturedPetAppearance(selection);
}
}
#endif
