#ifndef AREA52_VANITY_PURCHASE_H
#define AREA52_VANITY_PURCHASE_H

#include "Area52MasterOffers.h"
#include "Bag.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "Player.h"
#include "WorldSession.h"
#include <memory>
#include <vector>

namespace Area52Vanity
{
constexpr uint32 Token = 975001;
constexpr uint32 Request = 0xA52B0001;
constexpr uint32 MaximumPrice = 5000000;

struct Debit
{
    uint32 Guid, Count, Spent;
    Item* Stack;
};

inline bool Commit(CharacterDatabaseTransaction transaction)
{
    return CharacterDatabase.AsyncCommitTransaction(transaction).m_future.get();
}

inline bool Purchase(Player* player, uint32 itemId, uint32 cost, bool consumable, uint32 currency = Token, std::vector<uint32> const& pieces = {}, std::map<uint32, uint32> const& bundleAppearances = {})
{
    ItemPosCountVec destinations;
    std::unique_ptr<Item> reward;
    Item* stack = nullptr;
    if ((!cost && !Area52Master::Bundles.contains(itemId)) || cost > MaximumPrice || (cost && !player->HasItemCount(currency, cost)))
        return false;
    bool const deliverToBags = consumable || Area52Master::IsHeirloomCache(itemId);
    if (deliverToBags)
    {
        if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, destinations, itemId, 1) != EQUIP_ERR_OK
            || destinations.size() != 1 || destinations[0].count != 1)
            return false;
        reward.reset(Item::CreateItem(itemId, 1, player));
        if (!reward)
            return false;
        reward->SetBinding(true);
        stack = player->GetItemByPos(destinations[0].pos);
    }

    std::vector<Debit> debits;
    uint32 remaining = cost;
    auto add = [&](Item* item)
    {
        if (!remaining || !item || item->GetEntry() != currency || item->IsInTrade())
            return;
        uint32 spent = std::min(remaining, item->GetCount());
        debits.push_back({item->GetGUID().GetCounter(), item->GetCount(), spent, item});
        remaining -= spent;
    };
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        add(player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot));
    for (uint8 slot = KEYRING_SLOT_START; slot < CURRENCYTOKEN_SLOT_END; ++slot)
        add(player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot));
    for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
        if (Bag* bag = player->GetBagByPos(slot))
            for (uint32 inner = 0; inner < bag->GetBagSize(); ++inner)
                add(bag->GetItemByPos(inner));
    if (remaining)
        return false;

    auto checkpoint = CharacterDatabase.BeginTransaction();
    player->SaveInventoryAndGoldToDB(checkpoint);
    if (!Commit(checkpoint))
        return false;
    uint32 owner = player->GetGUID().GetCounter();
    auto transaction = CharacterDatabase.BeginTransaction();
    transaction->Append("DROP TEMPORARY TABLE IF EXISTS area52_vanity_guard");
    transaction->Append("CREATE TEMPORARY TABLE area52_vanity_guard (ok TINYINT NOT NULL CHECK (ok = 1)) ENGINE=InnoDB");
    for (auto const& [piece, appearance] : bundleAppearances)
    {
        bool missing = std::find(pieces.begin(), pieces.end(), piece) != pieces.end();
        transaction->Append("INSERT INTO area52_vanity_guard SELECT IF((EXISTS(SELECT 1 FROM account_vanity_collection "
            "WHERE account_id={} AND item_id={} FOR UPDATE) OR ({} AND EXISTS(SELECT 1 FROM account_appearance_collection "
            "WHERE account_id={} AND appearance_id={} FOR UPDATE)))={},1,0)",
            player->GetSession()->GetAccountId(), piece, !Area52Master::IsHeirloomCache(itemId), player->GetSession()->GetAccountId(), appearance, missing ? 0 : 1);
    }
    for (Debit const& debit : debits)
    {
        transaction->Append("UPDATE item_instance SET count=count-{} WHERE guid={} AND owner_guid={} AND count={}",
            debit.Spent, debit.Guid, owner, debit.Count);
        transaction->Append("INSERT INTO area52_vanity_guard VALUES(IF(ROW_COUNT()=1,1,0))");
        if (debit.Spent == debit.Count)
        {
            transaction->Append("DELETE FROM character_inventory WHERE guid={} AND item={}", owner, debit.Guid);
            transaction->Append("DELETE FROM item_instance WHERE guid={} AND owner_guid={} AND count=0", debit.Guid, owner);
        }
    }
    for (uint32 piece : pieces)
        transaction->Append("INSERT INTO account_vanity_collection (account_id,item_id) VALUES ({},{})",
            player->GetSession()->GetAccountId(), piece);
    if (!consumable)
        transaction->Append("INSERT INTO account_vanity_collection (account_id,item_id) VALUES ({},{})",
            player->GetSession()->GetAccountId(), itemId);
    if (deliverToBags && stack)
    {
        transaction->Append("UPDATE item_instance SET count=count+1,flags=flags|1 WHERE guid={} AND owner_guid={} AND count={}",
            stack->GetGUID().GetCounter(), owner, stack->GetCount());
        transaction->Append("INSERT INTO area52_vanity_guard VALUES(IF(ROW_COUNT()=1,1,0))");
    }
    else if (deliverToBags)
    {
        uint8 bag = uint8(destinations[0].pos >> 8);
        uint32 bagGuid = bag == INVENTORY_SLOT_BAG_0 ? 0 : player->GetBagByPos(bag)->GetGUID().GetCounter();
        reward->SaveToDB(transaction);
        transaction->Append("INSERT INTO character_inventory (guid,bag,slot,item) VALUES ({},{},{},{})",
            owner, bagGuid, uint8(destinations[0].pos), reward->GetGUID().GetCounter());
    }
    transaction->Append("DROP TEMPORARY TABLE area52_vanity_guard");
    if (!Commit(transaction))
        return false;
    for (Debit const& debit : debits)
    {
        uint32 spent = debit.Spent;
        player->DestroyItemCount(debit.Stack, spent, true);
    }
    if (deliverToBags)
    {
        Item* bought = player->StoreItem(destinations, reward.release(), true);
        bought->SetBinding(true);
        player->SendNewItem(bought, 1, true, false);
    }
    return true;
}
}
#endif
