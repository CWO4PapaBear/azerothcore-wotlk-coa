// Included with Area52CacheRuntime.inc in the cache reward translation unit.
namespace Area52CacheRuntime
{
namespace
{
#include "Area52CacheCatalogMetadata.inc"
#include "CallboardDungeonDrops.inc"
bool HasOpenCallboardDrop(uint32 item)
{
    auto found = CallboardDropMaps.find(item);
    if (found == CallboardDropMaps.end()) return false;
    for (uint32 route : found->second)
    {
        uint32 map = route / 4;
        uint8 difficulty = route % 4;
        if (sObjectMgr->GetInstanceTemplate(map) && GetMapDifficultyData(map, Difficulty(difficulty))
            && !DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, map, nullptr, difficulty)) return true;
    }
    return false;
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
        if (((build.Roles & 2) && tank) || ((build.Roles & 4) && healer) || ((build.Roles & 8) && damage)) role = 1;
        // Generic spell power benefits both healing and spell damage builds.
        if ((build.Roles & 12) && stat.ItemStatType == ITEM_MOD_SPELL_POWER) role = 1;
        total += points;
    }
    return {role, primaryPoints ? 1u : 0u, matches, total};
}

bool Candidates(Player* player, Options options, std::vector<Candidate>& result)
{
    auto build = Area52Build::GetCacheBuildContext(player);
    if (!build.Ready) return false;
    uint32 cap = RewardCap(player, options);
    std::map<uint32, AscensionCacheRewards::SetRelevance> sets;
    auto const sources = AscensionCacheRewards::OpenSources();
    for (auto const& [id, record] : CacheCatalog)
    {
        if (record.cap != cap || (!options.prestige && !HasOpenCallboardDrop(id))) continue;
        auto const* item = sObjectMgr->GetItemTemplate(id);
        if (!item || item->RequiredLevel != record.cap || item->ItemLevel != record.itemLevel
            || item->ItemSet != record.set) return false;
        Candidate candidate;
        candidate.item = id;
        candidate.requiredLevel = item->RequiredLevel;
        candidate.itemLevel = item->ItemLevel;
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
    return !result.empty() && AscensionCacheRewards::PopulateInventoryAndBankOwnership(player, result);
}

std::vector<uint32> Tokens(Player* player, std::vector<Candidate> const& candidates)
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
    std::vector<uint32> result;
    std::optional<std::pair<Category, std::array<uint32,4>>> best;
    for (auto const& [id, score] : scores)
    {
        if (!best || score.first < best->first || (score.first == best->first && score.second > best->second))
        { best = score; result.clear(); }
        if (score == *best) result.push_back(id);
    }
    return result;
}
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
    if (!options.prestige)
    {
        float const average = Area52CacheSelection::CallboardAverage(candidates,
            Area52CacheItemLevel::Equipped(player), RewardCap(player, options));
        for (auto& candidate : candidates)
            if (candidate.itemLevel < average || candidate.itemLevel > average + 6.0f)
                candidate.contentUnlocked = false;
    }
    float const effectiveAverage = options.prestige ? Area52CacheItemLevel::Equipped(player)
        : Area52CacheSelection::CallboardAverage(candidates, Area52CacheItemLevel::Equipped(player),
            RewardCap(player, options));
    auto selection = Area52CacheSelection::Select(candidates, uint32(effectiveAverage),
        options.allowance, state.declined, RewardCap(player, options), true);
    bool compensation = options.prestige && selection.outcome == Area52CacheSelection::Outcome::AllOwned;
    if (selection.choices.empty() && !compensation)
        return fail("No unowned eligible reward is available in your item-level range. The cache has not been consumed.");
    if (!previous)
    {
        state.owner = player->GetGUID().GetCounter();
        state.cache = cache->GetGUID().GetCounter();
        state.request = (uint64(urand(1, UINT32_MAX)) << 32) | urand(1, UINT32_MAX);
        uint32 const maximumGold = MAX_MONEY_AMOUNT / GOLD;
        state.copper = urand(std::min(options.goldMin, maximumGold),
            std::min(std::max(options.goldMin, options.goldMax), maximumGold)) * GOLD;
        if (options.prestige && roll_chance_i(int32(std::min(options.tokenChance, 100u))))
        {
            auto tokens = Tokens(player, candidates);
            uint32 count = roll_chance_i(int32(std::min(options.secondTokenChance, 100u))) ? 2 : 1;
            if (!tokens.empty())
                for (uint32 index = 0; index < count; ++index)
                    state.tokens.push_back({tokens[urand(0, uint32(tokens.size()-1))], 1});
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
        auto const choice = selection.choices[urand(0, uint32(selection.choices.size()-1))];
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
    if (!Candidates(player, options, candidates)) return false;
    bool eligible = false;
    if (options.prestige && state.offered == 975001 && state.offeredCount == 10000)
        eligible = Area52CacheSelection::Select(candidates, 0, 0, {},
            uint32(sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)), true).outcome == Area52CacheSelection::Outcome::AllOwned;
    else
        for (auto const& candidate : candidates)
            if (candidate.item == state.offered)
            {
                float const equipped = Area52CacheItemLevel::Equipped(player);
                float const average = options.prestige ? float(uint32(equipped))
                    : Area52CacheSelection::CallboardAverage(candidates, equipped, RewardCap(player, options));
                eligible = candidate.contentUnlocked && candidate.usable && candidate.bonusesQualified && !candidate.owned
                    && candidate.itemLevel >= average && candidate.itemLevel <= average + options.allowance;
            }
    if (!eligible) return false;
    auto tokens = Tokens(player, candidates);
    for (auto const& token : state.tokens)
        if (std::find(tokens.begin(), tokens.end(), token.item) == tokens.end()) return false;
    return true;
}
}
