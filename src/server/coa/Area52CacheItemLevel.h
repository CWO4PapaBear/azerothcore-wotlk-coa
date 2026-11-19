#ifndef AREA52_CACHE_ITEM_LEVEL_H
#define AREA52_CACHE_ITEM_LEVEL_H

#include "Item.h"
#include "Player.h"

namespace Area52CacheItemLevel
{
inline float Equipped(Player const* player, float heirloomFloor = 0.0f)
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
                sum += proto->Quality == ITEM_QUALITY_HEIRLOOM ? heirloomFloor : proto->ItemLevel;
    }
    return count ? float(sum / count) : 0.0f;
}

inline uint32 SlotGroup(uint32 inventoryType)
{
    switch (inventoryType)
    {
        case INVTYPE_ROBE: return INVTYPE_CHEST;
        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONMAINHAND: return INVTYPE_WEAPONMAINHAND;
        case INVTYPE_RANGED: case INVTYPE_RANGEDRIGHT:
        case INVTYPE_THROWN: case INVTYPE_RELIC: return INVTYPE_RANGED;
        default: return inventoryType;
    }
}

inline bool HandItem(uint32 type)
{
    return type == INVTYPE_WEAPON || type == INVTYPE_WEAPONMAINHAND || type == INVTYPE_WEAPONOFFHAND
        || type == INVTYPE_2HWEAPON || type == INVTYPE_SHIELD || type == INVTYPE_HOLDABLE;
}

inline ItemTemplate const* EquippedTemplate(Player const* player, uint8 slot)
{
    Item const* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    return item ? item->GetTemplate() : nullptr;
}

inline bool OffhandFitsMain(Player const* player, ItemTemplate const* main, ItemTemplate const* off)
{
    if (!main)
        return true;
    bool const restricted = main->InventoryType == INVTYPE_2HWEAPON
        || main->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || main->SubClass == ITEM_SUBCLASS_WEAPON_STAFF
        || main->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
    return !restricted || player->CanTitanGrip(main) || player->CanUseTwoHandWithShield(main, off);
}

inline bool MatchesReplacement(Player const* player, ItemTemplate const* reward, uint8 slot,
    ItemTemplate const* equipped)
{
    uint32 const type = reward->InventoryType;
    if (!HandItem(type))
        return slot != EQUIPMENT_SLOT_MAINHAND && slot != EQUIPMENT_SLOT_OFFHAND
            && SlotGroup(type) == SlotGroup(equipped->InventoryType);
    auto const* main = EquippedTemplate(player, EQUIPMENT_SLOT_MAINHAND);
    auto const* off = EquippedTemplate(player, EQUIPMENT_SLOT_OFFHAND);
    if (slot == EQUIPMENT_SLOT_MAINHAND)
    {
        if (equipped->InventoryType == INVTYPE_2HWEAPON)
            return type == INVTYPE_2HWEAPON && (!off || OffhandFitsMain(player, reward, off));
        return (equipped->InventoryType == INVTYPE_WEAPON || equipped->InventoryType == INVTYPE_WEAPONMAINHAND)
            && (type == INVTYPE_WEAPON || type == INVTYPE_WEAPONMAINHAND);
    }
    if (slot != EQUIPMENT_SLOT_OFFHAND || !OffhandFitsMain(player, main, reward))
        return false;
    if (equipped->InventoryType == INVTYPE_SHIELD || equipped->InventoryType == INVTYPE_HOLDABLE)
        return type == equipped->InventoryType;
    if (!player->CanDualWield())
        return false;
    if (equipped->InventoryType == INVTYPE_2HWEAPON)
        return type == INVTYPE_2HWEAPON && player->CanTitanGrip(reward);
    return (equipped->InventoryType == INVTYPE_WEAPON || equipped->InventoryType == INVTYPE_WEAPONOFFHAND)
        && (type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND)
        && reward->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM
        && reward->SubClass != ITEM_SUBCLASS_WEAPON_FISHING_POLE;
}

inline bool ImprovesSlot(Player const* player, ItemTemplate const* reward, float floor)
{
    double replacement = UINT32_MAX;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;
        Item const* equipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        ItemTemplate const* item = equipped ? equipped->GetTemplate() : nullptr;
        if (!item)
            continue;
        if (MatchesReplacement(player, reward, slot, item))
            replacement = std::min(replacement,
                item->Quality == ITEM_QUALITY_HEIRLOOM ? double(floor) : double(item->ItemLevel));
    }
    if (replacement == UINT32_MAX && HandItem(reward->InventoryType))
        return !EquippedTemplate(player, EQUIPMENT_SLOT_MAINHAND)
            && !EquippedTemplate(player, EQUIPMENT_SLOT_OFFHAND)
            && (reward->InventoryType == INVTYPE_WEAPON || reward->InventoryType == INVTYPE_WEAPONMAINHAND
                || reward->InventoryType == INVTYPE_2HWEAPON);
    return replacement == UINT32_MAX || reward->ItemLevel > replacement;
}

inline uint32 ReplacementWeight(Player const* player, ItemTemplate const* reward, float floor)
{
    double lowest = UINT32_MAX;
    double replacement = UINT32_MAX;
    bool heirloom = false;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;
        Item const* equipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        ItemTemplate const* item = equipped ? equipped->GetTemplate() : nullptr;
        if (!item) continue;
        double const level = item->Quality == ITEM_QUALITY_HEIRLOOM ? floor : item->ItemLevel;
        lowest = std::min(lowest, level);
        bool const compatible = MatchesReplacement(player, reward, slot, item);
        if (compatible && level < replacement)
        {
            replacement = level;
            heirloom = item->Quality == ITEM_QUALITY_HEIRLOOM;
        }
    }
    return replacement != UINT32_MAX && replacement == lowest
        && (reward->ItemLevel > replacement || (heirloom && reward->ItemLevel >= replacement)) ? 3 : 1;
}

}

#endif
