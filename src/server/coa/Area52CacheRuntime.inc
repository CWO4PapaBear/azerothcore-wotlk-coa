// Included by AscensionCacheRewards.cpp after its namespace closes.
namespace Area52CacheRuntime
{
namespace
{
#include "Area52CacheClassRefresh.inc"
constexpr uint32 Menu = 905206;
struct Session
{
    uint64 generation = 0;
    bool busy = false;
    std::optional<Area52CacheAttempt::State> prompt;
    Options options;
    uint32 promptMenu = 0;
    std::size_t refreshed = 0;
};
struct Request
{
    ObjectGuid owner;
    ObjectGuid cache;
    uint64 generation;
    Options options;
    std::optional<Area52CacheAttempt::State> answer;
    bool accept = false;
};
std::mutex RuntimeLock;
std::map<ObjectGuid, Session> Sessions;
std::deque<Request> Requests;
uint64 Generation = 0;
uint32 MenuSequence = Menu;
std::atomic<bool> Enabled{false};
bool Started = false;
bool Ready = false;
bool Reload = false;
AsyncCallbackProcessor<QueryCallback> Reads;

bool Applies(Player* player)
{
    return Enabled && player && player->getClass() == 10;
}

void Finish(Request const& request)
{
    std::lock_guard lock(RuntimeLock);
    auto found = Sessions.find(request.owner);
    if (found != Sessions.end() && found->second.generation == request.generation)
        found->second.busy = false;
}

Player* Current(Request const& request)
{
    std::lock_guard lock(RuntimeLock);
    auto found = Sessions.find(request.owner);
    if (found == Sessions.end() || found->second.generation != request.generation)
        return nullptr;
    return ObjectAccessor::FindConnectedPlayer(request.owner);
}

bool Save(std::optional<Area52CacheAttempt::State> const& before, Area52CacheAttempt::State const& after)
{
    auto transaction = CharacterDatabase.BeginTransaction();
    if (!Area52CacheAttempt::AppendStoredMutation(transaction, before, after))
        return false;
    return CharacterDatabase.AsyncCommitTransaction(transaction).m_future.get();
}

void Show(Player* player, Request const& request, Area52CacheAttempt::State const& state)
{
    uint32 menu = 0;
    {
        std::lock_guard lock(RuntimeLock);
        auto found = Sessions.find(request.owner);
        if (found == Sessions.end() || found->second.generation != request.generation)
            return;
        found->second.prompt = state;
        found->second.options = request.options;
        menu = ++MenuSequence;
        if (!menu) menu = ++MenuSequence;
        found->second.promptMenu = menu;
    }
    auto const* item = sObjectMgr->GetItemTemplate(state.offered);
    if (!item)
        return;
    ClearGossipMenuFor(player);
    player->PlayerTalkClass->GetGossipMenu().SetMenuId(menu);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT,
        "You already own this appearance. Receive " + item->Name1 + " again", Menu, 1);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Choose a different item", Menu, 2);
    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, player->GetGUID());
}

void CompleteRead(Request request, QueryResult rows)
{
    Player* player = Current(request);
    if (!player)
        return;
    auto stop = [&](char const* message)
    {
        ChatHandler(player->GetSession()).SendSysMessage(message);
        Finish(request);
    };
    Item* cache = player->GetItemByGuid(request.cache);
    if (!Applies(player) || !Ready || !cache || cache->IsInTrade()
        || !RewardCap(player, request.options) || player->GetLevel() < RewardCap(player, request.options))
    {
        stop("The cache is not available to open now. It has not been consumed.");
        return;
    }
    auto read = Area52CacheAttempt::DecodeStored(rows, request.owner.GetCounter(), request.cache.GetCounter());
    if (read.status == Area52CacheAttempt::ReadStatus::Unavailable)
    {
        stop("The saved cache reward could not be read. Nothing was consumed; try again shortly.");
        return;
    }
    auto state = read.state;
    if (!request.options.prestige && state && state->offered
        && (state->phase == Area52CacheAttempt::Phase::Choosing || state->phase == Area52CacheAttempt::Phase::Ready))
    {
        auto const* item = sObjectMgr->GetItemTemplate(state->offered);
        float const average = Area52CacheItemLevel::Equipped(player);
        if (!item || item->RequiredLevel != RewardCap(player, request.options)
            || item->ItemLevel < average || item->ItemLevel > average + 6.0f
            || !CallboardDropMaps.contains(state->offered))
        {
            if (request.answer)
            {
                stop("Your Callboard reward eligibility changed. Open the cache again.");
                return;
            }
            auto next = SelectAttempt(player, cache, request.options, state);
            if (!next || !Save(state, *next))
            {
                Finish(request);
                return;
            }
            state = next;
        }
    }
    if (request.answer)
    {
        auto const& expected = *request.answer;
        if (!state || Area52CacheAttempt::Encode(*state) != Area52CacheAttempt::Encode(expected))
        {
            stop("That cache choice is no longer current. Open the cache again.");
            return;
        }
        auto next = *state;
        if (Area52CacheAttempt::Answer(next, expected.owner, expected.cache, expected.request,
            expected.revision, expected.offered, request.accept) != Area52CacheAttempt::Result::Changed
            || !Save(state, next))
        {
            stop("The cache choice could not be saved. Open it again to check the saved result.");
            return;
        }
        state = next;
    }
    if (!state || (state->phase == Area52CacheAttempt::Phase::Choosing && !state->offered))
    {
        auto selected = SelectAttempt(player, cache, request.options, state);
        if (!selected)
        {
            Finish(request); // Selector explains an empty/blocked pool without consuming the cache.
            return;
        }
        if (!Save(state, *selected))
        {
            stop("The cache reward could not be saved. Open it again to check the saved result.");
            return;
        }
        state = selected;
    }
    if (state->phase == Area52CacheAttempt::Phase::Choosing)
        Show(player, request, *state);
    else if (state->phase == Area52CacheAttempt::Phase::Ready)
    {
        if (!StillEligible(player, *state, request.options))
            ChatHandler(player->GetSession()).SendSysMessage("Your saved reward is not currently eligible. The cache and exact reward were kept.");
        else
        {
            if (AscensionCacheRewards::DeliverReserved(player, *state, cache) && state->offered == 975001
                && state->offeredCount == 10000)
                ChatHandler(player->GetSession()).SendSysMessage("You own every eligible reward for this build. You're insane! Enjoy 10,000 Bazaar Tokens.");
        }
    }
    else
        ChatHandler(player->GetSession()).SendSysMessage("This cache already has a completed or unresolved delivery. It cannot be opened again.");
    Finish(request);
}

class RuntimeWorld final : public WorldScript
{
public:
    RuntimeWorld() : WorldScript("Area52CacheRuntime", {WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_STARTUP, WORLDHOOK_ON_UPDATE}) { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true)
            && sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero"
            && sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live"
            && sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) == 0;
        std::lock_guard lock(RuntimeLock);
        Reload = true;
    }
    void OnStartup() override { Started = true; }
    void OnUpdate(uint32) override
    {
        std::deque<Request> requests;
        bool reload = false;
        {
            std::lock_guard lock(RuntimeLock);
            reload = Reload && Started;
            if (reload) Reload = false;
            requests.swap(Requests);
        }
        if (reload)
            Ready = Enabled && LoadCatalog();
        if (Ready)
        {
            std::lock_guard lock(RuntimeLock);
            for (auto& [owner, session] : Sessions)
                if (Player* player = ObjectAccessor::FindConnectedPlayer(owner); Applies(player))
                    for (uint32 batch = 0; batch < 32 && session.refreshed < CacheClassRefresh.size(); ++batch)
                        player->GetSession()->SendItemQuerySingleResponse(CacheClassRefresh[session.refreshed++]);
        }
        Reads.ProcessReadyCallbacks();
        for (auto const& request : requests)
        {
            if (!Current(request))
                continue;
            Reads.AddCallback(Area52CacheAttempt::LoadStored(request.cache.GetCounter())
                .WithCallback([request](QueryResult rows) { CompleteRead(request, rows); }));
        }
    }
};

class RuntimePlayer final : public PlayerScript
{
public:
    RuntimePlayer() : PlayerScript("Area52CacheSessions", {PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LOGOUT, PLAYERHOOK_ON_GOSSIP_SELECT}) { }
    void OnPlayerLogin(Player* player) override
    {
        std::lock_guard lock(RuntimeLock);
        Sessions[player->GetGUID()] = Session{++Generation, false, {}, {}};
    }
    void OnPlayerLogout(Player* player) override
    {
        std::lock_guard lock(RuntimeLock);
        Sessions.erase(player->GetGUID());
    }
    void OnPlayerGossipSelect(Player* player, uint32 menu, uint32 sender, uint32 action) override
    {
        if (!Applies(player) || sender != Menu || (action != 1 && action != 2))
            return;
        std::lock_guard lock(RuntimeLock);
        auto found = Sessions.find(player->GetGUID());
        if (found == Sessions.end() || found->second.busy || !found->second.prompt || found->second.promptMenu != menu)
            return;
        auto& session = found->second;
        auto const state = *session.prompt;
        session.busy = true;
        session.prompt.reset();
        Requests.push_back({player->GetGUID(), ObjectGuid::Create<HighGuid::Item>(state.cache),
            session.generation, session.options, state, action == 1});
        CloseGossipMenuFor(player);
    }
};
}

bool QueueOpen(Player* player, Item* item, Options options)
{
    if (!Applies(player) || !item)
        return false;
    player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
    std::lock_guard lock(RuntimeLock);
    auto found = Sessions.find(player->GetGUID());
    if (found == Sessions.end())
    {
        ChatHandler(player->GetSession()).SendSysMessage("Cache selection is still initializing. Try again shortly.");
        return true;
    }
    if (found->second.busy)
        return true;
    found->second.busy = true;
    found->second.prompt.reset();
    Requests.push_back({player->GetGUID(), item->GetGUID(), found->second.generation, options, {}, false});
    return true;
}
}

void AddArea52CacheRuntime()
{
    new Area52CacheRuntime::RuntimeWorld();
    new Area52CacheRuntime::RuntimePlayer();
}
