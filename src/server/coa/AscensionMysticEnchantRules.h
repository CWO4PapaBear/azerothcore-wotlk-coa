/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#ifndef ASCENSION_MYSTIC_ENCHANT_RULES_H
#define ASCENSION_MYSTIC_ENCHANT_RULES_H

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace AscensionMysticEnchant
{
constexpr std::uint32_t SLOT_COUNT = 17;
constexpr std::size_t REQUIREMENT_COUNT = 3;
constexpr std::size_t REALM_COUNT = 5;
constexpr std::uint32_t HERO_CLASS = 10;
constexpr std::uint32_t FUSION_EXCLUDED_MODES = 0x1DEF;
constexpr std::uint32_t WILDCARD_MODE = 0x40;
constexpr std::uint32_t NO_REQUIREMENT = 1;
constexpr std::uint32_t MAX_PRESETS = 100;
constexpr std::uint32_t RARE_WORLDFORGED_LIMIT = 3;
constexpr std::uint32_t UNTARNISHED_MYSTIC_SCROLL = 992720;
constexpr std::uint32_t PRESET_UNLOCK_TOKEN = 1806961;

enum Quality : std::uint32_t
{
    QUALITY_POOR,
    QUALITY_NORMAL,
    QUALITY_UNCOMMON,
    QUALITY_RARE,
    QUALITY_EPIC,
    QUALITY_LEGENDARY,
    QUALITY_ARTIFACT,
    QUALITY_HEIRLOOM,
    QUALITY_MAX
};

extern std::array<char const*, QUALITY_MAX + 1> const QUALITY_NAMES;

enum ApplyResult : std::uint32_t
{
    APPLY_OK,
    APPLY_UNKNOWN,
    APPLY_BAD_ITEM,
    APPLY_NOT_MYSTIC_SCROLL,
    APPLY_BAD_SLOT,
    APPLY_DUPLICATE_BAG_SLOT_PAIR,
    APPLY_STACK_LIMIT,
    APPLY_UNCOMMON_LIMIT,
    APPLY_RARE_LIMIT,
    APPLY_EPIC_LIMIT,
    APPLY_LEGENDARY_LIMIT,
    APPLY_ARTIFACT_LIMIT,
    APPLY_UNHANDLED_LIMIT,
    APPLY_BAD_ENCHANTMENT,
    APPLY_BUILD_DRAFT,
    APPLY_NOT_WHILE_CASTING,
    APPLY_NO_MONEY,
    APPLY_BAD_CLASS,
    APPLY_BAD_REALM,
    APPLY_DISABLED_IN_WILDCARD,
    APPLY_RARE_WORLDFORGED_LIMIT,
    APPLY_REQUIRED_AE_INVESTMENT,
    APPLY_REQUIRED_TE_INVESTMENT,
    APPLY_TOO_LOW_LEVEL,
    APPLY_DUPLICATE_SLOT,
    APPLY_NO_MYSTIC_ALTAR,
    APPLY_RESULT_COUNT
};

enum DestroyResult : std::uint32_t
{
    DESTROY_OK,
    DESTROY_UNKNOWN,
    DESTROY_BAD_SLOT,
    DESTROY_NO_ENCHANT_APPLIED,
    DESTROY_NOT_WHILE_CASTING,
    DESTROY_BAD_CLASS,
    DESTROY_BUILD_DRAFT,
    DESTROY_RESULT_COUNT
};

enum PurchaseResult : std::uint32_t
{
    PURCHASE_OK,
    PURCHASE_UNKNOWN,
    PURCHASE_ITEM_NOT_FOUND,
    PURCHASE_NOT_ENOUGH_MONEY,
    PURCHASE_NOT_ENOUGH_SPACE,
    PURCHASE_NO_MYSTIC_ALTAR,
    PURCHASE_NOT_WHILE_CASTING,
    PURCHASE_BAD_CLASS,
    PURCHASE_RESULT_COUNT
};

enum InspectResult : std::uint32_t
{
    INSPECT_OK,
    INSPECT_UNKNOWN,
    INSPECT_PLAYER_NOT_FOUND,
    INSPECT_PLAYER_NOT_IN_MAP,
    INSPECT_BAD_CLASS,
    INSPECT_RESULT_COUNT
};

enum PresetSaveResult : std::uint32_t
{
    PRESET_SAVE_OK,
    PRESET_SAVE_UNKNOWN,
    PRESET_SAVE_BUILD_DRAFT,
    PRESET_SAVE_BAD_CLASS,
    PRESET_SAVE_RESULT_COUNT
};

enum PresetActivateResult : std::uint32_t
{
    PRESET_SET_ACTIVE_OK,
    PRESET_SET_ACTIVE_UNKNOWN,
    PRESET_SET_ACTIVE_BUILD_DRAFT,
    PRESET_SET_ACTIVE_NOT_WHILE_CASTING,
    PRESET_SET_ACTIVE_BAD_CLASS,
    PRESET_SET_ACTIVE_RESULT_COUNT
};

enum PresetUnlockResult : std::uint32_t
{
    PRESET_UNLOCK_OK,
    PRESET_UNLOCK_UNKNOWN,
    PRESET_UNLOCK_MAX_VALUE_REACHED,
    PRESET_UNLOCK_NO_MONEY,
    PRESET_UNLOCK_BUILD_DRAFT,
    PRESET_UNLOCK_NOT_WHILE_CASTING,
    PRESET_UNLOCK_BAD_CLASS,
    PRESET_UNLOCK_RESULT_COUNT
};

extern std::array<char const*, APPLY_RESULT_COUNT> const APPLY_RESULTS;
extern std::array<char const*, DESTROY_RESULT_COUNT> const DESTROY_RESULTS;
extern std::array<char const*, PURCHASE_RESULT_COUNT> const PURCHASE_RESULTS;
extern std::array<char const*, INSPECT_RESULT_COUNT> const INSPECT_RESULTS;
extern std::array<char const*, PRESET_SAVE_RESULT_COUNT> const PRESET_SAVE_RESULTS;
extern std::array<char const*, PRESET_SET_ACTIVE_RESULT_COUNT> const PRESET_SET_ACTIVE_RESULTS;
extern std::array<char const*, PRESET_UNLOCK_RESULT_COUNT> const PRESET_UNLOCK_RESULTS;

struct Enchant
{
    std::uint32_t Id = 0;
    std::uint32_t Spell = 0;
    float Weight = 0.0f;
    std::uint32_t Quality = QUALITY_POOR;
    std::uint32_t RebornQuality = QUALITY_POOR;
    std::uint32_t RequiredLevel = 0;
    std::uint32_t Item = 0;
    bool Worldforged = false;
    std::array<bool, REALM_COUNT> Realms{};
    std::uint64_t ClassMask = 0;
    std::array<std::uint32_t, REQUIREMENT_COUNT> ClassTypes{};
    std::array<std::uint32_t, REQUIREMENT_COUNT> Tabs{};
    std::array<std::uint32_t, REQUIREMENT_COUNT> RequiredAE{};
    std::array<std::uint32_t, REQUIREMENT_COUNT> RequiredTE{};
};

struct Catalog
{
    std::vector<Enchant> Rows;
    std::unordered_map<std::uint32_t, std::size_t> BySpell;
    std::unordered_map<std::uint32_t, std::size_t> ByItem;

    [[nodiscard]] Enchant const* FindSpell(std::uint32_t spell) const;
    [[nodiscard]] Enchant const* FindItem(std::uint32_t item) const;
};

bool LoadCatalog(Catalog& catalog);

struct ClientConfig
{
    std::unordered_map<std::string, std::int32_t> Integers;
    std::unordered_map<std::string, bool> Booleans;

    [[nodiscard]] std::int32_t Int(std::string const& name, std::int32_t fallback) const;
    [[nodiscard]] bool Bool(std::string const& name, bool fallback) const;
};

using InvestmentLookup = std::function<std::uint32_t(std::uint32_t classType, std::uint32_t tab, bool talent)>;
using StackLookup = std::function<std::uint32_t(std::uint32_t spell)>;

struct Character
{
    std::uint32_t Class = 0;
    std::uint32_t Level = 0;
    std::uint32_t Money = 0;
    std::uint32_t GameModes = 0;
    bool Draft = false;
    bool Casting = false;
    bool AltarNearby = false;
    bool BagSpace = true;
    std::uint32_t UnlockTokens = 0;
    std::array<bool, REALM_COUNT> RealmGates{};
    ClientConfig const* Config = nullptr;
    InvestmentLookup Invested;
    StackLookup StackLimit;

    [[nodiscard]] bool Fusion() const;
    [[nodiscard]] bool Wildcard() const { return (GameModes & WILDCARD_MODE) != 0; }
};

using Slots = std::array<std::uint32_t, SLOT_COUNT>;

struct StagedApply
{
    std::uint32_t Slot = 0;
    std::uint32_t ItemEntry = 0;
    std::uint32_t ItemKey = 0;
    bool ItemFound = false;
};

bool IsStockClass(std::uint32_t classId);
bool IsConquestOfAzerothClass(std::uint32_t classId);
std::uint32_t QualityOf(Enchant const& enchant, std::uint32_t classId);
bool ClassAllowed(Enchant const& enchant, std::uint32_t classId);
bool RealmAllows(Enchant const& enchant, std::array<bool, REALM_COUNT> const& gates);
bool SlotValid(std::uint32_t slot, bool fusion);
std::uint32_t SlotAt(Slots const& slots, std::uint32_t slot, bool fusion);
std::uint32_t QualityCap(Character const& character, std::uint32_t quality);
bool InvestmentMet(Character const& character, Enchant const& enchant, bool talent);

std::vector<std::uint32_t> CheckApplySlot(Catalog const& catalog, Character const& character, Slots const& slots,
    std::vector<StagedApply> const& staged, StagedApply const& entry);
std::uint32_t CheckSaveApply(Catalog const& catalog, Character const& character, Slots const& slots,
    std::vector<StagedApply> const& staged);
std::uint32_t CheckDestroy(Character const& character, Slots const& slots, std::uint32_t slot);
std::uint32_t CheckPurchaseScroll(Character const& character, bool scrollKnown, std::uint32_t price);
std::uint32_t CheckInspect(bool found, bool sameMap, std::uint32_t targetClass);
std::uint32_t CheckPresetSave(Character const& character);
std::uint32_t CheckPresetActivate(Character const& character, std::uint32_t preset, std::uint32_t presetCount);
std::uint32_t CheckPresetUnlock(Character const& character, std::uint32_t presetCount);
}

#endif
