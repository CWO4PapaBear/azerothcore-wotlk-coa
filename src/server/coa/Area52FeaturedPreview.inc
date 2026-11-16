struct PreviewRequest
{
    WorldSession* Owner = nullptr;
    uint32 Account = 0;
    uint32 Serial = 0;
    uint32 Archetype = 0;
    Clock::time_point Received{};
};

bool ValidatePreviewEffects(FeaturedPreviewEffects const& effects)
{
    for (std::size_t i = 0; i < 2; ++i)
        if (effects[i] && effects[i] != FeaturedPreviewDefault && !sObjectMgr->GetCreatureTemplate(effects[i]))
            return false;
    for (std::size_t i = 2; i < effects.size(); ++i)
        if (effects[i] && effects[i] != FeaturedPreviewDefault && !sSpellMgr->GetSpellInfo(effects[i]))
            return false;
    return true;
}

std::mutex PreviewLock;
std::unordered_map<uint32, PreviewRequest> PreviewRequests;
std::unordered_map<uint32, Clock::time_point> PreviewCooldowns;

class FeaturedPreviewServer final : public ServerScript
{
public:
    FeaturedPreviewServer() : ServerScript("Area52FeaturedPreviewServer",
        { SERVERHOOK_CAN_PACKET_RECEIVE_EARLY }) { }

    bool CanPacketReceiveEarly(WorldSession* session, WorldPacket const& packet) override
    {
        if (packet.GetOpcode() != FeaturedPreviewRequest)
            return true;
        if (!Enabled || !session || packet.size() != 12)
            return false;
        auto const version = packet.read<uint32>(0);
        auto const serial = packet.read<uint32>(4);
        auto const archetype = packet.read<uint32>(8);
        if (version != FeaturedPreviewVersion || !serial || !archetype || archetype > 65535)
            return false;
        std::lock_guard lock(PreviewLock);
        auto const now = Clock::now();
        auto const account = session->GetAccountId();
        if (auto it = PreviewCooldowns.find(account); it != PreviewCooldowns.end() && now < it->second)
            return false;
        if (PreviewCooldowns.size() >= 4096)
            std::erase_if(PreviewCooldowns, [now](auto const& row) { return row.second < now; });
        if (PreviewRequests.size() >= 1024 || PreviewCooldowns.size() >= 4096)
            return false;
        PreviewCooldowns[account] = now + std::chrono::milliseconds(250);
        PreviewRequests[account] = { session, account, serial, archetype, now };
        return false;
    }
};

class FeaturedPreviewWorld final : public WorldScript
{
public:
    FeaturedPreviewWorld() : WorldScript("Area52FeaturedPreviewWorld", { WORLDHOOK_ON_UPDATE }) { }

    void OnUpdate(uint32) override
    {
        std::vector<PreviewRequest> requests;
        {
            std::lock_guard lock(PreviewLock);
            if (!Enabled)
            {
                PreviewRequests.clear();
                PreviewCooldowns.clear();
                return;
            }
            while (!PreviewRequests.empty() && requests.size() < 16)
            {
                auto it = PreviewRequests.begin();
                requests.push_back(it->second);
                PreviewRequests.erase(it);
            }
        }
        for (auto const& request : requests)
        {
            auto* session = sWorldSessionMgr->FindSession(request.Account);
            if (!session || session != request.Owner || Clock::now() - request.Received > std::chrono::seconds(5))
                continue;
            auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_CREATION_DRAFT);
            query->SetData(0, request.Archetype);
            session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
                [session, request](PreparedQueryResult rows)
                {
                    if (!Enabled || Clock::now() - request.Received > std::chrono::seconds(5))
                        return;
                    uint32 revision = 0;
                    uint32 status = 1;
                    FeaturedRewardItems items{};
                    FeaturedPreviewEffects effects{};
                    Entry entry;
                    if (rows && ReadRecord(rows->Fetch(), entry) && DraftArchetype(entry) == request.Archetype
                        && (entry.Description.find("###AREA52_FEATURED_VANITY\n") != std::string::npos
                            || entry.Description.find("###AREA52_FEATURED_PREVIEW\n") != std::string::npos)
                        && ValidateFeaturedRewards(entry.Description, items)
                        && ReadFeaturedPreview(entry.Description, effects) && ValidatePreviewEffects(effects))
                    {
                        revision = rows->Fetch()[1].Get<uint32>();
                        status = revision ? 0 : 1;
                    }
                    if (status)
                    {
                        items.fill(0);
                        effects.fill(0);
                    }
                    WorldPacket response(FeaturedPreviewResponse, 92);
                    response << FeaturedPreviewVersion << request.Serial << request.Archetype << revision << status;
                    for (auto value : items)
                        response << value;
                    for (auto value : effects)
                        response << value;
                    session->SendPacket(&response);
                }));
        }
    }
};
