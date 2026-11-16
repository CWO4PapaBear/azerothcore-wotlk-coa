#include "Area52FeaturedIncarnations.h"
#include "Area52FeaturedPets.h"
#include "Area52DraftPet.h"
#include "Pet.h"
#include "ObjectMgr.h"
#include "Map.h"
#include "Area52ActiveBuild.h"
#include "Area52BuildProtocol.h"
#include "Area52DraftAssignment.h"
#include "Area52DraftLifecycle.h"
#include "Area52FeaturedRewardValidation.h"
#include "Area52FeaturedMilestoneCollection.h"
#include "GameTime.h"
#include "Item.h"
#include "Mail.h"
#include "MailMgr.h"
#include <memory>
#include <set>
#include "AscensionMysticEnchantCatalog.h"
#include "AscensionCompatOpcodes.h"
#include "AscensionCoAConfig.h"
#include "AscensionFreePick.h"
#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Trainer.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <deque>
#include <mutex>
#include <unordered_map>

namespace Area52Build
{
namespace
{
constexpr char StateKey[] = "coa.area52.active_build";
constexpr char Setting[] = "core.area52.active_build";
std::atomic<bool> Enabled{false};
std::atomic<std::uint64_t> NextGeneration{1};
using Clock = std::chrono::steady_clock;
std::mutex InboxLock;
std::unordered_map<WorldSession*, std::deque<WorldPacket>> Inboxes;

struct State : DataMap::Base
{
    std::uint64_t Generation = NextGeneration.fetch_add(1);
    Entry Plan;
    std::uint32_t Revision = 0;
    bool Loaded = false;
    bool Busy = false;
    bool Automatic = false;
    bool Draft = false;
    bool RewardsLoaded = false;
    std::set<std::uint32_t> RewardLevels;
    Clock::time_point NextReward{};
    bool InternalSelection = false;
    bool Pending = false;
    std::string LastFailure;
    Clock::time_point NextRequest{};
};

bool Applies(Player* player)
{
    return Enabled && player && AscensionFreePick::Applies(player);
}

bool Safe(Player* player)
{
    return !Area52DraftPetBusy(player) && player->IsInWorld() && player->FindMap() && player->IsAlive() && !player->IsInCombat()
        && !player->InBattleground() && !player->InArena() && !player->IsBeingTeleported();
}

bool ParseId(std::string const& value, std::uint64_t& id)
{
    auto const parsed = std::from_chars(value.data(), value.data() + value.size(), id);
    return parsed.ec == std::errc{} && parsed.ptr == value.data() + value.size() && id;
}

bool DecodeHex(std::string const& hex, Entry& entry)
{
    if (hex.size() % 2 || hex.size() > MaxPacketBytes * 2)
        return false;
    std::vector<std::uint8_t> bytes;
    for (std::size_t i = 0; i < hex.size(); i += 2)
    {
        unsigned value = 0;
        auto const parsed = std::from_chars(hex.data() + i, hex.data() + i + 2, value, 16);
        if (parsed.ec != std::errc{} || parsed.ptr != hex.data() + i + 2)
            return false;
        bytes.push_back(std::uint8_t(value));
    }
    return Decode(bytes, entry);
}

void SendState(Player* player, State const& state)
{
    WorldPacket packet(0x633, 64);
    auto const active = AscensionFreePick::ActiveSpecialization(player);
    packet << std::uint32_t(AscensionFreePick::SpecializationSpells.size());
    for (std::uint32_t spec = 0; spec < AscensionFreePick::SpecializationSpells.size(); ++spec)
    {
        auto const key = "core.area52.spec." + std::to_string(spec) + ".active_build";
        auto const* saved = player->FindPlayerSettings(key);
        std::uint64_t const id = saved && saved->size() >= 4 ?
            std::uint64_t((*saved)[0].value) | (std::uint64_t((*saved)[1].value) << 32) : 0;
        bool const automatic = spec == active ? state.Automatic : id && (*saved)[3].value;
        packet << (spec == active ? state.Plan.Id : id ? std::to_string(id) : std::string{})
            << std::uint8_t(automatic) << std::uint8_t(automatic);
    }
    player->GetSession()->SendPacket(&packet);
    WorldPacket changed(0x632, 64);
    changed << active << state.Plan.Id
        << std::uint8_t(state.Automatic) << std::uint8_t(state.Automatic);
    player->GetSession()->SendPacket(&changed);
}

void Reply(Player* player, bool activate, char const* result)
{
    WorldPacket packet(activate ? 0x5C3 : 0x5C5, 64);
    packet << result;
    player->GetSession()->SendPacket(&packet);
}

void Store(Player* player, State const& state)
{
    std::uint64_t id = 0;
    ParseId(state.Plan.Id, id);
    player->UpdatePlayerSetting(Setting, 0, std::uint32_t(id));
    player->UpdatePlayerSetting(Setting, 1, std::uint32_t(id >> 32));
    player->UpdatePlayerSetting(Setting, 2, state.Revision);
    player->UpdatePlayerSetting(Setting, 3, state.Automatic ? 1 : 0);
    player->UpdatePlayerSetting(Setting, 4, state.Draft ? 1 : 0);
}

bool Learn(Player* player, State& state, std::uint32_t maximumLevel, bool learnSpells = true)
{
    if (!Safe(player) || state.Plan.Id.empty())
        return false;
    std::vector<AscensionFreePick::KnownEntry> wanted;
    auto result = AscensionFreePick::PlanSelection(player, state.Plan, wanted, true, maximumLevel, learnSpells);
    if (result && AscensionFreePick::SameSelection(wanted, AscensionFreePick::KnownEntries(player)))
    {
        state.LastFailure.clear();
        return true;
    }
    if (result)
    {
        state.InternalSelection = true;
        result = AscensionFreePick::Apply(player, wanted, false);
        state.InternalSelection = false;
    }
    if (!result)
    {
        std::string const failure = std::string(result.Status) + " " + result.Learn;
        if (failure != state.LastFailure)
            ChatHandler(player->GetSession()).PSendSysMessage("Active build could not be learned: {}", failure);
        state.LastFailure = failure;
        return false;
    }
    state.LastFailure.clear();
    SendArea52Advancement(player);
    return true;
}

#include "Area52FeaturedRewardMail.inc"

void Fetch(Player* player, std::uint64_t id, std::uint32_t revision, bool automatic, bool restore)
{
    auto const archetype = CreationArchetypeFromReference(id);
    if (id >= CreationReferenceBase && (!archetype || restore || player->GetLevel() != 1))
    {
        Reply(player, true, "ACTIVATE_BUILD_NOT_AVAILABLE");
        return;
    }
    if (archetype)
        automatic = true;

    auto* state = player->CustomData.GetDefault<State>(StateKey);
    state->Busy = true;
    WorldSession* session = player->GetSession();
    ObjectGuid const guid = player->GetGUID();
    auto const generation = state->Generation;
    auto* query = CharacterDatabase.GetPreparedStatement(archetype ? CHAR_SEL_A52_CREATION_DRAFT
        : restore ? CHAR_SEL_A52_ACTIVE_REVISION : CHAR_SEL_A52_BUILD_PUBLIC);
    query->SetData(0, archetype ? archetype : id);
    if (!archetype)
        query->SetData(1, restore ? revision : session->GetAccountId());
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, guid, generation, automatic, restore, id, archetype](PreparedQueryResult rows)
        {
            Player* owner = session->GetPlayer();
            if (!Applies(owner) || owner->GetGUID() != guid)
                return;
            auto* current = owner->CustomData.Get<State>(StateKey);
            if (!current || current->Generation != generation)
                return;
            current->Busy = false;
            Entry plan;
            if (!rows || !DecodeHex(rows->Fetch()[2].Get<std::string>(), plan)
                || (!archetype && plan.Id != std::to_string(id))
                || (archetype && DraftArchetype(plan) != archetype))
            {
                if (restore)
                {
                    current->Plan = {};
                    current->Revision = 0;
                    current->Automatic = false;
                    current->Loaded = true;
                    Store(owner, *current);
                    SendState(owner, *current);
                    ChatHandler(session).SendSysMessage("Your saved active build is unavailable. Learned abilities were retained.");
                }
                else
                    Reply(owner, true, "ACTIVATE_BUILD_NOT_FOUND");
                return;
            }
            bool const drafted = restore ? owner->GetPlayerSetting(Setting, 4).value != 0
                : rows->Fetch()[3].Get<std::uint8_t>() != 0;
            if ((drafted && (plan.Category != FeaturedDraftCategory || !DraftArchetype(plan)))
                || (!restore && (drafted || current->Draft) && !AscensionMysticEnchant::DraftEquipmentReady(owner)))
            {
                Reply(owner, true, "ACTIVATE_BUILD_NOT_AVAILABLE");
                return;
            }
            std::uint64_t previousId = 0;
            ParseId(current->Plan.Id, previousId);
            std::uint64_t nextId = 0;
            ParseId(plan.Id, nextId);
            DraftLifecycle progress{current->Draft ? previousId : 0, current->Draft ? current->Revision : 0,
                owner->GetPlayerSetting("core.area52.draft_progress", 0).value != 0,
                owner->GetPlayerSetting("core.area52.draft_progress", 1).value != 0,
                owner->GetPlayerSetting("core.area52.draft_progress", 2).value != 0};
            if (!restore)
                progress = SwitchFeaturedDraft(progress, owner->GetLevel(), nextId,
                    rows->Fetch()[1].Get<std::uint32_t>(), drafted);
            bool const learnAutomatically = drafted || automatic;
            bool const grantDraftEnchants = drafted && !progress.EntitlementForfeited;
            if (!restore && grantDraftEnchants && !AscensionMysticEnchant::CanApplyDraftEnchants(owner, plan.Enchants))
            {
                Reply(owner, true, "ACTIVATE_BUILD_NOT_AVAILABLE");
                return;
            }
            if (!restore && !Safe(owner))
            {
                Reply(owner, true, "ACTIVATE_BUILD_NOT_AVAILABLE");
                return;
            }
            std::vector<AscensionFreePick::KnownEntry> wanted;
            auto result = AscensionFreePick::PlanSelection(owner, plan, wanted, false, 80, learnAutomatically);
            if (!result && !restore)
            {
                Reply(owner, true, *result.Learn ? result.Learn : result.Status);
                return;
            }
            Entry const previousPlan = current->Plan;
            bool const previousDraft = current->Draft;
            auto const previousSelection = AscensionFreePick::KnownEntries(owner);
            auto const previousRevision = current->Revision;
            bool const previousAutomatic = current->Automatic;
            current->Plan = std::move(plan);
            current->Revision = rows->Fetch()[1].Get<std::uint32_t>();
            current->Automatic = learnAutomatically;
            current->Draft = drafted;
            current->Loaded = true;
            if (!restore)
            {
                current->InternalSelection = true;
                result = AscensionFreePick::Apply(owner, wanted);
                current->InternalSelection = false;
                if (result && ((grantDraftEnchants
                    && !AscensionMysticEnchant::ApplyDraftEnchants(owner, current->Plan.Enchants))
                    || (!grantDraftEnchants && previousDraft && !AscensionMysticEnchant::ReleaseDraftEnchants(owner, previousId, previousRevision))))
                    result = { "ACTIVATE_BUILD_NOT_AVAILABLE" };
                if (!result)
                {
                    current->InternalSelection = true;
                    AscensionFreePick::Apply(owner, previousSelection);
                    current->InternalSelection = false;
                    current->Draft = previousDraft;
                    if (previousDraft && !owner->GetPlayerSetting("core.area52.draft_progress", 2).value)
                        AscensionMysticEnchant::ApplyDraftEnchants(owner, previousPlan.Enchants);
                    current->Plan = previousPlan;
                    current->Revision = previousRevision;
                    current->Automatic = previousAutomatic;
                    Store(owner, *current);
                    Reply(owner, true, *result.Learn ? result.Learn : result.Status);
                    return;
                }
                owner->UpdatePlayerSetting("core.area52.draft_progress", 0, progress.Forfeited ? 1 : 0);
                owner->UpdatePlayerSetting("core.area52.draft_progress", 2, progress.EntitlementForfeited ? 1 : 0);
                Store(owner, *current);
                owner->SaveToDB(false, false);
                SendArea52Advancement(owner);
            }
            current->Pending = true;
            SendState(owner, *current);
            if (!restore)
                Reply(owner, true, "ACTIVATE_BUILD_OK");
        }));
}

bool Handle(WorldSession* session, WorldPacket const& packet)
{
    if (!Enabled || !session)
        return false;
    std::lock_guard lock(InboxLock);
    auto it = Inboxes.find(session);
    if (it != Inboxes.end() && packet.size() <= 70 && it->second.size() < 2)
        it->second.push_back(packet);
    return true;
}

class ActiveWorld final : public WorldScript
{
public:
    ActiveWorld() : WorldScript("Area52ActiveBuildWorld", { WORLDHOOK_ON_AFTER_CONFIG_LOAD }) { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.FreePickBuilds.Apply", false)
            && sConfigMgr->GetOption<bool>("CoA.FreePickBuilds.Enable", false)
            && sConfigMgr->GetOption<bool>("CoA.Enable", true)
            && sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero"
            && sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live"
            && sConfigMgr->GetOption<std::uint32_t>("CoA.GameModeMask", 0) == 0;
    }
};

class ActiveTrainer final : public AllCreatureScript
{
public:
    ActiveTrainer() : AllCreatureScript("Area52ActiveBuildTrainer") { }
    bool CanCreatureGossipHello(Player* player, Creature* creature) override
    {
        if (Applies(player) && creature->HasNpcFlag(UNIT_NPC_FLAG_TRAINER)
            && creature->GetScriptName() != "npc_spellbook_trainer")
        {
            auto const* trainer = Trainer::GetTrainerFor(creature, player);
            if (trainer && trainer->GetTrainerType() == Trainer::Type::Class)
                Train(player, creature);
        }
        return false;
    }
};

class ActivePlayer final : public PlayerScript
{
public:
    ActivePlayer() : PlayerScript("Area52ActiveBuildPlayer",
        { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LOGOUT, PLAYERHOOK_ON_LEVEL_CHANGED,
          PLAYERHOOK_ON_UPDATE, PLAYERHOOK_ON_OPEN_TRAINER }) { }
    void OnPlayerOpenTrainer(Player* player, Creature* trainer) override
    {
        auto const* definition = Trainer::GetTrainerFor(trainer, player);
        if (definition && definition->GetTrainerType() == Trainer::Type::Class
            && trainer->GetScriptName() != "npc_spellbook_trainer")
            Train(player, trainer);
    }
    void OnPlayerLogin(Player* player) override
    {
        if (!Applies(player))
            return;
        {
            std::lock_guard lock(InboxLock);
            Inboxes[player->GetSession()].clear();
        }
        auto* state = player->CustomData.GetDefault<State>(StateKey);
        std::uint64_t const id = std::uint64_t(player->GetPlayerSetting(Setting, 0).value)
            | (std::uint64_t(player->GetPlayerSetting(Setting, 1).value) << 32);
        if (id)
            Fetch(player, id, player->GetPlayerSetting(Setting, 2).value,
                player->GetPlayerSetting(Setting, 3).value != 0, true);
        else
        {
            state->Loaded = true;
            SendState(player, *state);
        }
    }
    void OnPlayerLogout(Player* player) override
    {
        std::lock_guard lock(InboxLock);
        Inboxes.erase(player->GetSession());
    }
    void OnPlayerLevelChanged(Player* player, std::uint8_t) override
    {
        if (auto* state = player->CustomData.Get<State>(StateKey); Applies(player) && state && state->Automatic)
            state->Pending = true;
    }
    void OnPlayerUpdate(Player* player, std::uint32_t) override
    {
        UpdateArea52DraftPet(player);
        auto* state = player->CustomData.Get<State>(StateKey);
        if (!Applies(player) || !state || state->Busy || Clock::now() < state->NextRequest)
            return;
        if (state->Pending && state->Loaded && Safe(player))
        {
            if (state->Draft && !AscensionMysticEnchant::DraftEquipmentReady(player))
                return;
            state->Pending = false;
            if (Learn(player, *state, 80, state->Automatic))
            {
                if (state->Automatic)
                    AscensionFreePick::LearnAvailableRanks(player);
                if (state->Draft && !player->GetPlayerSetting("core.area52.draft_progress", 2).value
                    && !AscensionMysticEnchant::ApplyDraftEnchants(player, state->Plan.Enchants))
                {
                    state->Pending = true;
                    state->NextRequest = Clock::now() + std::chrono::seconds(2);
                    return;
                }
            }
            else if (state->Draft)
            {
                state->Pending = true;
                state->NextRequest = Clock::now() + std::chrono::seconds(2);
                return;
            }
        }
        DeliverFeaturedRewards(player, *state);
        if (state->Busy)
            return;
        if (state->Draft && state->Loaded && !state->Pending)
        {
            std::uint64_t build = 0;
            ParseId(state->Plan.Id, build);
            DraftLifecycle progress{build, state->Revision,
                player->GetPlayerSetting("core.area52.draft_progress", 0).value != 0,
                player->GetPlayerSetting("core.area52.draft_progress", 1).value != 0,
                player->GetPlayerSetting("core.area52.draft_progress", 2).value != 0};
            if (DraftRewardDue(progress, player->GetLevel(), sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
                && AscensionMysticEnchant::GrantDraftReward(player, build, state->Revision, state->Plan.Enchants))
                state->NextRequest = Clock::now() + std::chrono::seconds(2);
        }
        WorldPacket packet;
        {
            std::lock_guard lock(InboxLock);
            auto it = Inboxes.find(player->GetSession());
            if (it == Inboxes.end() || it->second.empty())
                return;
            packet = std::move(it->second.front());
            it->second.pop_front();
        }
        state->NextRequest = Clock::now() + std::chrono::seconds(2);
        Reader input(std::span(packet.contents(), packet.size()));
        std::string id;
        std::uint8_t automatic = 0, secondary = 0;
        bool const activate = packet.GetOpcode() == 0x5C2;
        std::uint64_t numericId = 0;
        if (!input.Text(id, 64) || !ParseId(id, numericId)
            || (activate && (!input.Number(automatic) || !input.Number(secondary) || automatic > 1 || secondary > 1))
            || !input.Finished() || !Safe(player))
        {
            Reply(player, activate, "ACTIVATE_BUILD_NOT_AVAILABLE");
            return;
        }
        if (activate && id == state->Plan.Id && state->Draft && player->GetLevel() < 10)
        {
            Fetch(player, numericId, 0, true, false);
            return;
        }
        if (id == state->Plan.Id)
        {
            if (!activate && state->Draft && !BeforeManualDraftChange(player))
            {
                Reply(player, false, "ACTIVATE_BUILD_NOT_AVAILABLE");
                return;
            }
            state->Automatic = activate && (state->Draft || automatic);
            state->Pending = activate;
            if (!activate)
            {
                state->Plan = {};
                state->Revision = 0;
            }
            Store(player, *state);
            player->SaveToDB(false, false);
            SendState(player, *state);
            Reply(player, activate, activate ? "ACTIVATE_BUILD_OK" : "DEACTIVATE_BUILD_OK");
        }
        else if (activate)
            Fetch(player, numericId, 0, automatic != 0, false);
        else
            Reply(player, false, "DEACTIVATE_BUILD_NOT_FOUND");
    }
};
}

std::uint32_t Train(Player* player, Creature* trainer, std::uint32_t maximumLevel)
{
    if (!Applies(player) || !trainer || !trainer->IsAlive() || !player->IsWithinDistInMap(trainer, 10.0f)
        || !player->HaveAtClient(trainer))
        return 0;
    auto* state = player->CustomData.Get<State>(StateKey);
    if (!state || !state->Loaded || state->Busy || state->Plan.Id.empty() || Clock::now() < state->NextRequest)
        return 0;
    state->NextRequest = Clock::now() + std::chrono::seconds(2);
    auto const before = AscensionFreePick::KnownEntries(player);
    if (!Learn(player, *state, maximumLevel))
        return 0;
    return AscensionFreePick::SameSelection(before, AscensionFreePick::KnownEntries(player)) ? 0 : 1;
}

bool CanSwitchSpecialization(Player* player)
{
    auto const* state = player->CustomData.Get<State>(StateKey);
    return !Applies(player) || (state && state->Loaded && !state->Busy);
}

void RestoreSpecialization(Player* player)
{
    if (!Applies(player))
        return;
    {
        std::lock_guard lock(InboxLock);
        Inboxes[player->GetSession()].clear();
    }
    auto* state = player->CustomData.GetDefault<State>(StateKey);
    *state = State{};
    std::uint64_t const id = std::uint64_t(player->GetPlayerSetting(Setting, 0).value)
        | (std::uint64_t(player->GetPlayerSetting(Setting, 1).value) << 32);
    if (id)
        Fetch(player, id, player->GetPlayerSetting(Setting, 2).value,
            player->GetPlayerSetting(Setting, 3).value != 0, true);
    else
    {
        state->Loaded = true;
        SendState(player, *state);
    }
}

std::optional<std::uint32_t> GetReadyFeaturedArchetype(Player* player)
{
    if (!Applies(player))
        return 0;
    auto const* state = player->CustomData.Get<State>(StateKey);
    if (!state || !state->Loaded || state->Busy || state->Pending)
        return std::nullopt;
    return state->Draft ? DraftArchetype(state->Plan) : 0;
}

CacheBuildContext GetCacheBuildContext(Player* player)
{
    if (!Applies(player))
        return {};
    auto const* state = player->CustomData.Get<State>(StateKey);
    if (!state || !state->Loaded || state->Busy || state->Pending)
        return {};
    if (state->Plan.Id.empty())
        return { true, {}, 0, 0, 0 };
    return { true, state->Plan.Id, state->Revision, state->Plan.Roles, state->Plan.PrimaryStat };
}

DraftVanityContext GetDraftVanityContext(Player* player)
{
    if (!Applies(player))
        return { 0, 0, 0, true };
    auto const* state = player ? player->CustomData.Get<State>(StateKey) : nullptr;
    if (!state || !state->Loaded || state->Busy || state->Pending)
        return {};
    if (!state->Draft || player->GetPlayerSetting("core.area52.draft_progress", 2).value)
        return { 0, 0, 0, true };
    std::uint64_t build = 0;
    if (!ParseId(state->Plan.Id, build))
        return {};
    DraftVanityContext context{ build, state->Revision, DraftArchetype(state->Plan), true };
    FeaturedPreviewEffects preview{};
    if (!ReadFeaturedIncarnations(state->Plan.Description, context.Incarnations)
        || !ReadFeaturedPreview(state->Plan.Description, preview) || !ValidFeaturedPet(preview[0]))
        return {};
    context.Pet = preview[0];
    return context;
}

bool IsFeaturedDraft(Player* player)
{
    auto const* state = player ? player->CustomData.Get<State>(StateKey) : nullptr;
    return Applies(player) && state && state->Draft;
}

bool BeforeManualDraftChange(Player* player)
{
    if (!Applies(player))
        return true;
    auto* state = player->CustomData.Get<State>(StateKey);
    if (!state || state->InternalSelection || !state->Draft)
        return true;
    std::uint64_t build = 0;
    if (state->Busy || !Safe(player) || !ParseId(state->Plan.Id, build) ||
        !AscensionMysticEnchant::ReleaseDraftEnchants(player, build, state->Revision))
        return false;
    player->UpdatePlayerSetting("core.area52.draft_progress", 0, 1);
    player->UpdatePlayerSetting("core.area52.draft_progress", 2, 1);
    for (std::uint32_t spec = 0; spec < AscensionFreePick::SpecializationSpells.size(); ++spec)
    {
        auto const key = "core.area52.spec." + std::to_string(spec) + ".active_build";
        if (player->GetPlayerSetting(key, 4).value)
            for (std::uint32_t index = 0; index < 5; ++index)
                player->UpdatePlayerSetting(key, index, 0);
    }
    state->Draft = false;
    state->Automatic = false;
    state->Pending = false;
    state->Plan = {};
    state->Revision = 0;
    Store(player, *state);
    player->SaveToDB(false, false);
    SendState(player, *state);
    return true;
}

void PauseAutoLearnForReset(Player* player)
{
    if (!Applies(player))
        return;
    auto* state = player->CustomData.Get<State>(StateKey);
    if (!state)
        return;
    state->Generation = NextGeneration.fetch_add(1);
    state->Automatic = false;
    state->Pending = false;
    state->Busy = false;
    state->Loaded = true;
    Store(player, *state);
    SendState(player, *state);
}
}

#include "Area52DraftPet.inc"

void AddArea52ActiveBuildScripts()
{
    RegisterAscensionClientConfig([](AscensionClientConfig& config)
        { config.Booleans.emplace_back("CONFIG_AREA52_ACTIVE_BUILDS_ENABLED", Area52Build::Enabled.load()); });
    AscensionCompatOpcodes::Claim(0x5C2, &Area52Build::Handle);
    AscensionCompatOpcodes::Claim(0x5C4, &Area52Build::Handle);
    new Area52Build::ActiveWorld();
    new Area52Build::ActivePlayer();
    new Area52Build::ActiveTrainer();
}
