#ifndef AREA52_STRENGTH_FAITH_H
#define AREA52_STRENGTH_FAITH_H

namespace Area52StrengthFaith
{
inline float Multiplier(bool ability, unsigned school, int physical, int holy)
{
    if (!ability)
        return 1.0f;
    float result = 1.0f;
    if (school & 1u)
        result *= 1.0f + physical / 100.0f;
    if (school & 2u)
        result *= 1.0f + holy / 100.0f;
    return result;
}
}

#endif
