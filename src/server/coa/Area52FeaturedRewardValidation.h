#ifndef AREA52_FEATURED_REWARD_VALIDATION_H
#define AREA52_FEATURED_REWARD_VALIDATION_H

#include "Area52FeaturedRewards.h"
#include "Area52FeaturedFlyingMounts.h"
#include "AscensionCollectibleSpellData.h"
#include <algorithm>
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include <set>

namespace Area52Build
{
inline void FeaturedMountSpells(std::uint32_t id, std::set<std::uint32_t>& seen, bool& mount, bool& flying)
{
    if (!id || seen.size() >= 64 || !seen.insert(id).second)
        return;
    auto const* spell = sSpellMgr->GetSpellInfo(id);
    if (!spell)
        return;
    auto const& wrappers = AscensionCollectibles::MountWrappers;
    auto wrapper = std::lower_bound(wrappers.begin(), wrappers.end(), id,
        [](auto const& entry, std::uint32_t value) { return entry.SpellId < value; });
    if (wrapper != wrappers.end() && wrapper->SpellId == id)
        for (auto child : {wrapper->Ground60, wrapper->Ground100, wrapper->Flying150,
            wrapper->Flying280, wrapper->Flying310})
            FeaturedMountSpells(child, seen, mount, flying);
    for (auto const& effect : spell->Effects)
    {
        mount = mount || effect.ApplyAuraName == SPELL_AURA_MOUNTED;
        flying = flying || effect.ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED;
        if (effect.Effect == SPELL_EFFECT_LEARN_SPELL || effect.Effect == SPELL_EFFECT_TRIGGER_SPELL)
            FeaturedMountSpells(effect.TriggerSpell, seen, mount, flying);
    }
}

inline bool ValidateFeaturedRewards(std::string_view description, FeaturedRewardItems& items)
{
    if (!ReadFeaturedRewards(description, items))
        return false;
    for (std::size_t slot = 0; slot < items.size(); ++slot)
    {
        if (!items[slot])
            continue;
        auto const* item = sObjectMgr->GetItemTemplate(items[slot]);
        if (!item || !FeaturedSlotAccepts(slot, item->InventoryType))
            return false;
        if (slot >= 12)
        {
            bool mount = false, flying = false;
            std::set<std::uint32_t> seen;
            for (auto const& spell : item->Spells)
                FeaturedMountSpells(spell.SpellId, seen, mount, flying);
            flying = flying || std::binary_search(FeaturedVanityFlyingItems.begin(),
                FeaturedVanityFlyingItems.end(), items[slot]);
            bool ground = !flying || std::binary_search(FeaturedVanityGroundItems.begin(),
                FeaturedVanityGroundItems.end(), items[slot]);
            if (!mount || (slot == 13 ? !flying : !ground))
                return false;
        }
    }
    auto const* main = items[9] ? sObjectMgr->GetItemTemplate(items[9]) : nullptr;
    return !main || main->InventoryType != INVTYPE_2HWEAPON || !items[10];
}
}
#endif
