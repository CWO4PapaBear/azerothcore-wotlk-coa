// Included with Area52CacheRuntime.inc in the cache reward translation unit.
namespace Area52CacheRuntime
{
namespace
{
#include "Area52CacheCatalogMetadata.inc"
#include "CallboardDungeonDrops.inc"
#include "Area52CallboardCategoryMetadata.inc"
#include "Area52CacheRewardExclusions.inc"
bool HasOpenCallboardDrop(uint32 item, bool raidOnly = false)
{
    auto found = CallboardDropMaps.find(item);
    if (found == CallboardDropMaps.end()) return false;
    for (uint32 route : found->second)
    {
        uint32 map = route / 4;
        uint8 difficulty = route % 4;
        auto const* entry = sMapStore.LookupEntry(map);
        if (entry && (!raidOnly || entry->IsRaid())
            && sObjectMgr->GetInstanceTemplate(map) && GetMapDifficultyData(map, Difficulty(difficulty))
            && !DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, map, nullptr, difficulty)) return true;
    }
    return false;
}

uint64 NewCacheRequest()
{
    uint64 request = 0;
    uint64 const maximum = UINT64_MAX - UINT64_MAX % 10;
    do
        request = (uint64(urand(0, UINT32_MAX)) << 32) | urand(0, UINT32_MAX);
    while (!request || request > maximum);
    return request;
}

uint32 RewardCap(Player const*, Options)
{
    return sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
}
using Candidate = Area52CacheSelection::Candidate;
using Category = Area52CacheSelection::Category;

std::array<uint32, 4> Score(Player* player, ItemTemplate const* item,
    Area52Build::CacheBuildContext const& build, uint32 matches)
{
    uint32 path = AscensionWildcard::PrimaryStat(player);
    uint32 primary = path == 1149 ? uint32(ITEM_MOD_STRENGTH) : path == 1150 ? uint32(ITEM_MOD_AGILITY)
        : path == 1151 ? uint32(ITEM_MOD_INTELLECT) : path == 1152 ? uint32(ITEM_MOD_SPIRIT) : UINT32_MAX;
    uint32 role = 0, primaryPoints = 0, total = 0;
    bool tankDefense = false, tankStamina = false;
    for (uint32 index = 0; index < std::min<uint32>(item->StatsCount, MAX_ITEM_PROTO_STATS); ++index)
    {
        auto const& stat = item->ItemStat[index];
        if (stat.ItemStatValue <= 0) continue;
        uint32 points = uint32(stat.ItemStatValue);
        if (stat.ItemStatType == primary) primaryPoints += points;
        bool tank = stat.ItemStatType == ITEM_MOD_DEFENSE_SKILL_RATING
            || stat.ItemStatType == ITEM_MOD_DODGE_RATING || stat.ItemStatType == ITEM_MOD_PARRY_RATING
            || stat.ItemStatType == ITEM_MOD_BLOCK_RATING || stat.ItemStatType == ITEM_MOD_BLOCK_VALUE;
        bool healer = stat.ItemStatType == ITEM_MOD_MANA_REGENERATION || stat.ItemStatType == ITEM_MOD_SPELL_HEALING_DONE;
        bool damage = stat.ItemStatType == ITEM_MOD_ATTACK_POWER || stat.ItemStatType == ITEM_MOD_RANGED_ATTACK_POWER
            || stat.ItemStatType == ITEM_MOD_HIT_RATING || stat.ItemStatType == ITEM_MOD_EXPERTISE_RATING
            || stat.ItemStatType == ITEM_MOD_ARMOR_PENETRATION_RATING || stat.ItemStatType == ITEM_MOD_SPELL_DAMAGE_DONE;
        tankDefense = tankDefense || tank;
        tankStamina = tankStamina || stat.ItemStatType == ITEM_MOD_STAMINA;
        if (((build.Roles & 4) && healer) || ((build.Roles & 8) && damage)) role = 1;
        // Generic spell power benefits both healing and spell damage builds.
        if ((build.Roles & 12) && stat.ItemStatType == ITEM_MOD_SPELL_POWER) role = 1;
        total += points;
    }
    if (build.Roles & 2)
        role = std::max(role, uint32(tankDefense) + uint32(tankStamina) + uint32(item->Armor > 0));
    return {role, primaryPoints, matches, total};
}

bool CategoryOpen(Area52CallboardCategory::Pool const& pool)
{
    if (pool.cap == 60 && pool.item >= 1615003 && pool.item <= 1615009)
    {
        uint32 stage = std::min(7u, sConfigMgr->GetOption<uint32>("Ascension.CallboardCache.ReleaseStage", 7));
        if (stage && sConfigMgr->GetOption<bool>("Ascension.CallboardCache.PreviousStageOnly", false)) --stage;
        if (pool.item - 1615002 > stage) return false;
    }
    if (!pool.minimum || (!pool.map && pool.difficulty == 2 && !Area52CallboardCategory::MythicAvailable(pool.cap)))
        return false;
    if (pool.map)
    {
        for (uint32 difficulty = 0; difficulty < 4; ++difficulty)
            if (sObjectMgr->GetInstanceTemplate(pool.map) && GetMapDifficultyData(pool.map, Difficulty(difficulty))
                && !DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, pool.map, nullptr, uint8(difficulty)))
                return true;
        return false;
    }
    for (uint32 map = 0; map < sMapStore.GetNumRows(); ++map)
    {
        auto const* entry = sMapStore.LookupEntry(map);
        if (entry && entry->IsNonRaidDungeon() && entry->Expansion() == (pool.cap - 60) / 10
            && sObjectMgr->GetInstanceTemplate(map) && GetMapDifficultyData(map, Difficulty(pool.difficulty))
            && !DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, map, nullptr, uint8(pool.difficulty)))
            return true;
    }
    return false;
}

bool CategoryContains(Area52CallboardCategory::Pool const& pool, uint32 item)
{
    auto const found = CallboardCategoryMembers.find(pool.item);
    return found != CallboardCategoryMembers.end() && found->second.contains(item);
}

bool CategoryItemOpen(Area52CallboardCategory::Pool const& pool, uint32 item)
{
    if (!CategoryContains(pool, item))
        return false;
    if (pool.cap == 60)
        return true;
    auto const found = CallboardCategoryRoutes.find(item);
    if (found == CallboardCategoryRoutes.end())
        return false;
    for (uint32 route : found->second)
    {
        uint32 const map = route / 4;
        uint8 const difficulty = route % 4;
        auto const* entry = sMapStore.LookupEntry(map);
        bool const matches = pool.map ? pool.map == map : entry && entry->IsNonRaidDungeon()
            && entry->Expansion() == (pool.cap - 60) / 10 && difficulty == pool.difficulty;
        if (matches && sObjectMgr->GetInstanceTemplate(map) && GetMapDifficultyData(map, Difficulty(difficulty))
            && !DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, map, nullptr, difficulty))
            return true;
    }
    return false;
}

std::set<uint32> OpenCategoryItems(uint32 cap)
{
    std::set<uint32> result;
    for (auto const& pool : CallboardCategories)
        if (pool.cap == cap && CategoryOpen(pool))
            for (uint32 item : CallboardCategoryMembers.at(pool.item))
                if (CategoryItemOpen(pool, item))
                    result.insert(item);
    return result;
}

uint32 FilterCategory(Player* player, Options options, std::vector<Candidate>& candidates)
{
    uint32 const cap = RewardCap(player, options);
    std::vector<Area52CallboardCategory::Pool> pools;
    for (auto pool : CallboardCategories)
    {
        if (pool.cap != cap || !CategoryOpen(pool))
            continue;
        pool.minimum = UINT32_MAX;
        pool.maximum = 0;
        for (auto const& candidate : candidates)
            if (Area52CacheSelection::Available(candidate, cap) && CategoryItemOpen(pool, candidate.item))
            {
                pool.minimum = std::min(pool.minimum, candidate.itemLevel);
                pool.maximum = std::max(pool.maximum, candidate.itemLevel);
            }
        if (pool.maximum)
            pools.push_back(pool);
    }
    if (options.cacheEntry == 1615000)
    {
        std::erase_if(candidates, [&](Candidate const& candidate)
        {
            return std::none_of(pools.begin(), pools.end(), [&](auto const& pool)
            {
                return CategoryItemOpen(pool, candidate.item);
            });
        });
        return candidates.empty() ? 0 : 1615000;
    }
    uint32 selected = options.cacheEntry;
    if (!selected || selected == 978050 || selected == 1378050 || selected == 1478050 || selected == 1615000)
    {
        float const floor = Area52CacheSelection::RewardFloor(candidates, cap);
        selected = Area52CallboardCategory::Choose(pools, cap,
            Area52CacheItemLevel::Equipped(player, floor), options.allowance);
    }
    auto const found = std::find_if(pools.begin(), pools.end(),
        [selected](auto const& pool) { return pool.item == selected; });
    if (found == pools.end())
    {
        candidates.clear();
        return 0;
    }
    std::erase_if(candidates, [&](auto const& candidate) { return !CategoryItemOpen(*found, candidate.item); });
    return selected;
}

Area52CacheSelection::RewardRange CacheRange(std::vector<Candidate> const& candidates,
    double equipped, uint32 cap, Options options, uint64 request)
{
    return Area52CacheSelection::OfferRange(candidates, equipped, cap, options.allowance, request);
}

bool Candidates(Player* player, Options options, std::vector<Candidate>& result,
    uint32* category = nullptr, bool savedOffer = false)
{
    auto build = Area52Build::GetCacheBuildContext(player);
    if (!build.Ready) return false;
    uint32 cap = RewardCap(player, options);
    std::map<uint32, AscensionCacheRewards::SetRelevance> sets;
    auto const sources = options.prestige ? AscensionCacheRewards::OpenSources() : OpenCategoryItems(cap);
    auto const& catalog = options.prestige ? CacheCatalog : CallboardCategoryCatalog;
    for (auto const& [id, record] : catalog)
    {
        if (record.cap != cap) continue;
        auto const* item = sObjectMgr->GetItemTemplate(id);
        if (!item || item->RequiredLevel != record.cap || item->ItemLevel != record.itemLevel
            || item->ItemSet != record.set) return false;
        Candidate candidate;
        candidate.item = id;
        candidate.requiredLevel = item->RequiredLevel;
        candidate.itemLevel = item->ItemLevel;
        candidate.raidSource = HasOpenCallboardDrop(id, true);
        candidate.category = item->Class == ITEM_CLASS_WEAPON ? Category::Weapon
            : (item->InventoryType == INVTYPE_NECK || item->InventoryType == INVTYPE_FINGER
                || item->InventoryType == INVTYPE_TRINKET) ? Category::Accessory : Category::Armor;
        candidate.contentUnlocked = AscensionCacheRewards::TierOpen(item->ItemSet, cap)
            && sources.contains(id);
        ItemTemplate rewardRequirements = *item;
        if (!options.prestige)
            rewardRequirements.RequiredLevel = 0;
        candidate.usable = player->CanUseItem(&rewardRequirements) == EQUIP_ERR_OK;
        auto [set, inserted] = sets.try_emplace(item->ItemSet);
        if (inserted) set->second = AscensionCacheRewards::EvaluateSet(player, item->ItemSet);
        if (!set->second.ready) return false;
        candidate.bonusesQualified = set->second.useful;
        candidate.suitability = Score(player, item, build, set->second.matchingAbilities);
        result.push_back(candidate);
    }
    if (result.empty() || !AscensionCacheRewards::PopulateInventoryAndBankOwnership(player, result))
        return false;
    if (!options.prestige && !savedOffer)
    {
        std::set<uint32> ownedFamilies;
        for (auto const& candidate : result)
            if (candidate.owned)
            {
                auto const alias = CallboardRewardAliases.find(candidate.item);
                ownedFamilies.insert(alias == CallboardRewardAliases.end() ? candidate.item : alias->second);
            }
        for (auto& candidate : result)
        {
            auto const alias = CallboardRewardAliases.find(candidate.item);
            uint32 const family = alias == CallboardRewardAliases.end() ? candidate.item : alias->second;
            candidate.owned = candidate.owned || ownedFamilies.contains(family);
        }
    }
    float const floor = Area52CacheSelection::RewardFloor(result, cap);
    for (Candidate& candidate : result)
        candidate.replacementWeight = Area52CacheItemLevel::ReplacementWeight(player,
            sObjectMgr->GetItemTemplate(candidate.item), floor);
    if (!options.prestige)
    {
        if (!savedOffer)
            std::erase_if(result, [&](Candidate const& candidate)
            {
                return CallboardNewRewardExclusions.contains(candidate.item);
            });
        if (!savedOffer)
            for (auto& candidate : result)
                candidate.improvesSlot = Area52CacheItemLevel::ImprovesSlot(player,
                    sObjectMgr->GetItemTemplate(candidate.item), floor);
        uint32 const selected = FilterCategory(player, options, result);
        if (category) *category = selected;
        return selected != 0;
    }
    return true;
}

std::vector<Area52CacheSelection::Choice> Tokens(Player* player, std::vector<Candidate> const& candidates)
{
    std::map<uint32, Candidate const*> items;
    for (auto const& candidate : candidates)
        if (candidate.contentUnlocked && candidate.usable && candidate.bonusesQualified && !candidate.owned)
            items.emplace(candidate.item, &candidate);
    std::map<uint32, std::pair<Category, std::array<uint32,4>>> scores;
    for (auto const& route : CacheTokenRoutes)
    {
        auto found = items.find(route.item);
        auto const* token = sObjectMgr->GetItemTemplate(route.token);
        if (found == items.end() || !token || player->CanUseItem(token) != EQUIP_ERR_OK) continue;
        if (route.quest)
        {
            auto const* quest = sObjectMgr->GetQuestTemplate(route.quest);
            if (!quest || DisableMgr::IsDisabledFor(DISABLE_TYPE_QUEST, route.quest, nullptr)
                || !player->CanTakeQuest(quest, false)) continue;
        }
        auto score = std::make_pair(found->second->category, found->second->suitability);
        auto old = scores.find(route.token);
        if (old == scores.end() || score.first < old->second.first
            || (score.first == old->second.first && score.second > old->second.second)) scores[route.token] = score;
    }
    std::vector<Candidate> tokenCandidates;
    tokenCandidates.reserve(scores.size());
    for (auto const& [id, score] : scores)
    {
        Candidate candidate;
        candidate.item = id;
        candidate.category = score.first;
        candidate.suitability = score.second;
        tokenCandidates.push_back(candidate);
    }
    std::vector<Candidate const*> ranked;
    for (auto const& candidate : tokenCandidates)
        ranked.push_back(&candidate);
    return Area52CacheSelection::WeightedChoices(ranked);
}
}

uint32 CallboardRewardItem(Player* player)
{
    std::vector<Candidate> candidates;
    uint32 selected = 0;
    return Candidates(player, Options{}, candidates, &selected) ? selected : 0;
}

bool LoadCatalog()
{
    uint32 cap = uint32(sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
    if (!CallboardRealmTier::PlayerTier(cap, cap)) return false;
    for (auto const& [id, record] : CacheCatalog)
    {
        auto const* item = sObjectMgr->GetItemTemplate(id);
        if (!item || item->RequiredLevel != record.cap || item->ItemLevel != record.itemLevel || item->ItemSet != record.set)
            return false;
    }
    for (auto const& [id, record] : CallboardCategoryCatalog)
    {
        auto const* item = sObjectMgr->GetItemTemplate(id);
        if (!item || item->RequiredLevel != record.cap || item->ItemLevel != record.itemLevel || item->ItemSet != record.set)
            return false;
    }
    for (auto const& pool : CallboardCategories)
        if (!sObjectMgr->GetItemTemplate(pool.item))
            return false;
    // Missing schema must block initialization, never masquerade as an empty saved reward.
    return bool(CharacterDatabase.Query("SELECT COUNT(*) FROM mod_area52_cache_attempt"));
}

std::optional<Area52CacheAttempt::State> SelectAttempt(Player* player, Item* cache,
    Options options, std::optional<Area52CacheAttempt::State> previous)
{
    std::vector<Candidate> candidates;
    auto fail = [&](char const* message) -> std::optional<Area52CacheAttempt::State>
    { ChatHandler(player->GetSession()).SendSysMessage(message); return std::nullopt; };
    if (!Candidates(player, options, candidates)) return fail("Cache eligibility or ownership is still loading. Nothing was consumed.");
    auto state = previous.value_or(Area52CacheAttempt::State{});
    if (!previous)
    {
        state.owner = player->GetGUID().GetCounter();
        state.cache = cache->GetGUID().GetCounter();
        state.request = NewCacheRequest();
    }
    float const floor = Area52CacheSelection::RewardFloor(candidates, RewardCap(player, options));
    auto const range = CacheRange(candidates,
        Area52CacheItemLevel::Equipped(player, floor), RewardCap(player, options), options, state.request);
    auto selection = Area52CacheSelection::Select(candidates, range.minimum,
        range.maximum, state.declined, RewardCap(player, options), true);
    bool compensation = options.prestige && selection.outcome == Area52CacheSelection::Outcome::AllOwned;
    if (selection.choices.empty() && !compensation)
        return fail("No unowned eligible reward is available in your item-level range. The cache has not been consumed.");
    if (!previous)
    {
        uint32 const maximumGold = MAX_MONEY_AMOUNT / GOLD;
        state.copper = urand(std::min(options.goldMin, maximumGold),
            std::min(std::max(options.goldMin, options.goldMax), maximumGold)) * GOLD;
        if (options.prestige && roll_chance_i(int32(std::min(options.tokenChance, 100u))))
        {
            auto tokens = Tokens(player, candidates);
            uint32 count = roll_chance_i(int32(std::min(options.secondTokenChance, 100u))) ? 2 : 1;
            if (!tokens.empty())
            {
                std::vector<double> weights;
                for (auto const& token : tokens)
                    weights.push_back(token.weight);
                for (uint32 index = 0; index < count; ++index)
                    state.tokens.push_back({tokens[urandweighted(weights.size(), weights.data())].item, 1});
            }
        }
    }
    else ++state.revision;
    if (compensation)
    {
        state.offered = 975001;
        state.offeredCount = 10000;
        state.offeredRandomProperty = 0;
        state.phase = Area52CacheAttempt::Phase::Ready;
    }
    else
    {
        std::vector<double> weights;
        weights.reserve(selection.choices.size());
        for (auto const& choice : selection.choices)
            weights.push_back(choice.weight);
        auto const choice = selection.choices[urandweighted(weights.size(), weights.data())];
        state.offered = choice.item;
        state.offeredCount = 1;
        state.offeredRandomProperty = Item::GenerateItemRandomPropertyId(choice.item);
        state.phase = choice.requiresAppearanceChoice ? Area52CacheAttempt::Phase::Choosing : Area52CacheAttempt::Phase::Ready;
    }
    return state;
}

bool StillEligible(Player* player, Area52CacheAttempt::State const& state, Options options)
{
    std::vector<Candidate> candidates;
    if (!Candidates(player, options, candidates, nullptr, true)) return false;
    bool eligible = false;
    if (options.prestige && state.offered == 975001 && state.offeredCount == 10000)
        eligible = Area52CacheSelection::Select(candidates, 0, 0, {},
            uint32(sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)), true).outcome == Area52CacheSelection::Outcome::AllOwned;
    else
        for (auto const& candidate : candidates)
            if (candidate.item == state.offered)
            {
                float const floor = Area52CacheSelection::RewardFloor(candidates, RewardCap(player, options));
                auto const range = CacheRange(candidates,
                    Area52CacheItemLevel::Equipped(player, floor), RewardCap(player, options), options, state.request);
                auto const ordinary = Area52CacheSelection::ProgressionRange(candidates,
                    Area52CacheItemLevel::Equipped(player, floor), RewardCap(player, options), options.allowance);
                bool const inRange = candidate.itemLevel >= range.minimum && candidate.itemLevel <= range.maximum;
                bool const legacyOffer = (candidate.itemLevel >= ordinary.minimum || candidate.replacementWeight > 1)
                    && candidate.itemLevel <= ordinary.maximum;
                eligible = candidate.contentUnlocked && candidate.usable && candidate.bonusesQualified && !candidate.owned
                    && (!options.prestige || inRange || legacyOffer);
            }
    if (!eligible) return false;
    auto tokens = Tokens(player, candidates);
    for (auto const& token : state.tokens)
        if (std::none_of(tokens.begin(), tokens.end(),
            [&](auto const& choice) { return choice.item == token.item; })) return false;
    return true;
}
}
