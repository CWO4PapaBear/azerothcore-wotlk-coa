    uint32 MasterQuote(PlayerCollectionState const& state, uint32 item, std::vector<uint32>& missing)
    {
        auto const bundle = Area52Master::Bundles.find(item);
        if (bundle == Area52Master::Bundles.end())
            return Area52Master::Offers.at(item).Price;
        uint32 total = 0;
        for (uint32 part : bundle->second)
        {
            auto appearance = _itemAppearances.find(part);
            if (state.OwnedVanityItems.contains(part) || (!Area52Master::IsHeirloomCache(item) && appearance != _itemAppearances.end()
                && state.CollectedAppearances.contains(appearance->second)))
                continue;
            missing.push_back(part);
            total += Area52Master::IsHeirloomCache(item) ? 400 : Area52Master::Offers.at(part).Price;
        }
        return total;
    }

    bool BuyMasterOffer(Player* player, uint32 item, uint32 currency, uint32 price)
    {
        auto state = GetState(player);
        auto offer = Area52Master::Offers.find(item);
        if (!state || offer == Area52Master::Offers.end() || currency != offer->second.Currency
            || !AscensionFreePick::Applies(player) || !sObjectMgr->GetItemTemplate(item)
            || state->OwnedVanityItems.contains(item))
            return false;
        std::vector<uint32> missing;
        if (MasterQuote(*state, item, missing) != price)
            return false;
        std::map<uint32,uint32> bundleAppearances;
        if (auto bundle = Area52Master::Bundles.find(item); bundle != Area52Master::Bundles.end())
            for (uint32 part : bundle->second)
            {
                auto appearance = _itemAppearances.find(part);
                bundleAppearances.emplace(part, appearance == _itemAppearances.end() ? 0 : appearance->second);
            }
        if (!Area52Vanity::Purchase(player, item, price, false, currency, missing, bundleAppearances))
            return false;
        for (uint32 part : missing)
            CollectItem(player, *state, part, true);
        CollectItem(player, *state, item, true);
        return true;
    }

    void GrantCompletedHeirloomCaches(Player* player, PlayerCollectionState& state)
    {
        state.CheckHeirloomCaches = false;
        for (auto const& [cache, parts] : Area52Master::Bundles)
        {
            if (!Area52Master::IsHeirloomCache(cache) || state.OwnedVanityItems.contains(cache))
                continue;
            if (!std::all_of(parts.begin(), parts.end(), [&](uint32 part) { return state.OwnedVanityItems.contains(part); }))
                continue;
            if (!BuyMasterOffer(player, cache, Area52Master::Offers.at(cache).Currency, 0))
                ChatHandler(player->GetSession()).SendSysMessage("Your heirloom set is complete. Free a bag slot and claim its free cache in Vanity.");
        }
    }

    bool HandleMasterStore(Player* player, WorldPacket const& incoming)
    {
        if (!AscensionFreePick::Applies(player))
            return false;
        WorldPacket read(incoming);
        uint32 key = 0;
        read >> key;
        if (incoming.GetOpcode() == CMSG_QUERY_CUSTOM_STORE)
        {
            if (key != 1 && key != 2)
                return false;
            auto state = GetState(player);
            WorldPacket result(SMSG_QUERY_CUSTOM_STORE_RESULT, 1400);
            result << "QUERY_CUSTOM_STORE_OK";
            auto countAt = result.wpos();
            uint32 count = 0;
            result << count;
            if (state)
                for (auto const& [item, offer] : Area52Master::Offers)
                {
                    if (offer.Store != key || state->OwnedVanityItems.contains(item))
                        continue;
                    std::vector<uint32> missing;
                    auto cost = MasterQuote(*state, item, missing);
                    // Bind the price shown to the click, even if account ownership changes.
                    uint32 quoteKey = 0x80000000u | (cost << 24) | item;
                    result << quoteKey << key << item << uint32(0) << offer.Currency;
                    for (unsigned i = 0; i < 4; ++i) result << uint32(0);
                    result << cost;
                    for (unsigned i = 0; i < 6; ++i) result << uint32(0);
                    ++count;
                }
            result.put<uint32>(countAt, count);
            player->SendDirectMessage(&result);
            return true;
        }
        if (!(key & 0x80000000u))
            return false;
        uint32 item = key & 0xffffffu;
        auto offer = Area52Master::Offers.find(item);
        if (offer == Area52Master::Offers.end() || !offer->second.Store)
            return false;
        uint32 quantity = 0;
        read >> quantity;
        bool ok = quantity == 1 && BuyMasterOffer(player, item, offer->second.Currency, (key >> 24) & 127);
        WorldPacket result(0x06BC, 64);
        result << (ok ? "PURCHASE_CUSTOM_STORE_ITEM_OK" : "Purchase declined. Refresh the rewards menu.");
        player->SendDirectMessage(&result);
        return true;
    }
