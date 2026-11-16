public:
    bool VanityPriceCommand(ChatHandler* handler, uint32 itemId, Optional<uint32> price)
    {
        if (!handler->GetPlayer() || !AscensionFreePick::Applies(handler->GetPlayer()))
            return false;
        auto found = _vanityItems.find(itemId);
        if (Area52Master::Offers.contains(itemId) || itemId == Area52Vanity::Token || found == _vanityItems.end() || !sObjectMgr->GetItemTemplate(itemId))
        {
            handler->SendSysMessage("Unknown Vanity item or missing item template.");
            return false;
        }
        if (price)
        {
            if (*price > Area52Vanity::MaximumPrice)
                return false;
            auto transaction = WorldDatabase.BeginTransaction();
            transaction->Append("INSERT INTO coa_client_vanity_collection (ItemID,DonationPointCost,SeasonalPointCost,BazaarTokenCost) "
                "VALUES ({},0,0,{}) ON DUPLICATE KEY UPDATE DonationPointCost=0,SeasonalPointCost=0,BazaarTokenCost=VALUES(BazaarTokenCost)", itemId, *price);
            if (!WorldDatabase.AsyncCommitTransaction(transaction).m_future.get())
            {
                handler->SendSysMessage("Price was not saved. No change was published.");
                return false;
            }
            VanityCatalogSignal.Request();
            PumpVanityCatalog();
            handler->PSendSysMessage("Saved item {} at {} Bazaar Tokens (0 disables purchases). Display update queued for the launcher. Stale displayed prices are rejected without charging.", itemId, *price);
            LOG_INFO("coa", "GM account {} set vanity item {} to {} Bazaar Tokens", handler->GetSession()->GetAccountId(), itemId, *price);
        }
        else
        {
            auto row = WorldDatabase.Query("SELECT BazaarTokenCost FROM coa_client_vanity_collection WHERE ItemID={}", itemId);
            handler->PSendSysMessage("Item {}: {} Bazaar Tokens. Missing or zero prices are not sold.", itemId,
                row ? row->Fetch()[0].Get<uint32>() : 0);
        }
        return true;
    }

    void PumpVanityCatalog()
    {
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        if (VanityCatalogSignal.Pump(now, Area52Build::WriteCatalogSignal) < 0)
            LOG_ERROR("coa", "Vanity price notification deferred; retained for retry");
    }

private:
    static bool IsConsumableVanityItem(VanityInfo const& info)
    {
        return (info.CategoryMask & 0x40000000u) != 0;
    }

    static bool IsVanityItemOffered(Player* player, VanityInfo const& info)
    {
        if ((info.Flags & 8u) || !info.StoreRecord[7])
            return false;
        uint64 classes = info.StoreRecord[13] | (uint64(info.StoreRecord[14]) << 32);
        uint32 races = info.StoreRecord[15];
        uint32 classIndex = player->getClass() - 1;
        uint32 raceIndex = player->getRace() - 1;
        return (!classes || (classIndex < 64 && (classes & (uint64(1) << classIndex))))
            && (!races || (raceIndex < 32 && (races & (uint32(1) << raceIndex))));
    }

    void SendVanityCollectionAdded(Player* player, uint32 itemId)
    {
        WorldPacket packet(SMSG_VANITY_COLLECTION_ADDED, sizeof(uint32));
        packet << itemId;
        player->GetSession()->SendPacket(&packet);
        if (!AscensionFreePick::Applies(player))
            return;
        WorldPacket learned;
        ChatHandler::BuildChatPacket(learned, CHAT_MSG_WHISPER, LANG_ADDON, player, player,
            Acore::StringFormat("ASCENSION_STORE_COLLECTION_ITEM_LEARNED\t{}", itemId));
        player->SendDirectMessage(&learned);
    }

    void PurchaseVanityItemWithBazaarTokens(Player* player, uint32 itemId, uint32 displayedCost)
    {
        auto state = GetState(player);
        if (!state || !AscensionFreePick::Applies(player))
            return;
        ChatHandler handler(player->GetSession());
        auto found = _vanityItems.find(itemId);
        if (Area52Master::Offers.contains(itemId) || itemId == Area52Vanity::Token || found == _vanityItems.end() || !sObjectMgr->GetItemTemplate(itemId)
            || std::binary_search(AscensionCollectibles::SigilVanityItems.begin(),
                AscensionCollectibles::SigilVanityItems.end(), itemId)
            || !IsVanityItemOffered(player, found->second))
        {
            handler.SendSysMessage("That item is not sold on this realm.");
            return;
        }
        auto row = WorldDatabase.Query("SELECT BazaarTokenCost FROM coa_client_vanity_collection WHERE ItemID={}", itemId);
        uint32 cost = row ? row->Fetch()[0].Get<uint32>() : 0;
        if (!cost || cost > Area52Vanity::MaximumPrice)
        {
            handler.SendSysMessage("That item cannot be bought with Bazaar Tokens.");
            return;
        }
        if (cost != displayedCost)
        {
            handler.SendSysMessage("This item's price has changed. Update through the launcher before buying; no tokens were charged.");
            return;
        }
        bool consumable = IsConsumableVanityItem(found->second);
        bool unlockAll = ascensionCompatConfig.GetConfigValue<bool>(AscensionCompatConfig::UNLOCK_ALL_VANITY);
        if (!consumable && (state->OwnedVanityItems.contains(itemId) || (unlockAll && !IsBankVanityItem(itemId))))
        {
            handler.SendSysMessage("You already own this item.");
            return;
        }
        if (!Area52Vanity::Purchase(player, itemId, cost, consumable))
        {
            handler.SendSysMessage("Purchase not completed. Check your carried Bazaar Tokens and bag space, then try again.");
            return;
        }
        if (!consumable)
            CollectItem(player, *state, itemId, true);
        LOG_INFO("coa", "Account {} bought vanity item {} for {} Bazaar Tokens", state->AccountId, itemId, cost);
    }
