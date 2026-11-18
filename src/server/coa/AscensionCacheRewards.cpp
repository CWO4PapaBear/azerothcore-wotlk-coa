/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionCacheRewards.h"
#include "Area52CacheRuntime.h"
#include "Area52CacheItemLevel.h"
#include "Area52ActiveBuild.h"
#include "AscensionWildcard.h"
#include "AsyncCallbackProcessor.h"
#include "ObjectAccessor.h"
#include "ScriptedGossip.h"
#include "PlayerScript.h"
#include "World.h"
#include "CallboardRealmTier.h"
#include "Random.h"
#include "QuestDef.h"
#include <atomic>
#include <deque>
#include <mutex>
#include "Area52CacheBagPlan.h"
#include "Area52CacheOwnership.h"
#include "Area52CacheSelection.h"
#include "AscensionPersonalBank.h"
#include "Area52CacheAttemptPersistence.h"
#include "Area52CacheAttemptSQL.h"
#include "DatabaseEnv.h"
#include "Config.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "DBCStores.h"
#include "DisableMgr.h"
#include "Bag.h"
#include "AuctionHouseMgr.h"
#include "Mail.h"
#include "Chat.h"
#include "Item.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <algorithm>
#include <memory>
#include <map>
#include <string>
#include <utility>

namespace Area52CacheAttempt
{
QueryCallback LoadStored(std::uint64_t cache)
{
    return CharacterDatabase.AsyncQuery("SELECT owner_guid,cache_guid,request_id,revision,phase,HEX(payload),1 "
        "FROM mod_area52_cache_attempt WHERE cache_guid=" + std::to_string(cache)
        + " UNION ALL SELECT 0,0,0,0,0,'',0");
}

StoredRead DecodeStored(QueryResult const& rows, std::uint64_t owner, std::uint64_t cache)
{
    if (!rows || !owner || !cache)
        return {};
    StoredRead result{ReadStatus::Missing, std::nullopt};
    do
    {
        Field* fields = rows->Fetch();
        if (!fields[6].Get<uint32>())
            continue;
        if (result.state)
            return {};
        std::string const hex = fields[5].Get<std::string>();
        if (hex.empty() || hex.size() > 82000 || hex.size() % 2)
            return {};
        auto nibble = [](char value) -> int
        {
            if (value >= '0' && value <= '9') return value - '0';
            if (value >= 'A' && value <= 'F') return value - 'A' + 10;
            return -1;
        };
        std::vector<std::uint8_t> bytes;
        bytes.reserve(hex.size() / 2);
        for (std::size_t index = 0; index < hex.size(); index += 2)
        {
            int const high = nibble(hex[index]);
            int const low = nibble(hex[index + 1]);
            if (high < 0 || low < 0)
                return {};
            bytes.push_back(std::uint8_t(high * 16 + low));
        }
        auto state = Decode(bytes);
        if (!state || state->owner != owner || state->cache != cache
            || state->owner != fields[0].Get<uint64>() || state->cache != fields[1].Get<uint64>()
            || state->request != fields[2].Get<uint64>() || state->revision != fields[3].Get<uint64>()
            || unsigned(state->phase) != fields[4].Get<uint32>())
            return {};
        result = {ReadStatus::Found, std::move(state)};
    } while (rows->NextRow());
    return result;
}

bool AppendStoredMutation(CharacterDatabaseTransaction transaction, std::optional<State> const& previous,
    State const& next)
{
    auto statements = Mutation(previous, next);
    if (!transaction || !statements)
        return false;
    for (std::string const& statement : *statements)
        transaction->Append(statement);
    return true;
}
}

namespace AscensionCacheRewards
{
#include "Area52CacheSetMetadata.inc"
#include "Area52CacheTierMetadata.inc"
#include "Area52CacheSourceMetadata.inc"

std::set<uint32> OpenSources()
{
    std::vector<bool> values(CacheSourceNodes.size(), false);
    for (std::size_t index = 0; index < CacheSourceNodes.size(); ++index)
    {
        auto const& node = CacheSourceNodes[index];
        if (node.kind == 1)
            values[index] = true;
        else if (node.kind == 2 && node.values.size() == 1)
        {
            uint32 const map = node.values.front() / 4;
            uint8 const difficulty = uint8(node.values.front() % 4);
            auto const* entry = sMapStore.LookupEntry(map);
            values[index] = entry && (!entry->IsDungeon() || GetMapDifficultyData(map, Difficulty(difficulty)))
                && !DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, map, nullptr, difficulty);
        }
        else if (node.kind == 3 && node.values.size() == 1)
            values[index] = sObjectMgr->GetQuestTemplate(node.values.front())
                && !DisableMgr::IsDisabledFor(DISABLE_TYPE_QUEST, node.values.front(), nullptr);
        else if (node.kind == 4 || node.kind == 5)
        {
            bool value = node.kind == 4;
            for (uint32 child : node.values)
            {
                if (child >= index) return {};
                if (node.kind == 4) value = value && values[child];
                else value = value || values[child];
            }
            values[index] = value;
        }
    }
    std::set<uint32> result;
    for (auto const& [item, node] : CacheItemSources)
        if (node < values.size() && values[node]) result.insert(item);
    return result;
}

bool SourceOpen(uint32 itemId)
{
    return OpenSources().contains(itemId);
}

bool TierOpen(uint32 setId, uint32 realmCap)
{
    auto tier = CacheTierSets.find(setId);
    if (tier == CacheTierSets.end())
        return true;
    if (tier->second.cap != realmCap)
        return false;
    for (uint32 map : tier->second.mainRaids)
    {
        bool open = false;
        for (uint32 difficulty = 0; difficulty < 4; ++difficulty)
            if (GetMapDifficultyData(map, Difficulty(difficulty))
                && !DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, map, nullptr, uint8(difficulty)))
                open = true;
        if (!open)
            return false;
    }
    return true;
}

SetRelevance EvaluateSet(Player* player, uint32 setId)
{
    if (!player)
        return {};
    if (!setId)
        return {true, true, 0};
    auto const* set = sItemSetStore.LookupEntry(setId);
    if (!set)
        return {};
    std::map<uint32, SpellInfo const*> learned;
    for (auto const& [id, state] : player->GetSpellMap())
    {
        if (!player->HasActiveSpell(id))
            continue;
        if (auto const* spell = sSpellMgr->GetSpellInfo(id))
            learned.emplace(id, spell);
        for (uint32 origin : player->GetSpellInteractionOrigins(id))
            if (auto const* spell = sSpellMgr->GetSpellInfo(origin))
                learned.emplace(origin, spell);
    }
    bool useful = false;
    std::set<uint32> matches;
    for (uint32 bonus : set->spells)
    {
        if (!bonus)
            continue;
        auto metadata = CacheBonuses.find(bonus);
        auto const* info = sSpellMgr->GetSpellInfo(bonus);
        if (metadata == CacheBonuses.end() || !info)
            return {};
        unsigned const mode = metadata->second.mode;
        if (mode == 3) // Cosmetic disguise is not a combat reward benefit.
            continue;
        if (mode == 0)
        {
            useful = true;
            continue;
        }
        for (auto const& [id, spell] : learned)
        {
            bool match = false;
            if (mode == 1)
            {
                for (auto const& effect : info->Effects)
                    if (effect.SpellClassMask && spell->IsAffected(info->SpellFamilyName, effect.SpellClassMask))
                        match = true;
                if (auto const* proc = sSpellMgr->GetSpellProcEntry(bonus))
                    if (proc->SpellFamilyMask && spell->IsAffected(proc->SpellFamilyName, proc->SpellFamilyMask))
                        match = true;
            }
            else if (mode == 2)
            {
                for (uint32 required : metadata->second.spells)
                    if (sSpellMgr->GetFirstSpellInChain(id) == sSpellMgr->GetFirstSpellInChain(required))
                    {
                        match = true;
                        break;
                    }
            }
            else if (mode == 4)
                match = spell->NeedsComboPoints();
            else if (mode == 5)
                match = spell->HasAura(SPELL_AURA_MOD_SHAPESHIFT);
            else if (mode == 6)
                match = spell->HasEffect(SPELL_EFFECT_SUMMON_PET) || spell->HasEffect(SPELL_EFFECT_TAMECREATURE);
            else if (mode == 7)
                match = spell->SpellFamilyName == SPELLFAMILY_HUNTER && !spell->IsPassive();
            if (match)
            {
                useful = true;
                matches.insert(sSpellMgr->GetFirstSpellInChain(id));
            }
        }
    }
    return {true, useful, uint32(matches.size())};
}

namespace
{
bool IsVisiblePosition(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0)
        return slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END;
    return bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END;
}

bool FitsAt(Player* player, uint8 bag, uint8 slot, uint32 itemId, uint32 count, bool onExistingStack,
    ItemPosCountVec& dest)
{
    Item const* held = player->GetItemByPos(bag, slot);
    if (onExistingStack != (held && held->GetEntry() == itemId))
        return false;

    dest.clear();
    return player->CanStoreItem(bag, slot, dest, itemId, count) == EQUIP_ERR_OK;
}

bool FindVisibleDestination(Player* player, uint32 itemId, uint32 count, ItemPosCountVec& dest)
{
    for (bool const onExistingStack : { true, false })
    {
        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
            if (FitsAt(player, INVENTORY_SLOT_BAG_0, slot, itemId, count, onExistingStack, dest))
                return true;

        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            Bag* container = player->GetBagByPos(bag);
            if (!container)
                continue;

            for (uint8 slot = 0; slot < container->GetBagSize(); ++slot)
                if (FitsAt(player, bag, slot, itemId, count, onExistingStack, dest))
                    return true;
        }
    }

    dest.clear();
    return false;
}

InventoryResult StoreOne(Player* player, Reward const& reward, Item*& stored)
{
    if (!sObjectMgr->GetItemTemplate(reward.itemId))
        return EQUIP_ERR_ITEM_NOT_FOUND;

    ItemPosCountVec dest;
    if (!FindVisibleDestination(player, reward.itemId, reward.count, dest))
        return EQUIP_ERR_INVENTORY_FULL;

    stored = player->StoreNewItem(dest, reward.itemId, true, reward.randomPropertyId);
    if (!stored || !IsVisiblePosition(stored->GetBagSlot(), stored->GetSlot()))
        return EQUIP_ERR_INVENTORY_FULL;

    return EQUIP_ERR_OK;
}

bool PrepareMail(Player* player, std::vector<Reward> const& rewards, std::vector<std::unique_ptr<Item>>& items)
{
    for (Reward const& reward : rewards)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(reward.itemId);
        if (!proto)
            return false;

        uint32 remaining = reward.count;
        uint32 const perStack = std::max<uint32>(1, proto->GetMaxStackSize());
        while (remaining)
        {
            uint32 const chunk = std::min<uint32>(remaining, perStack);
            std::unique_ptr<Item> item(Item::CreateItem(reward.itemId, chunk, player, false, reward.randomPropertyId));
            if (!item)
                return false;
            items.push_back(std::move(item));
            remaining -= chunk;
        }
    }
    return true;
}

void UndoOne(Player* player, Item* item, uint32 count)
{
    if (item->GetCount() > count)
    {
        item->SetCount(item->GetCount() - count);
        item->SetState(ITEM_CHANGED, player);
        return;
    }

    player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
}
}

// Called only by the world-thread cache service. Reject pending native/database
// disagreement rather than treating an uncertain attachment or auction as unowned.
bool ReadExternalOwnedItems(Player* player, std::unordered_set<uint32>& items)
{
    items.clear();
    if (!player)
        return false;
    std::map<uint64, uint32> native;
    auto add = [&](uint64 guid, uint32 entry)
    {
        if (!guid || !entry || !sObjectMgr->GetItemTemplate(entry))
            return false;
        auto [position, inserted] = native.emplace(guid, entry);
        return inserted || position->second == entry;
    };
    for (Mail* mail : player->GetMails())
    {
        if (!mail || mail->receiver != player->GetGUID().GetCounter())
            return false;
        if (mail->state == MAIL_STATE_DELETED)
            continue;
        for (auto const& attachment : mail->items)
            if (!add(attachment.item_guid, attachment.item_template))
                return false;
    }
    for (auto house : {AuctionHouseId::Alliance, AuctionHouseId::Horde, AuctionHouseId::Neutral})
    {
        AuctionHouseObject* auctions = sAuctionMgr->GetAuctionsMapByHouseId(house);
        if (!auctions)
            return false;
        for (auto const& [id, auction] : auctions->GetAuctions())
        {
            if (auction->owner != player->GetGUID())
                continue;
            Item* item = sAuctionMgr->GetAItem(auction->item_guid);
            if (!item || item->GetEntry() != auction->item_template || item->GetCount() != auction->itemCount
                || !add(auction->item_guid.GetCounter(), auction->item_template))
                return false;
        }
    }
    uint64 const owner = player->GetGUID().GetCounter();
    QueryResult rows = CharacterDatabase.Query(
        "SELECT mi.item_guid,IF(m.receiver=mi.receiver,ii.itemEntry,0),1 FROM mail_items mi LEFT JOIN mail m ON m.id=mi.mail_id "
        "LEFT JOIN item_instance ii ON ii.guid=mi.item_guid WHERE mi.receiver={} "
        "UNION ALL SELECT a.itemguid,ii.itemEntry,1 FROM auctionhouse a "
        "LEFT JOIN item_instance ii ON ii.guid=a.itemguid WHERE a.itemowner={} "
        "UNION ALL SELECT 0,0,0", owner, owner);
    if (!rows)
        return false;
    std::map<uint64, uint32> persisted;
    do
    {
        Field* fields = rows->Fetch();
        if (!fields[2].Get<uint32>())
            continue;
        uint64 const guid = fields[0].Get<uint64>();
        uint32 const entry = fields[1].Get<uint32>();
        if (!guid || !entry || !sObjectMgr->GetItemTemplate(entry) || !persisted.emplace(guid, entry).second)
            return false;
    } while (rows->NextRow());
    if (persisted != native)
        return false;
    for (auto const& [guid, entry] : persisted)
        items.insert(entry);
    return true;
}

bool PopulateInventoryAndBankOwnership(Player* player, std::vector<Area52CacheSelection::Candidate>& candidates)
{
    for (auto& candidate : candidates)
    {
        candidate.owned = false;
        candidate.appearanceOwned = false;
    }
    if (!player || candidates.empty())
        return false;
    std::unordered_set<uint32> bankItems;
    if (!AscensionPersonalBank::ReadOwnedItems(player, bankItems))
        return false;
    std::unordered_set<uint32> externalItems;
    if (!ReadExternalOwnedItems(player, externalItems))
        return false;
    auto populated = candidates;
    for (auto& candidate : populated)
    {
        if (!candidate.item || !sObjectMgr->GetItemTemplate(candidate.item))
            return false;
        auto const appearance = Area52CacheOwnership::ReadAppearance(player, candidate.item);
        if (!appearance.Ready)
            return false;
        candidate.owned = bankItems.contains(candidate.item) || externalItems.contains(candidate.item)
            || player->GetItemCount(candidate.item, true) != 0;
        candidate.appearanceOwned = appearance.Owned;
    }
    candidates.swap(populated);
    return true;
}

InventoryResult CheckVisibleBagSpace(Player* player, std::vector<Reward> const& rewards)
{
    if (rewards.empty())
        return EQUIP_ERR_ITEM_NOT_FOUND;
    std::vector<std::unique_ptr<Item>> items;
    for (Reward const& reward : rewards)
        if (!reward.count || !sObjectMgr->GetItemTemplate(reward.itemId))
            return EQUIP_ERR_ITEM_NOT_FOUND;
    if (!PrepareMail(player, rewards, items))
        return EQUIP_ERR_ITEM_NOT_FOUND;
    std::vector<Item*> pointers;
    for (auto const& item : items)
        pointers.push_back(item.get());
    uint32 limitedBy = 0;
    InventoryResult const native = player->CanStoreItems(pointers.data(), int(pointers.size()), &limitedBy);
    if (native != EQUIP_ERR_OK)
        return native;
    std::vector<Area52CacheBagPlan::Stack> incoming;
    for (auto const& item : items)
        incoming.push_back({ item->GetEntry(), item->GetItemRandomPropertyId(), item->GetCount(),
            uint32(std::max<int32>(1, item->GetTemplate()->GetMaxStackSize())) });
    std::vector<Area52CacheBagPlan::Slot> slots;
    auto append = [&](uint8 bag, uint8 slot)
    {
        Item* held = player->GetItemByPos(bag, slot);
        Area52CacheBagPlan::Slot value;
        value.position = uint16(bag) << 8 | slot;
        value.available = !held || !held->IsInTrade();
        if (held)
            value.held = { held->GetEntry(), held->GetItemRandomPropertyId(), held->GetCount(),
                uint32(std::max<int32>(1, held->GetTemplate()->GetMaxStackSize())) };
        slots.push_back(value);
    };
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        append(INVENTORY_SLOT_BAG_0, slot);
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        if (Bag* container = player->GetBagByPos(bag))
            for (uint8 slot = 0; slot < container->GetBagSize(); ++slot)
                append(bag, slot);
    auto const result = Area52CacheBagPlan::Plan(incoming, slots,
        [&](uint16 position, Area52CacheBagPlan::Stack const& reward)
        {
            ItemPosCountVec dest;
            return player->CanStoreItem(uint8(position >> 8), uint8(position), dest, reward.item, 1) == EQUIP_ERR_OK
                && dest.size() == 1 && dest.front().pos == position && dest.front().count == 1;
        });
    return result.complete ? EQUIP_ERR_OK : EQUIP_ERR_INVENTORY_FULL;
}

void NotifyInsufficientBagSpace(Player* player)
{
    WorldPacket packet;
    ChatHandler::BuildChatPacket(packet, CHAT_MSG_RAID_WARNING, LANG_UNIVERSAL, player->GetGUID(),
        player->GetGUID(), "Not enough bag space for all cache rewards. Free up bag space and try again.",
        0, player->GetName(), player->GetName());
    player->GetSession()->SendPacket(&packet);
}

bool DeliverReserved(Player* player, Area52CacheAttempt::State& attempt, Item* cache)
{
    using namespace Area52CacheAttempt;
    if (!player || !cache || attempt.phase != Phase::Ready || !Valid(attempt)
        || attempt.owner != player->GetGUID().GetCounter() || attempt.cache != cache->GetGUID().GetCounter()
        || cache->GetOwnerGUID() != player->GetGUID() || cache->IsInTrade() || cache->GetCount() != 1
        || cache->GetTemplate()->GetMaxStackSize() != 1
        || sConfigMgr->GetOption<uint32>("CharacterDatabase.WorkerThreads", 1) != 1
        || attempt.copper > MAX_MONEY_AMOUNT - player->GetMoney())
        return false;
    std::vector<Reward> rewards{{attempt.offered, 0, 0, 0, attempt.offeredCount, attempt.offeredRandomProperty}};
    for (auto const& token : attempt.tokens)
        rewards.push_back({token.item, 0, 0, 0, token.count, 0});
    std::vector<std::unique_ptr<Item>> prepared;
    std::vector<Area52CacheBagPlan::Stack> incoming;
    for (auto const& reward : rewards)
    {
        auto const* proto = sObjectMgr->GetItemTemplate(reward.itemId);
        if (!proto || !reward.count)
            return false;
        uint32 const maximum = uint32(std::max<int32>(1, proto->GetMaxStackSize()));
        incoming.push_back({reward.itemId, reward.randomPropertyId, reward.count, maximum});
        for (uint32 remaining = reward.count; remaining;)
        {
            uint32 const count = std::min(remaining, maximum);
            prepared.emplace_back(Item::CreateItem(reward.itemId, count, player, true, reward.randomPropertyId));
            if (!prepared.back())
                return false;
            remaining -= count;
        }
    }
    std::vector<Item*> pointers;
    for (auto const& item : prepared)
        pointers.push_back(item.get());
    uint32 limitedBy = 0;
    InventoryResult native = player->CanStoreItems(pointers.data(), int(pointers.size()), &limitedBy);
    if (native != EQUIP_ERR_OK)
    {
        if (native == EQUIP_ERR_INVENTORY_FULL)
            NotifyInsufficientBagSpace(player);
        else
            player->SendEquipError(native, cache, nullptr);
        return false;
    }
    std::vector<Area52CacheBagPlan::Slot> slots;
    auto append = [&](uint8 bag, uint8 slot)
    {
        Item* item = player->GetItemByPos(bag, slot);
        Area52CacheBagPlan::Slot value;
        value.position = uint16(bag) << 8 | slot;
        value.available = !item || !item->IsInTrade();
        if (item)
            value.held = {item->GetEntry(), item->GetItemRandomPropertyId(), item->GetCount(),
                uint32(std::max<int32>(1, item->GetTemplate()->GetMaxStackSize()))};
        slots.push_back(value);
    };
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        append(INVENTORY_SLOT_BAG_0, slot);
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        if (Bag* container = player->GetBagByPos(bag))
            for (uint8 slot = 0; slot < container->GetBagSize(); ++slot)
                append(bag, slot);
    auto const plan = Area52CacheBagPlan::Plan(incoming, slots,
        [&](uint16 position, Area52CacheBagPlan::Stack const& reward)
        {
            ItemPosCountVec destination;
            return player->CanStoreItem(uint8(position >> 8), uint8(position), destination, reward.item, 1) == EQUIP_ERR_OK
                && destination.size() == 1 && destination.front().pos == position && destination.front().count == 1;
        });
    if (!plan.complete)
    {
        NotifyInsufficientBagSpace(player);
        return false;
    }
    struct Addition {std::size_t reward = 0; uint32 count = 0; std::unique_ptr<Item> item;};
    std::map<uint16, Addition> additions;
    for (auto const& placement : plan.placements)
    {
        auto& addition = additions[placement.position];
        if (addition.count && (rewards[addition.reward].itemId != rewards[placement.reward].itemId
            || rewards[addition.reward].randomPropertyId != rewards[placement.reward].randomPropertyId))
            return false;
        addition.reward = placement.reward;
        addition.count += placement.count;
    }
    State committing = attempt;
    if (BeginDelivery(committing, attempt.owner, attempt.cache, attempt.request, attempt.revision, true, true)
        != Result::CommitRequired)
        return false;
    State delivered = committing;
    if (ConfirmCommit(delivered, committing.request, committing.revision) != Result::Changed)
        return false;
    auto transaction = CharacterDatabase.BeginTransaction();
    if (!AppendStoredMutation(transaction, attempt, committing))
        return false;
    auto guard = [&]()
    {
        transaction->Append("INSERT INTO mod_area52_cache_attempt_guard(request_id,matched_rows) VALUES({},ROW_COUNT())",
            attempt.request);
        transaction->Append("DELETE FROM mod_area52_cache_attempt_guard WHERE request_id={}", attempt.request);
    };
    uint64 const owner = player->GetGUID().GetCounter();
    auto bagGuid = [&](uint8 bag) -> uint64
    {
        if (bag == INVENTORY_SLOT_BAG_0)
            return 0;
        Bag* container = player->GetBagByPos(bag);
        return container ? container->GetGUID().GetCounter() : 0;
    };
    transaction->Append("DELETE ci FROM character_inventory ci INNER JOIN item_instance ii ON ii.guid=ci.item "
        "WHERE ci.guid={} AND ci.item={} AND ci.bag={} AND ci.slot={} AND ii.owner_guid={} AND ii.count=1 AND ii.itemEntry={}",
        owner, attempt.cache, bagGuid(cache->GetBagSlot()), cache->GetSlot(), owner, cache->GetEntry());
    guard();
    Item::DeleteFromDB(transaction, cache->GetGUID().GetCounter());
    for (auto& [position, addition] : additions)
    {
        auto const& reward = rewards[addition.reward];
        addition.item.reset(Item::CreateItem(reward.itemId, addition.count, player, true, reward.randomPropertyId));
        if (!addition.item)
            return false;
        if (addition.item->GetTemplate()->Bonding == BIND_WHEN_PICKED_UP
            || addition.item->GetTemplate()->Bonding == BIND_QUEST_ITEM)
            addition.item->SetBinding(true);
        uint8 const bag = uint8(position >> 8);
        uint64 const container = bagGuid(bag);
        if (bag != INVENTORY_SLOT_BAG_0 && !container)
            return false;
        if (container)
        {
            transaction->Append("INSERT INTO mod_area52_cache_attempt_guard(request_id,matched_rows) "
                "SELECT {},COUNT(*) FROM character_inventory ci INNER JOIN item_instance ii ON ii.guid=ci.item "
                "WHERE ci.guid={} AND ci.item={} AND ci.bag=0 AND ci.slot={} AND ii.owner_guid={}",
                attempt.request, owner, container, bag, owner);
            transaction->Append("DELETE FROM mod_area52_cache_attempt_guard WHERE request_id={}", attempt.request);
        }
        if (Item* held = player->GetItemByPos(position))
        {
            transaction->Append("UPDATE item_instance ii INNER JOIN character_inventory ci ON ci.item=ii.guid "
                "SET ii.count=ii.count+{} WHERE ii.guid={} AND ii.owner_guid={} AND ii.itemEntry={} "
                "AND ii.count={} AND ii.randomPropertyId={} AND ci.guid={} AND ci.bag={} AND ci.slot={}",
                addition.count, held->GetGUID().GetCounter(), owner, reward.itemId, held->GetCount(),
                reward.randomPropertyId, owner, container, uint8(position));
            guard();
        }
        else
        {
            addition.item->SaveToDB(transaction);
            transaction->Append("INSERT INTO character_inventory(guid,bag,slot,item) "
                "SELECT {},{},{},{} WHERE NOT EXISTS (SELECT 1 FROM character_inventory WHERE guid={} AND bag={} AND slot={})",
                owner, container, uint8(position), addition.item->GetGUID().GetCounter(), owner, container, uint8(position));
            guard();
        }
    }
    if (attempt.copper)
    {
        transaction->Append("UPDATE characters SET money=money+{} WHERE guid={} AND money={}",
            attempt.copper, owner, player->GetMoney());
        guard();
    }
    if (!AppendStoredMutation(transaction, committing, delivered))
        return false;
    if (!CharacterDatabase.AsyncCommitTransaction(transaction).m_future.get())
    {
        ChatHandler(player->GetSession()).SendSysMessage("The cache could not be saved. Your cache and selected rewards were kept; try again shortly.");
        return false;
    }
    attempt = delivered;
    uint32 consumed = 1;
    player->DestroyItemCount(cache, consumed, true);
    std::vector<std::pair<Item*, uint32>> stored;
    for (auto& [position, addition] : additions)
    {
        ItemPosCountVec destination{{position, addition.count}};
        Item* item = player->StoreItem(destination, addition.item.release(), true);
        if (!item)
        {
            LOG_ERROR("coa", "Committed cache {} needs inventory reload for {}", attempt.request, owner);
            ChatHandler(player->GetSession()).SendSysMessage("Your cache rewards are saved. Relog to reload your inventory.");
            return true;
        }
        stored.emplace_back(item, addition.count);
    }
    if (attempt.copper)
        player->ModifyMoney(int32(attempt.copper));
    for (auto const& [item, count] : stored)
    {
        player->ItemAddedQuestCheck(item->GetEntry(), count);
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM, item->GetEntry(), count);
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM, item->GetEntry(), count);
        sScriptMgr->OnPlayerStoreNewItem(player, item, count);
        player->SendNewItem(item, count, true, false, false);
    }
    return true;
}

bool Deliver(Player* player, std::vector<Reward> const& rewards, Item* cache)
{
    if (rewards.empty())
        return false;

    for (Reward const& reward : rewards)
        if (!reward.count || !sObjectMgr->GetItemTemplate(reward.itemId))
        {
            LOG_ERROR("coa",
                "cache {} has reward {} x{}, which is not a valid item; cache kept",
                cache ? cache->GetEntry() : 0, reward.itemId, reward.count);
            return false;
        }

    std::vector<std::pair<Item*, uint32>> stored;
    stored.reserve(rewards.size());
    std::vector<Reward> posted;

    for (Reward const& reward : rewards)
    {
        uint32 held = player->GetItemCount(reward.itemId);
        Item* item = nullptr;
        InventoryResult result = StoreOne(player, reward, item);
        if (result == EQUIP_ERR_OK && player->GetItemCount(reward.itemId) < held + reward.count)
            result = EQUIP_ERR_INVENTORY_FULL;

        if (result == EQUIP_ERR_OK)
        {
            stored.emplace_back(item, reward.count);
            continue;
        }

        if (item)
            UndoOne(player, item, reward.count);
        posted.push_back(reward);
    }

    std::vector<std::unique_ptr<Item>> mailItems;
    if (!PrepareMail(player, posted, mailItems))
    {
        for (auto given = stored.rbegin(); given != stored.rend(); ++given)
            UndoOne(player, given->first, given->second);
        LOG_ERROR("coa", "cache {} rewards could not be prepared for {}; cache kept",
            cache ? cache->GetEntry() : 0, player->GetGUID().ToString());
        return false;
    }

    for (std::unique_ptr<Item>& item : mailItems)
        player->SendItemRetrievalMail(item.release());

    for (auto const& given : stored)
        player->SendNewItem(given.first, given.second, true, false, false);

    if (!posted.empty())
    {
        std::string message = "Your bags were full, so your reward was sent to your mailbox.";
        if (posted.size() > 1)
            message = "Your bags were full, so all " + std::to_string(posted.size()) +
                " of your rewards were sent to your mailbox.";
        ChatHandler handler(player->GetSession());
        handler.SendSysMessage(message);
        handler.SendNotification(message);

        LOG_DEBUG("coa",
            "cache {} posted {} reward(s) to {} because the bags were full",
            cache ? cache->GetEntry() : 0, uint32(posted.size()), player->GetGUID().ToString());
    }

    uint32 count = 1;
    player->DestroyItemCount(cache, count, true);
    return true;
}
}

#include "Area52CacheCatalog.inc"
#include "Area52CacheRuntime.inc"
