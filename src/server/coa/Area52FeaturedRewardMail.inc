struct FeaturedDelivery
{
    Area52MilestoneCollection Collection;
    std::unique_ptr<Mail> Letter;
    std::vector<std::unique_ptr<Item>> Items;
};

void DeliverFeaturedRewards(Player* player, State& state)
{
    if (!Area52MilestoneCatalogReady() || !state.Draft || !state.Loaded || state.Pending || state.Busy || player->GetSession()->IsBot()
        || player->GetPlayerSetting("core.area52.draft_progress", 0).value
        || player->GetPlayerSetting("core.area52.draft_progress", 2).value)
        return;
    auto const now = Clock::now();
    if (now < state.NextReward)
        return;
    state.NextReward = now + std::chrono::seconds(10);
    FeaturedRewardItems items{};
    if (!ValidateFeaturedRewards(state.Plan.Description, items))
        return;
    auto* session = player->GetSession();
    auto const guid = player->GetGUID();
    auto const generation = state.Generation;
    if (!state.RewardsLoaded)
    {
        state.Busy = true;
        auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_FEATURED_MILESTONES);
        query->SetData(0, guid.GetCounter());
        session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
            [session, guid, generation](PreparedQueryResult rows)
            {
                auto* owner = session->GetPlayer();
                auto* current = owner && owner->GetGUID() == guid ? owner->CustomData.Get<State>(StateKey) : nullptr;
                if (!current || current->Generation != generation)
                    return;
                current->Busy = false;
                current->RewardsLoaded = true;
                if (rows)
                    do
                        current->RewardLevels.insert(rows->Fetch()[0].Get<std::uint8_t>());
                    while (rows->NextRow());
            }));
        return;
    }
    std::uint32_t level = 0;
    for (std::size_t i = 0; i < items.size(); ++i)
        if (items[i] && FeaturedRewardLevels[i] <= player->GetLevel()
            && !state.RewardLevels.contains(FeaturedRewardLevels[i])
            && (!level || FeaturedRewardLevels[i] < level))
            level = FeaturedRewardLevels[i];
    if (!level)
        return;
    auto delivery = std::make_shared<FeaturedDelivery>();
    delivery->Letter = std::make_unique<Mail>();
    auto& mail = *delivery->Letter;
    mail.messageID = sObjectMgr->GenerateMailID();
    mail.messageType = MAIL_NORMAL;
    mail.stationery = MAIL_STATIONERY_GM;
    mail.receiver = guid.GetCounter();
    mail.subject = state.Plan.Name + " - Level " + std::to_string(level) + " Vanity Reward";
    mail.body = "Thank you for following your Featured Draft. Enjoy this milestone's assigned Vanity rewards.";
    mail.deliver_time = GameTime::GetGameTime().count();
    mail.expire_time = mail.deliver_time + 90 * DAY;
    mail.state = MAIL_STATE_UNCHANGED;
    for (std::size_t i = 0; i < items.size(); ++i)
        if (items[i] && FeaturedRewardLevels[i] == level)
        {
            std::unique_ptr<Item> item(Item::CreateItem(items[i], 1, player));
            if (!item)
                return;
            mail.AddItem(item->GetGUID().GetCounter(), item->GetEntry());
            delivery->Items.push_back(std::move(item));
        }
    std::uint64_t build = 0;
    if (!ParseId(state.Plan.Id, build))
        return;
    auto transaction = CharacterDatabase.BeginTransaction();
    auto* claim = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_FEATURED_MILESTONE);
    claim->SetData(0, guid.GetCounter());
    claim->SetData(1, level);
    claim->SetData(2, build);
    claim->SetData(3, state.Revision);
    claim->SetData(4, mail.messageID);
    transaction->Append(claim);
    for (auto const& item : delivery->Items)
        AppendArea52MilestoneUnlock(player, item->GetEntry(), transaction, delivery->Collection);
    for (auto const& item : delivery->Items)
        item->SaveToDB(transaction);
    auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL);
    statement->SetData(0, mail.messageID);
    statement->SetData(1, mail.messageType);
    statement->SetData(2, mail.stationery);
    statement->SetData(3, mail.mailTemplateId);
    statement->SetData(4, mail.sender);
    statement->SetData(5, mail.receiver);
    statement->SetData(6, mail.subject);
    statement->SetData(7, mail.body);
    statement->SetData(8, true);
    statement->SetData(9, std::uint32_t(mail.expire_time));
    statement->SetData(10, std::uint32_t(mail.deliver_time));
    statement->SetData(11, mail.money);
    statement->SetData(12, mail.COD);
    statement->SetData(13, mail.checked);
    transaction->Append(statement);
    for (auto const& item : mail.items)
    {
        statement = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL_ITEM);
        statement->SetData(0, mail.messageID);
        statement->SetData(1, item.item_guid);
        statement->SetData(2, mail.receiver);
        transaction->Append(statement);
    }
    state.Busy = true;
    session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(transaction)).AfterComplete(
        [session, guid, generation, level, delivery](bool success)
        {
            if (success)
                sMailMgr->OnMailSent(guid.GetCounter());
            auto* owner = session->GetPlayer();
            auto* current = owner && owner->GetGUID() == guid ? owner->CustomData.Get<State>(StateKey) : nullptr;
            if (success && owner && owner->GetGUID() == guid)
                ConfirmArea52MilestoneUnlock(owner, delivery->Collection);
            if (!current || current->Generation != generation)
                return;
            current->Busy = false;
            if (!success)
            {
                current->RewardsLoaded = false;
                return;
            }
            current->RewardLevels.insert(level);
            owner->AddNewMailDeliverTime(delivery->Letter->deliver_time);
            owner->AddMail(delivery->Letter.release());
            for (auto& item : delivery->Items)
                owner->AddMItem(item.release());
        });
}
