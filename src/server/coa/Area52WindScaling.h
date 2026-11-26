#ifndef AREA52_WIND_SCALING_H
#define AREA52_WIND_SCALING_H

#include <cstdint>

namespace Area52Wind
{
inline unsigned StrikeKind(std::uint32_t id)
{
    if (id == 978946 || id == 978947 || (id >= 979927 && id <= 979940))
        return 1;
    if (id == 983983 || id == 983984 || (id >= 984073 && id <= 984076) ||
        (id >= 984083 && id <= 984086))
        return 2;
    return 0;
}

inline bool OffhandStrike(std::uint32_t id)
{
    return id == 978947 || (id >= 979934 && id <= 979940) || id == 983984 ||
        (id >= 984083 && id <= 984086);
}

inline bool Coefficients(std::uint32_t id, float& spellPower, float& attackPower)
{
    unsigned const kind = StrikeKind(id);
    if (kind)
    {
        spellPower = kind == 2 ? 0.9f : 0.0f;
        attackPower = kind == 2 ? 0.9f : 0.54f;
        return true;
    }
    if (id == 878939)
    {
        spellPower = 0.18f / 8.0f;
        attackPower = 0.2f / 8.0f;
        return true;
    }
    if (id == 997083 || (id >= 997085 && id <= 997097))
    {
        spellPower = id == 997083 ? 0.125f : id == 997085 ? 0.317f : id == 997086 ? 0.561f : 0.714f;
        attackPower = 0.0f;
        return true;
    }
    return false;
}

inline std::int32_t TornadoBaseTick(std::int32_t total, unsigned tick)
{
    if (total <= 0 || tick < 1 || tick > 8)
        return 0;
    return total / 8 + (tick <= unsigned(total % 8) ? 1 : 0);
}
}

#endif
