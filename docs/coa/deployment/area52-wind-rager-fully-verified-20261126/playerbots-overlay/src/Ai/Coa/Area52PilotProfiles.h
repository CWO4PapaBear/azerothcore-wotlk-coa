#ifndef PLAYERBOTS_AREA52_PILOT_PROFILES_H
#define PLAYERBOTS_AREA52_PILOT_PROFILES_H
#include "Area52FixedEquipment.h"
#include <array>
#include <cstdint>
#include <string_view>

namespace Area52PilotProfiles
{
struct Profile
{
    unsigned archetype;
    std::string_view spec;
    std::uint32_t legendary;
    unsigned role;
    std::uint32_t vanity;
    std::array<std::uint32_t,19> equipment;
    bool recruitable = true;
};

inline constexpr std::array Profiles{
    Profile{86,"defias-gunslinger",997047,3,900586,
        {1339121,1339076,1642952,0,1648689,1339093,1339114,1339100,1339107,1339128,
         50255,1339079,1642991,1642991,1339088,1648716,0,1644093,0}},
    Profile{27,"mknight",81116,1,900527,Area52FixedEquipment::Profiles[0]},
    Profile{1,"naturalist",965937,2,900501,Area52FixedEquipment::Profiles[1]},
    Profile{73,"hoplite",978217,3,900573,Area52FixedEquipment::Profiles[2]},
    Profile{20,"bulwark",978724,1,900520,
        {1339126,1339078,3142949,0,3148685,1339098,1339119,1339105,1339112,1339133,
         1339081,1339081,1642991,1642992,1339087,1999520,1999521,1642900,0}},
    Profile{133,"white-walker",978202,3,900633,
        {1339123,1339077,1642951,0,1648683,1339095,1339116,1339102,1339109,1339130,
         1339080,1339080,1642992,1642991,1339090,1642947,0,1642901,0}},
    Profile{82,"marksman",979740,3,900582,
        {1339124,1339076,1642950,0,1648677,1339096,1339117,1339103,1339110,1339131,
         1339082,1339079,1642991,1642992,1339084,3142943,0,1642946,0}},
    Profile{13,"cryotherapist",86576,2,900513,
        {1339123,1339077,1642951,0,1648683,1339095,1339116,1339102,1339109,1339130,
         1339083,1339083,1642991,1642992,1339090,1642947,0,1642901,0}}
};

inline Profile const* FindRegistered(std::uint32_t legendary)
{
    for (auto const& profile : Profiles)
        if (profile.legendary == legendary)
            return &profile;
    return nullptr;
}

inline Profile const* Find(std::uint32_t legendary)
{
    auto profile = FindRegistered(legendary);
    return profile && profile->recruitable ? profile : nullptr;
}

inline Profile const* Select(unsigned role, std::uint32_t current, std::uint64_t bot)
{
    auto existing = Find(current);
    if (existing && existing->role == role)
        return existing;
    std::array<Profile const*,Profiles.size()> candidates{};
    unsigned count = 0;
    for (auto const& profile : Profiles)
        if (profile.recruitable && profile.role == role)
            candidates[count++] = &profile;
    return count ? candidates[bot % count] : nullptr;
}
}
#endif
