#ifndef AREA52_CACHE_ITEM_LEVEL_H
#define AREA52_CACHE_ITEM_LEVEL_H

#include "Item.h"
#include "Player.h"

namespace Area52CacheItemLevel
{
inline float Equipped(Player const* player)
{
    double sum = 0;
    uint32 count = 0;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_RANGED
            || slot == EQUIPMENT_SLOT_OFFHAND || slot == EQUIPMENT_SLOT_BODY)
            continue;
        ++count;
        if (Item const* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            if (ItemTemplate const* proto = item->GetTemplate())
                if (proto->Quality != ITEM_QUALITY_HEIRLOOM)
                    sum += proto->ItemLevel;
    }
    return count ? float(sum / count) : 0.0f;
}
}

#endif
