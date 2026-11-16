#ifndef AREA52_FEIGN_DEATH_H
#define AREA52_FEIGN_DEATH_H

#include "AscensionFreePick.h"
#include "Creature.h"
#include "Map.h"
#include "Player.h"

namespace Area52FeignDeath
{
inline bool IsRaidBossException(Unit const* attacker, Unit const* target)
{
    Creature const* boss = attacker ? attacker->ToCreature() : nullptr;
    return boss && target && AscensionFreePick::Applies(target->ToPlayer()) &&
        boss->GetMap() && boss->GetMap()->IsRaid() && !boss->GetCharmerOrOwnerGUID().IsPlayer() &&
        (boss->isWorldBoss() || boss->IsDungeonBoss());
}

inline bool SeesThrough(Unit const* attacker, Unit const* target)
{
    return target && target->HasAura(5384) && IsRaidBossException(attacker, target);
}
}

#endif
