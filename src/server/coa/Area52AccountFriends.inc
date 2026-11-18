namespace Area52AccountFriends
{
    struct Entry
    {
        ObjectGuid OriginalGuid;
        std::string OriginalName;
        std::string Note;
    };

    using Friends = std::map<uint32, Entry>;
    std::map<uint32, Friends> Loaded;

    uint32 Account(ObjectGuid guid)
    {
        return sCharacterCache->GetCharacterAccountIdByGuid(guid);
    }

    bool Reload(uint32 account)
    {
        auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_ACCOUNT_FRIENDS);
        statement->SetData(0, account);
        auto result = CharacterDatabase.Query(statement);
        if (!result)
        {
            Loaded.erase(account);
            LOG_ERROR("entities.player", "Area52 account friends could not be loaded");
            return false;
        }

        Friends friends;
        do
        {
            Field* fields = result->Fetch();
            uint32 target = fields[0].Get<uint32>();
            if (target && target != account)
                friends.emplace(target, Entry{ObjectGuid::Create<HighGuid::Player>(fields[1].Get<uint32>()),
                    fields[2].Get<std::string>(), fields[3].Get<std::string>()});
        } while (result->NextRow());
        Loaded[account] = std::move(friends);
        return true;
    }

    Friends const* Get(uint32 account)
    {
        if (!account || (!Loaded.count(account) && !Reload(account)))
            return nullptr;
        return &Loaded.at(account);
    }

    bool Has(ObjectGuid owner, ObjectGuid target)
    {
        auto const* friends = Get(Account(owner));
        return friends && friends->count(Account(target));
    }

    bool Add(ObjectGuid owner, ObjectGuid target)
    {
        uint32 account = Account(owner);
        uint32 other = Account(target);
        auto const* friends = Get(account);
        auto const* character = sCharacterCache->GetCharacterCacheByGuid(target);
        if (!friends || !character || !other || account == other)
            return false;
        if (friends->count(other))
            return true;
        if (friends->size() >= SOCIALMGR_FRIEND_LIMIT)
            return false;

        auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_ACCOUNT_FRIEND);
        statement->SetData(0, account);
        statement->SetData(1, other);
        statement->SetData(2, target.GetCounter());
        statement->SetData(3, character->Name);
        statement->SetData(4, std::string());
        CharacterDatabase.DirectExecute(statement);
        return Reload(account) && Loaded.at(account).count(other);
    }

    void Remove(ObjectGuid owner, ObjectGuid target)
    {
        uint32 account = Account(owner);
        auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_DEL_A52_ACCOUNT_FRIEND);
        statement->SetData(0, account);
        statement->SetData(1, Account(target));
        CharacterDatabase.DirectExecute(statement);
        Reload(account);
    }

    void SetNote(ObjectGuid owner, ObjectGuid target, std::string note)
    {
        utf8truncate(note, 48);
        uint32 account = Account(owner);
        auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_UPD_A52_ACCOUNT_FRIEND_NOTE);
        statement->SetData(0, note);
        statement->SetData(1, account);
        statement->SetData(2, Account(target));
        CharacterDatabase.DirectExecute(statement);
        Reload(account);
    }

    ObjectGuid Display(Player* observer, uint32 account, Entry const& entry, ObjectGuid excluded)
    {
        if (WorldSession* session = sWorldSessionMgr->FindSession(account))
            if (Player* current = session->GetPlayer())
                if (current->GetGUID() != excluded && current->IsInWorld())
                {
                    FriendInfo info;
                    SocialMgr::GetFriendInfo(observer, current->GetGUID(), info);
                    if (info.Status != FRIEND_STATUS_OFFLINE)
                        return current->GetGUID();
                }

        if (Account(entry.OriginalGuid) == account)
            return entry.OriginalGuid;

        auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_FRIEND_FALLBACK);
        statement->SetData(0, account);
        if (auto result = CharacterDatabase.Query(statement))
        {
            auto guid = ObjectGuid::Create<HighGuid::Player>(result->Fetch()[0].Get<uint32>());
            if (Account(guid) == account)
                return guid;
        }
        return ObjectGuid::Empty;
    }

    void String(WorldPacket& packet, std::string const& value)
    {
        packet << uint32(value.size());
        packet.append(value.data(), value.size());
    }

    template<class Contacts>
    void SendMetadata(Player* player, Contacts const& contacts)
    {
        auto const* friends = Get(player->GetSession()->GetAccountId());
        if (!friends)
            return;
        WorldPacket packet(0x9C0, 128);
        packet << uint32(0);
        uint32 count = 0;
        for (auto const& [guid, info] : contacts)
        {
            if (!(info.Flags & SOCIAL_FLAG_FRIEND) || count >= SOCIALMGR_FRIEND_LIMIT)
                continue;
            auto const* character = sCharacterCache->GetCharacterCacheByGuid(guid);
            if (!character)
                continue;
            auto found = friends->find(character->AccountId);
            if (found == friends->end())
                continue;
            String(packet, character->Name);
            String(packet, found->second.OriginalName);
            ++count;
        }
        packet.put<uint32>(0, count);
        player->SendDirectMessage(&packet);
    }
}

void PlayerSocial::ProjectArea52AccountFriends(Player* player, ObjectGuid excluded)
{
    for (auto itr = m_playerSocialMap.begin(); itr != m_playerSocialMap.end();)
    {
        itr->second.Flags &= ~SOCIAL_FLAG_FRIEND;
        if (!itr->second.Flags)
            itr = m_playerSocialMap.erase(itr);
        else
            ++itr;
    }
    auto const* friends = Area52AccountFriends::Get(player->GetSession()->GetAccountId());
    if (!friends)
        return;
    for (auto const& [account, entry] : *friends)
    {
        ObjectGuid guid = Area52AccountFriends::Display(player, account, entry, excluded);
        if (!guid)
            continue;
        auto& info = m_playerSocialMap[guid];
        info.Flags |= SOCIAL_FLAG_FRIEND;
        info.Note = entry.Note;
    }
}
