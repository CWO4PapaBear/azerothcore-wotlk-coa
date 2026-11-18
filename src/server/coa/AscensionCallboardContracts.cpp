/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Area52MysticRealm.h"
#include "AscensionCallboardContractsPolicy.h"
#include "CallboardRealmTier.h"
#include "CallboardHuntRotation.h"
#include "Timer.h"
#include <chrono>
#include "AscensionCallboardContracts.h"
#include "Mail.h"
#include "AscensionCompatOpcodes.h"
#include "Chat.h"
#include "Config.h"
#include "DBCStores.h"
#include "DisableMgr.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "LocalLevelScaling.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerScript.h"
#include "QuestDef.h"
#include "ScriptMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldScript.h"
#include "WorldSession.h"
#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
using namespace CallboardContracts;

constexpr char LedgerKey[] = "ascension_callboard_contract_ledger";
constexpr uint32 PruneIntervalMs = 1000;

struct ContractType
{
    std::string type;
    std::string contentType;
    std::vector<uint32> quests;
    uint32 minimumLevel = 0;
};

struct Catalogue
{
    bool ready = false;
    std::vector<ContractType> types;
    std::unordered_map<uint32, std::size_t> typeByQuest;
    std::unordered_map<std::string, std::size_t> typeByName;
    std::unordered_map<uint32, uint32> cachePoints;

    [[nodiscard]] ContractType const* ForQuest(uint32 questId) const
    {
        auto const found = typeByQuest.find(questId);
        return found == typeByQuest.end() ? nullptr : &types[found->second];
    }

    [[nodiscard]] ContractType const* ForName(std::string const& name) const
    {
        auto const found = typeByName.find(name);
        return found == typeByName.end() ? nullptr : &types[found->second];
    }
};

struct Settings
{
    uint32 dailyCompletions = 1;
    double moneyCostMultiplier = 2.0;
    uint32 minMoneyCost = 100000;
    double tokenCostMultiplier = 2.0;
    uint32 minTokenCost = 50;
    PaymentOptions payment;
};

struct LedgerData final : DataMap::Base
{
    WeekLedger ledger;
    bool loaded = false;
};

std::atomic<bool> enabled{false};
std::mutex stateMutex;
std::shared_ptr<Catalogue const> catalogue = std::make_shared<Catalogue const>();
Settings settings;
PendingRequests pendingRequests;
thread_local bool activating = false;
thread_local CharacterDatabaseTransaction rewardTransaction;

std::shared_ptr<Catalogue const> CurrentCatalogue()
{
    std::lock_guard lock(stateMutex);
    return catalogue;
}

Settings CurrentSettings()
{
    std::lock_guard lock(stateMutex);
    return settings;
}

void LoadSettings()
{
    auto const positive = [](char const* key, uint32 fallback)
    {
        return std::max<uint32>(1, sConfigMgr->GetOption<uint32>(key, fallback));
    };

    Settings loaded;
    loaded.dailyCompletions = positive("CoA.CallboardContracts.DailyCompletions", 1);
    loaded.moneyCostMultiplier = sConfigMgr->GetOption<float>("CoA.CallboardContracts.MoneyCostMultiplier", 2.0f);
    loaded.minMoneyCost = positive("CoA.CallboardContracts.MinMoneyCost", 100000);
    loaded.tokenCostMultiplier = sConfigMgr->GetOption<float>("CoA.CallboardContracts.TokenCostMultiplier", 2.0f);
    loaded.minTokenCost = positive("CoA.CallboardContracts.MinTokenCost", 50);
    loaded.payment.gold = sConfigMgr->GetOption<bool>("CoA.CallboardContracts.GoldEnable", true);
    loaded.payment.tokens = sConfigMgr->GetOption<bool>("CoA.CallboardContracts.TokenEnable", true);
    {
        std::lock_guard lock(stateMutex);
        settings = loaded;
    }
    enabled.store(sConfigMgr->GetOption<bool>("CoA.CallboardContracts.Enable", true), std::memory_order_relaxed);
}

void LoadCatalogue()
{
    auto loaded = std::make_shared<Catalogue>();
    uint32 skipped = 0;
    if (QueryResult result = WorldDatabase.Query("SELECT c.`QuestId`, c.`ContractType`, c.`ContentType`, "
        "COALESCE(p.`Points`, 0) FROM `ascension_callboard_contract` c LEFT JOIN `ascension_callboard_quest_points` p "
        "ON p.`QuestId` = c.`QuestId` UNION ALL SELECT 0, '', '', 0 ORDER BY 2, 1"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 const questId = fields[0].Get<uint32>();
            std::string const type = fields[1].Get<std::string>();
            std::string const contentType = fields[2].Get<std::string>();
            Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
            if (!quest || !IsValidTypeName(type) || !IsContentType(contentType) || loaded->typeByQuest.contains(questId))
            {
                ++skipped;
                continue;
            }

            auto [found, inserted] = loaded->typeByName.try_emplace(type, loaded->types.size());
            if (inserted)
                loaded->types.push_back({ type, contentType, {}, quest->GetMinLevel() });

            ContractType& contract = loaded->types[found->second];
            if (contract.contentType != contentType)
            {
                ++skipped;
                continue;
            }

            contract.quests.push_back(questId);
            contract.minimumLevel = std::min(contract.minimumLevel, quest->GetMinLevel());
            loaded->typeByQuest.emplace(questId, found->second);
            if (uint32 const points = fields[3].Get<uint32>())
                loaded->cachePoints.emplace(questId, points);
        } while (result->NextRow());
    }

    else
    {
        LOG_ERROR("server.loading", "Call Board contract catalogue read failed; retaining the previous catalogue");
        return;
    }

    LOG_INFO("server.loading", ">> Loaded {} Call Board contract types ({} quests, {} rows skipped)",
        loaded->types.size(), loaded->typeByQuest.size(), skipped);

    loaded->ready = true;
    std::lock_guard lock(stateMutex);
    catalogue = std::move(loaded);
}

bool IsBot(Player const* player)
{
    return !player->GetSession() || player->GetSession()->IsBot();
}

int64 CurrentWeekStart()
{
    return WeekStart(sWorld->GetNextWeeklyQuestsResetTime().count());
}

uint32 CurrentDaysBegun()
{
    return DaysBegun(sWorld->GetNextDailyQuestsResetTime().count(), sWorld->GetNextWeeklyQuestsResetTime().count());
}

Tally ThisWeek(Player const* player, ContractType const& contract)
{
    LedgerData const* data = player->CustomData.Get<LedgerData>(LedgerKey);
    return data ? data->ledger.For(contract.type, CurrentWeekStart()) : Tally{};
}

uint32 CompletedThisWeek(Player const* player, ContractType const& contract)
{
    return ThisWeek(player, contract).completions;
}

uint32 PurchasedThisWeek(Player const* player, ContractType const& contract)
{
    return ThisWeek(player, contract).purchases;
}

bool LedgerLoaded(Player const* player)
{
    LedgerData const* data = player->CustomData.Get<LedgerData>(LedgerKey);
    return data && data->loaded;
}

uint32 RemainingFor(Player const* player, ContractType const& contract, Settings const& current)
{
    if (!LedgerLoaded(player))
        return 0;

    return Remaining(CurrentDaysBegun(), current.dailyCompletions, CompletedThisWeek(player, contract));
}

uint32 InLog(Player const* player, Catalogue const& contracts, ContractType const& contract)
{
    uint32 count = 0;
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        if (uint32 const questId = player->GetQuestSlotQuestId(slot))
            if (contracts.ForQuest(questId) == &contract)
                ++count;
    return count;
}

uint32 SpareFor(Player const* player, Catalogue const& contracts, ContractType const& contract,
    Settings const& current)
{
    return Spare(RemainingFor(player, contract, current), InLog(player, contracts, contract));
}

bool ContractAvailable(Player const* player, std::string const& type)
{
    auto const cap = CallboardRealmTier::ForPlayer(player);
    if (!CallboardRealmTier::Available(type, cap)) return false;
    int const difficulty = RequiredDungeonDifficulty(type);
    if (difficulty < 0) return true;
    for (uint32 index = 0; index < sMapStore.GetNumRows(); ++index)
    {
        auto const* map = sMapStore.LookupEntry(index);
        if (map && map->IsDungeon() && !map->IsRaid() && CallboardRealmTier::DungeonMatches(cap, map->Expansion())
            && sObjectMgr->GetInstanceTemplate(map->MapID)
            && GetMapDifficultyData(map->MapID, Difficulty(difficulty))
            && !DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, map->MapID, nullptr, uint8(difficulty)))
            return true;
    }
    return false;
}

Quest const* ChooseQuest(Player* player, ContractType const& contract)
{
    if (!ContractAvailable(player, contract.type))
        return nullptr;

    for (uint32 const questId : contract.quests)
        if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
            if (CallboardHunts::CanOffer(player, questId)
                && !DisableMgr::IsDisabledFor(DISABLE_TYPE_QUEST, questId, player)
                && player->SatisfyQuestClass(quest, false) && player->SatisfyQuestRace(quest, false)
                && player->SatisfyQuestLevel(quest, false) && player->SatisfyQuestSkill(quest, false)
                && player->SatisfyQuestReputation(quest, false) && player->SatisfyQuestPreviousQuest(quest, false)
                && player->SatisfyQuestConditions(quest, false))
                return quest;
    return nullptr;
}

uint32 RewardMoney(Player const* player, Quest const* quest)
{
    bool const scaling = LocalLevelScaling::QuestScalingEnabled(player);
    int64 money = std::max<int32>(0, quest->GetRewOrReqMoney(player->GetLevel(), scaling));
    if (player->GetLevel() >= sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        money += quest->GetRewMoneyMaxLevel(scaling);
    return Saturate(uint64(money));
}

Reward RewardFor(Player const* player, Catalogue const& contracts, Quest const* quest)
{
    Reward reward;
    reward.money = RewardMoney(player, quest);
    reward.honor = quest->CalculateHonorGain(player->GetLevel());
    reward.arenaPoints = uint32(std::max<int32>(0, quest->GetRewArenaPoints()));
    if (auto const points = contracts.cachePoints.find(quest->GetQuestId()); points != contracts.cachePoints.end())
        reward.cachePoints = DisplayedCachePoints(points->second);

    for (uint32 i = 0; i < QUEST_REWARDS_COUNT; ++i)
        if (quest->RewardItemId[i] && quest->RewardItemIdCount[i])
            if (uint32 const itemId = quest->RewardItemId[i])
                reward.items.push_back({ itemId, quest->RewardItemIdCount[i] });
    return reward;
}

uint32 NextMoneyCost(Reward const& reward, Settings const& current, uint32 purchased)
{
    uint32 const base = MoneyCost(reward.money, current.moneyCostMultiplier, current.minMoneyCost);
    return EscalatedMoneyCost(base, current.moneyCostMultiplier, purchased);
}

uint32 NextTokenCost(Reward const& reward, Settings const& current, uint32 purchased)
{
    uint32 const base = TokenCost(reward.ItemAmount(BazaarToken), current.tokenCostMultiplier, current.minTokenCost);
    return EscalatedTokenCost(base, current.tokenCostMultiplier, purchased);
}

uint32 PriceFor(Player const* player, Catalogue const& contracts, ContractType const& contract, Quest const* quest,
    Settings const& current, bool payWithGold)
{
    Reward const reward = RewardFor(player, contracts, quest);
    uint32 const purchased = Area52MysticRealm::UsesFreePickEngine() ? 0 : PurchasedThisWeek(player, contract);
    return payWithGold ? NextMoneyCost(reward, current, purchased) : NextTokenCost(reward, current, purchased);
}

void SendView(Player* player)
{
    std::shared_ptr<Catalogue const> const contracts = CurrentCatalogue();
    Settings const current = CurrentSettings();
    std::vector<Category> categories;
    for (auto const& [key, name] : ContentTypes)
    {
        Category category{ std::string(key), std::string(name), {} };
        for (ContractType const& contract : contracts->types)
        {
            if (contract.contentType != key || !CallboardRealmTier::Available(contract.type, CallboardRealmTier::ForPlayer(player)))
                continue;

            SubCategory sub;
            sub.type = contract.type;
            sub.name = DisplayName(contract.type);
            sub.minimumLevel = contract.minimumLevel;
            if (Quest const* quest = ChooseQuest(player, contract))
            {
                sub.remaining = SpareFor(player, *contracts, contract, current);
                sub.reward = RewardFor(player, *contracts, quest);
                uint32 const purchased = Area52MysticRealm::UsesFreePickEngine() ? 0 : PurchasedThisWeek(player, contract);
                sub.moneyCost = NextMoneyCost(sub.reward, current, purchased);
                sub.tokenCost = NextTokenCost(sub.reward, current, purchased);
            }
            else
            {
                sub.moneyCost = current.minMoneyCost;
                sub.tokenCost = current.minTokenCost;
            }
            category.subCategories.push_back(std::move(sub));
        }
        categories.push_back(std::move(category));
    }

    std::vector<uint8> const bytes = SerializeView(categories, current.payment);
    WorldPacket packet(ViewOpcode, bytes.size());
    packet.append(bytes.data(), bytes.size());
    player->GetSession()->SendPacket(&packet);
}

void SaveTally(ObjectGuid::LowType guid, std::string const& type, Tally const& tally, int64 weekStart)
{
    CharacterDatabasePreparedStatement* statement =
        CharacterDatabase.GetPreparedStatement(CHAR_REP_COA_CALLBOARD_CONTRACT);
    statement->SetData(0, guid);
    statement->SetData(1, type);
    statement->SetData(2, uint32(weekStart));
    statement->SetData(3, tally.completions);
    statement->SetData(4, tally.purchases);
    CharacterDatabase.Execute(statement);
}

void CountCompletion(Player* player, ContractType const& contract, bool purchased)
{
    LedgerData* data = player->CustomData.GetDefault<LedgerData>(LedgerKey);
    int64 const weekStart = CurrentWeekStart();
    data->ledger.Roll(weekStart);
    Tally const tally = data->ledger.Add(contract.type, purchased);
    if (data->loaded && !Area52MysticRealm::UsesFreePickEngine())
        SaveTally(player->GetGUID().GetCounter(), contract.type, tally, weekStart);
}

void ApplyStoredLedger(Player* player, PreparedQueryResult result, int64 weekStart)
{
    LedgerData* data = player->CustomData.GetDefault<LedgerData>(LedgerKey);
    if (data->loaded || !result)
        return;

    WeekLedger stored;
    stored.Roll(weekStart);
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            if (!fields[0].Get<std::string>().empty())
                stored.Set(fields[0].Get<std::string>(), fields[1].Get<uint32>(), fields[2].Get<uint32>());
        } while (result->NextRow());
    }

    auto const unsaved = data->ledger.Types();
    data->ledger.Merge(stored);
    data->loaded = true;

    int64 const ledgerWeek = data->ledger.WeekStartTime();
    if (!Area52MysticRealm::UsesFreePickEngine())
        for (std::string const& type : unsaved)
            SaveTally(player->GetGUID().GetCounter(), type, data->ledger.For(type, ledgerWeek), ledgerWeek);
}

void LoadLedger(Player* player)
{
    int64 const weekStart = CurrentWeekStart();
    ObjectGuid const owner = player->GetGUID();
    player->CustomData.GetDefault<LedgerData>(LedgerKey)->ledger.Roll(weekStart);

    CharacterDatabasePreparedStatement* cleanup =
        CharacterDatabase.GetPreparedStatement(CHAR_DEL_COA_CALLBOARD_CONTRACT_OLD_WEEKS);
    cleanup->SetData(0, owner.GetCounter());
    cleanup->SetData(1, uint32(weekStart));
    CharacterDatabase.Execute(cleanup);

    CharacterDatabasePreparedStatement* select =
        CharacterDatabase.GetPreparedStatement(CHAR_SEL_COA_CALLBOARD_CONTRACTS);
    select->SetData(0, owner.GetCounter());
    select->SetData(1, uint32(weekStart));
    WorldSession* session = player->GetSession();
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(select)
        .WithPreparedCallback([session, owner, weekStart](PreparedQueryResult result)
        {
            Player* loaded = session->GetPlayer();
            if (loaded && loaded->GetGUID() == owner)
                ApplyStoredLedger(loaded, std::move(result), weekStart);
        }));
}

using RewardItems = std::vector<std::unique_ptr<Item>>;

bool PrepareRewardItems(Player* player, Quest const* quest, RewardItems& items)
{
    auto prepare = [&](uint32 rawItemId, uint32 remaining)
    {
        uint32 const itemId = rawItemId;
        auto const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate || !remaining) return false;
        while (remaining)
        {
            uint32 const count = std::min(remaining, itemTemplate->GetMaxStackSize());
            if (!count) return false;
            std::unique_ptr<Item> item(Item::CreateItem(itemId, count, player));
            if (!item) return false;
            items.push_back(std::move(item));
            remaining -= count;
        }
        return true;
    };
    for (uint32 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        if (quest->RewardChoiceItemId[i])
        {
            if (!prepare(quest->RewardChoiceItemId[i], quest->RewardChoiceItemCount[i])) return false;
            break;
        }
    for (uint32 i = 0; i < QUEST_REWARDS_COUNT; ++i)
        if (quest->RewardItemId[i] && !prepare(quest->RewardItemId[i], quest->RewardItemIdCount[i])) return false;
    return true;
}

void GrantRewards(Player* player, Quest const* quest, RewardItems& items)
{
    for (auto& prepared : items)
    {
        ItemPosCountVec dest;
        uint32 const count = prepared->GetCount();
        if (player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, prepared.get(), false) == EQUIP_ERR_OK)
        {
            Item* item = player->StoreItem(dest, prepared.release(), true);
            player->SendNewItem(item, count, true, false, false, false);
            sScriptMgr->OnPlayerQuestRewardItem(player, item, count);
        }
        else
        {
            prepared->SaveToDB(rewardTransaction);
            MailDraft draft("Temporal Contract Reward", "Your completed contract reward is enclosed.");
            draft.AddItem(prepared.release());
            draft.SendMailTo(rewardTransaction, MailReceiver(player, player->GetGUID().GetCounter()),
                MailSender(MAIL_CREATURE, 34337));
        }
    }

    player->RewardReputation(quest);

    bool const scaling = LocalLevelScaling::QuestScalingEnabled(player);
    uint32 xp = player->CalculateQuestRewardXP(quest);
    sScriptMgr->OnPlayerQuestComputeXP(player, quest, xp);
    int32 money = 0;
    if (player->GetLevel() >= sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)
        || sScriptMgr->OnPlayerShouldBeRewardedWithMoneyInsteadOfExp(player))
        money = quest->GetRewMoneyMaxLevel(scaling);
    else
    {
        sScriptMgr->OnPlayerGiveXP(player, xp, nullptr, PlayerXPSource::XPSOURCE_QUEST);
        player->GiveXP(xp, nullptr);
    }

    money += std::max<int32>(0, quest->GetRewOrReqMoney(player->GetLevel(), scaling));
    if (player->HasPlayerFlag(PLAYER_FLAGS_NO_PLAY_TIME))
        money = 0;
    else if (player->HasPlayerFlag(PLAYER_FLAGS_PARTIAL_PLAY_TIME))
        money /= 2;

    if (money > 0)
    {
        player->ModifyMoney(money);
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD, uint32(money));
    }

    if (uint32 const honor = quest->CalculateHonorGain(player->GetLevel()))
        player->RewardHonor(nullptr, 0, honor);

    if (quest->GetRewArenaPoints())
        player->ModifyArenaPoints(quest->GetRewArenaPoints());

    if (quest->IsDaily())
    {
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST, quest->GetQuestId());
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY, quest->GetQuestId());
    }
    player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST, quest->GetQuestId());

    sScriptMgr->OnPlayerCompleteQuest(player, quest);
}

bool Charge(Player* player, uint32 price, bool payWithGold)
{
    if (payWithGold)
        return player->HasEnoughMoney(price) && player->ModifyMoney(-int32(price));

    if (!player->HasItemCount(BazaarToken, price, false))
        return false;

    player->DestroyItemCount(BazaarToken, price, true);
    return true;
}

uint32 Activate(Player* player, Catalogue const& contracts, Settings const& current, Activation const& request)
{
    ChatHandler chat(player->GetSession());
    ContractType const* contract = contracts.ForName(request.type);
    if (!contract)
        return 0;

    std::string const name = DisplayName(contract->type);
    if (!current.payment.Allows(request.payWithGold))
    {
        chat.SendSysMessage(request.payWithGold ? "Temporal Contracts cannot be paid with gold."
            : "Temporal Contracts cannot be paid with Bazaar Tokens.");
        return 0;
    }

    Quest const* quest = ChooseQuest(player, *contract);
    if (!quest)
    {
        chat.PSendSysMessage("You cannot complete {} contracts yet.", name);
        return 0;
    }

    uint32 const spare = SpareFor(player, contracts, *contract, current);
    if (!spare)
    {
        chat.PSendSysMessage("No {} completions remain. Finish or abandon the quest in your log first.", name);
        return 0;
    }

    uint32 completed = 0;
    uint32 const wanted = std::min(request.Completions(), spare);
    activating = true;
    for (; completed < wanted; ++completed)
    {
        if (ChooseQuest(player, *contract) != quest || !SpareFor(player, contracts, *contract, current))
            break;
        RewardItems items;
        if (!PrepareRewardItems(player, quest, items))
        {
            chat.PSendSysMessage("Unable to prepare {} rewards. No payment was taken.", name);
            break;
        }
        uint32 const price = PriceFor(player, contracts, *contract, quest, current, request.payWithGold);
        if (!Charge(player, price, request.payWithGold))
        {
            chat.PSendSysMessage(request.payWithGold ? "You do not have enough gold to complete {}."
                : "You do not have enough Bazaar Tokens to complete {}.", name);
            break;
        }
        GrantRewards(player, quest, items);
    }
    activating = false;

    if (completed)
        chat.PSendSysMessage("Temporal Contract complete: {} x{}.", name, completed);
    return completed;
}

void ProcessRequests(Player* player)
{
    if (player->IsBeingTeleportedFar())
        return;
    WorldSession* session = player->GetSession();
    if (!session)
        return;

    bool const active = enabled.load(std::memory_order_relaxed) && !IsBot(player);
    if (active && !LedgerLoaded(player))
        return;

    Requests requests = pendingRequests.Take(session->GetAccountId(), PendingRequests::Clock::now());
    if (!active)
        return;

    bool send = requests.view;
    if (!requests.activations.empty())
    {
        std::shared_ptr<Catalogue const> const contracts = CurrentCatalogue();
        Settings const current = CurrentSettings();
        rewardTransaction = CharacterDatabase.BeginTransaction();
        uint32 completed = 0;
        for (Activation const& activation : requests.activations)
            completed += Activate(player, *contracts, current, activation);

        if (completed)
        {
            player->SaveToDB(rewardTransaction, false, false);
            CharacterDatabase.CommitTransaction(rewardTransaction);
        }
        rewardTransaction.reset();
        send = true;
    }

    if (send)
        SendView(player);
}

bool AcceptsRequests(WorldSession* session)
{
    return session && session->GetPlayer() && enabled.load(std::memory_order_relaxed);
}

bool HandleRequestView(WorldSession* session, WorldPacket const&)
{
    if (AcceptsRequests(session))
        pendingRequests.RequestView(session->GetAccountId(), PendingRequests::Clock::now());
    return true;
}

bool HandleActivate(WorldSession* session, WorldPacket const& packet)
{
    if (!AcceptsRequests(session))
        return true;

    if (std::optional<Activation> activation = ParseActivationPacket(packet))
        pendingRequests.Activate(session->GetAccountId(), std::move(*activation), PendingRequests::Clock::now());
    return true;
}

std::optional<std::string> ExhaustedContractName(Player const* player, Quest const* quest)
{
    if (!enabled.load(std::memory_order_relaxed) || IsBot(player))
        return std::nullopt;

    std::shared_ptr<Catalogue const> const contracts = CurrentCatalogue();
    ContractType const* contract = contracts->ForQuest(quest->GetQuestId());
    if (!contract || (ContractAvailable(player, contract->type)
        && RemainingFor(player, *contract, CurrentSettings()) > 0))
        return std::nullopt;

    return DisplayName(contract->type);
}

class ascension_callboard_contracts_player : public PlayerScript
{
public:
    ascension_callboard_contracts_player()
        : PlayerScript("ascension_callboard_contracts_player",
              { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LOGOUT, PLAYERHOOK_ON_UPDATE, PLAYERHOOK_CAN_TAKE_QUEST,
                PLAYERHOOK_CAN_REWARD_QUEST, PLAYERHOOK_ON_QUEST_GIVER_CHOOSE_REWARD,
                PLAYERHOOK_ON_PLAYER_COMPLETE_QUEST, PLAYERHOOK_ON_DELETE_FROM_DB }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!IsBot(player))
            LoadLedger(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        if (WorldSession* session = player->GetSession())
            static_cast<void>(pendingRequests.Take(session->GetAccountId(), PendingRequests::Clock::now()));
    }

    void OnPlayerUpdate(Player* player, uint32) override
    {
        if (pendingRequests.Size())
            ProcessRequests(player);
    }

    [[nodiscard]] bool OnPlayerCanTakeQuest(Player const* player, Quest const* quest) override
    {
        if (!CallboardRealmTier::CanUse(player, quest)) return false;
        if (!CallboardHunts::CanOffer(player, quest->GetQuestId())) return false;
        if (!enabled.load(std::memory_order_relaxed) || IsBot(player))
            return true;

        std::shared_ptr<Catalogue const> const contracts = CurrentCatalogue();
        ContractType const* contract = contracts->ForQuest(quest->GetQuestId());
        return !contract || (ContractAvailable(player, contract->type)
            && SpareFor(player, *contracts, *contract, CurrentSettings()) > 0);
    }

    [[nodiscard]] bool OnPlayerCanRewardQuest(Player const* player, Quest const* quest) override
    {
        return CallboardRealmTier::CanUse(player, quest) && !ExhaustedContractName(player, quest);
    }

    [[nodiscard]] bool OnPlayerQuestGiverChooseReward(Player* player, Object*, Quest const* quest, uint32) override
    {
        if (!CallboardRealmTier::CanUse(player, quest))
        {
            ChatHandler(player->GetSession()).SendSysMessage("This Callboard quest is unavailable at your current Callboard tier.");
            return true;
        }
        std::optional<std::string> const exhausted = ExhaustedContractName(player, quest);
        if (!exhausted)
            return false;

        ChatHandler(player->GetSession()).PSendSysMessage("No {} completions remain this week.", *exhausted);
        return true;
    }

    void OnPlayerCompleteQuest(Player* player, Quest const* quest) override
    {
        if (!enabled.load(std::memory_order_relaxed) || IsBot(player))
            return;

        std::shared_ptr<Catalogue const> const contracts = CurrentCatalogue();
        ContractType const* contract = contracts->ForQuest(quest->GetQuestId());
        if (!contract)
            return;

        if (activating || !Area52MysticRealm::UsesFreePickEngine())
            CountCompletion(player, *contract, activating);
        if (!activating)
            SendView(player);
    }

    void OnPlayerDeleteFromDB(CharacterDatabaseTransaction trans, uint32 guid) override
    {
        CharacterDatabasePreparedStatement* statement =
            CharacterDatabase.GetPreparedStatement(CHAR_DEL_COA_CALLBOARD_CONTRACTS);
        statement->SetData(0, guid);
        trans->Append(statement);
    }
};

class ascension_callboard_contracts_world : public WorldScript
{
public:
    ascension_callboard_contracts_world()
        : WorldScript("ascension_callboard_contracts_world",
              { WORLDHOOK_ON_STARTUP, WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_UPDATE })
    { }

    void OnStartup() override
    {
        LoadCatalogue();
    }

    void OnAfterConfigLoad(bool reload) override
    {
        LoadSettings();
        if (reload)
            LoadCatalogue();
    }

    void OnUpdate(uint32 diff) override
    {
        _sincePrune += diff;
        if (_sincePrune < PruneIntervalMs)
            return;

        _sincePrune = 0;
        pendingRequests.DropExpired(PendingRequests::Clock::now());
    }

private:
    uint32 _sincePrune = 0;
};
}

unsigned CallboardRealmTier::ForPlayer(Player const* player)
{
    return player ? PlayerTier(player->GetLevel(), sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)) : 0;
}

std::int64_t CallboardHunts::ResetDay()
{
    auto const timestamp = std::time_t(sWorld->GetNextDailyQuestsResetTime().count());
    std::tm date{};
    if (!timestamp || !localtime_r(&timestamp, &date)) return -1;
    return std::chrono::sys_days{std::chrono::year{1900 + date.tm_year}
        / unsigned(date.tm_mon + 1) / unsigned(date.tm_mday)}.time_since_epoch().count();
}

char const* CallboardHunts::Today(Player const* player)
{
    auto zone = Active(CallboardRealmTier::ForPlayer(player), ResetDay());
    return zone ? zone->name : nullptr;
}

bool CallboardHunts::CanOffer(Player const* player, unsigned quest)
{
    auto const required = ZoneFor(quest);
    if (!required) return true;
    auto zone = Active(CallboardRealmTier::ForPlayer(player), ResetDay());
    return zone && zone->id == required;
}

bool CallboardRealmTier::CanUse(Player const* player, Quest const* quest)
{
    if (!quest) return false;
    auto const tier = ForPlayer(player);
    if (quest->GetQuestId() >= 14204 && quest->GetQuestId() <= 14252 && quest->GetZoneOrSort() > 0) return tier == 60;
    auto const sort = quest->GetZoneOrSort();
    if (sort != -380 && sort != -381 && sort != -382 && (sort < -548 || sort > -538)) return true;
    auto const catalog = CurrentCatalogue();
    if (!tier || !catalog->ready) return false;
    auto const* contract = catalog->ForQuest(quest->GetQuestId());
    return !contract || Available(contract->type, tier);
}

namespace AscensionCallboardContracts
{
CharacterDatabaseTransaction CurrentRewardTransaction()
{
    return rewardTransaction;
}

bool CanReward(Player const* player, Quest const* quest)
{
    return CallboardRealmTier::CanUse(player, quest) && !ExhaustedContractName(player, quest);
}

void RecordReward(Player* player, Quest const* quest)
{
    if (!enabled.load(std::memory_order_relaxed) || IsBot(player)) return;
    auto contracts = CurrentCatalogue();
    if (Area52MysticRealm::UsesFreePickEngine())
        if (auto const* contract = contracts->ForQuest(quest->GetQuestId())) CountCompletion(player, *contract, false);
}

void AppendSave(Player* player, CharacterDatabaseTransaction transaction)
{
    auto const* data = player->CustomData.Get<LedgerData>(LedgerKey);
    if (!data || !data->loaded) return;
    auto const week = data->ledger.WeekStartTime();
    for (auto const& type : data->ledger.Types())
    {
        auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_REP_COA_CALLBOARD_CONTRACT);
        statement->SetData(0, player->GetGUID().GetCounter());
        statement->SetData(1, type);
        statement->SetData(2, uint32(week));
        auto const tally = data->ledger.For(type, week);
        statement->SetData(3, tally.completions);
        statement->SetData(4, tally.purchases);
        transaction->Append(statement);
    }
}
}

void AddSC_AscensionCallboardContracts()
{
    new ascension_callboard_contracts_player();
    new ascension_callboard_contracts_world();
    AscensionCompatOpcodes::Claim(RequestViewOpcode, &HandleRequestView);
    AscensionCompatOpcodes::Claim(ActivateOpcode, &HandleActivate);
}
