#include "Area52MysticRealm.h"
#include "Area52FeaturedPets.h"
#include "Area52DraftPet.h"
#include "Area52DraftVanity.h"
#include "Area52WorldforgedCatalog.h"
#include "Area52Worldforged.h"
/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionMysticEnchantCatalog.h"
#include "Area52BuildProtocol.h"
#include "Area52ActiveBuild.h"
#include "AscensionMysticSpellGrants.h"
#include "AscensionFreePick.h"
#include "AscensionCompatOpcodes.h"
#include "AscensionCoAConfig.h"
#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "GameObject.h"
#include "Item.h"
#include "Mail.h"
#include "MailMgr.h"
#include "LootMgr.h"
#include "Random.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScriptLoader.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <algorithm>
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string_view>

using namespace Acore::ChatCommands;

bool HasArea52EpicEquipmentHandlers(uint32 spell);
bool HasArea52WhiteWalkerHandlers(uint32 spell);
bool HasArea52HopliteHandlers(uint32 spell);

extern pAuraEffectHandler AuraEffectHandler[TOTAL_AURAS];
extern pEffect SpellEffects[TOTAL_SPELL_EFFECTS];

#include "AscensionMysticAudit.h"
#include "Area52MysticTestAccess.h"

#include "Area52AdvancementProcRules.h"
#include "Area52AdvancementProcs.inc"
#include "Area52FollowupProcRules.h"
#include "AllSpellScript.h"
#include "Creature.h"
#include "Area52FollowupProcs.inc"
#include "Area52LegendaryRules.h"
#include "GameTime.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Area52BattlemageRules.h"
#include "Area52LegendaryProcs.inc"
#include "Area52Battlemage.inc"
#include "Area52MarksmanRules.h"
#include "Area52Marksman.inc"
#include "Area52DragonWarriorRules.h"
#include "Area52DragonWarrior.inc"
#include "Area52LegendaryBatchRules.h"
#include "Area52LegendaryBatch.inc"
#include "Area52Felfire.inc"
#include "Area52CorruptedBear.inc"
#include "Area52CastMovement.inc"
#include "Area52Dependencies.inc"
#include "Area52HopliteSupport.inc"
#include "Area52HopliteRemaining.inc"

namespace { void ClearInvalidDefiasBuffs(Player* player); }

namespace AscensionMysticEnchant
{
namespace
{
Catalog Data;
std::map<std::uint32_t, std::uint32_t> Connected;
bool Active = false;
std::set<std::uint32_t> TestAccess;
std::set<std::uint32_t> TestAccessExcludedAccounts;
void AddTestAccess(Player* player, std::set<std::uint32_t>& known)
{
    if (player && player->GetSession())
        AddVerifiedTestAccess(known, TestAccess, TestAccessExcludedAccounts, player->GetSession()->GetAccountId());
}
std::atomic<std::uint64_t> NextGeneration{0};
constexpr char StateKey[] = "coa.freepick.mystic";
constexpr std::array<std::uint32_t, 6> Prices = {0, 0, 300000, 600000, 1000000, 2500000};
constexpr std::array<std::uint32_t, 6> Tokens = {0, 0, 250, 500, 1000, 2000};
struct State : DataMap::Base
{
    std::set<std::uint32_t> Known;
    bool DraftResolved = false;
    std::set<std::uint32_t> DraftEntitlement;
    std::set<std::uint32_t> Applied;
    std::uint64_t Generation = ++NextGeneration;
    bool Loaded = false;
    bool Busy = false;
};

bool Applies(Player const* player)
{
    return Active && AscensionFreePick::Applies(player);
}

bool GrantRequirementMet(Player const* player, std::uint32_t required)
{
    if (!required)
        return true;
    std::set<std::uint32_t> visited;
    for (auto rank = sSpellMgr->GetFirstSpellInChain(required); rank && visited.insert(rank).second;
        rank = sSpellMgr->GetNextSpellInChain(rank))
        if (player->HasSpell(rank))
            return true;
    return false;
}

constexpr char ActivePresetSetting[] = "core.area52.mystic.active";

std::uint32_t PresetCount(Player const* player)
{
    std::uint32_t count = std::max<std::uint32_t>(1, player->GetSpecsCount());
    for (std::uint32_t i = 0; i < AscensionFreePick::SpecializationSpells.size(); ++i)
    {
        if (!player->HasSpell(AscensionFreePick::SpecializationSpells[i]))
            break;
        count = std::max(count, i + 1);
    }
    return count;
}

std::uint32_t ActivePreset(Player const* player)
{
    auto const* stored = player->FindPlayerSettings(ActivePresetSetting);
    return stored && !stored->empty() ? (*stored)[0].value : player->GetActiveSpec();
}

std::string Setting(Player const* player)
{
    return "core.freepick.mystic." + std::to_string(ActivePreset(player));
}

constexpr char SpecPresetSetting[] = "core.area52.spec.preset";
constexpr char SpecEquipmentSetting[] = "core.area52.spec.equipment";

uint32 SpecLinkValue(Player const* player, char const* key, uint32 index)
{
    auto const* stored = player->FindPlayerSettings(key);
    return stored && index < stored->size() ? (*stored)[index].value : 0;
}

void SendSpecLink(Player* player, uint32 spec, bool equipment)
{
    WorldPacket packet(equipment ? 0x738 : 0x737, 13);
    uint32 const low = SpecLinkValue(player, equipment ? SpecEquipmentSetting : SpecPresetSetting,
        equipment ? spec * 2 : spec);
    uint32 const high = equipment ? SpecLinkValue(player, SpecEquipmentSetting, spec * 2 + 1) : 0;
    bool const linked = low || high;
    packet << uint32(spec) << uint8(linked);
    if (linked)
    {
        packet << low;
        if (equipment)
            packet << high;
    }
    player->GetSession()->SendPacket(&packet);
}

void SendSpecLinks(Player* player)
{
    for (uint32 spec = 0; spec < AscensionFreePick::SpecializationSpells.size(); ++spec)
    {
        SendSpecLink(player, spec, false);
        SendSpecLink(player, spec, true);
    }
}

void HandleSpecLink(Player* player, WorldPacket& packet)
{
    if (packet.size() < 5)
        return;
    uint32 const spec = packet.read<uint32>(0);
    uint8 const linked = packet.read<uint8>(4);
    bool const equipment = packet.GetOpcode() == 0x73A;
    if (spec >= AscensionFreePick::SpecializationSpells.size() || linked > 1 ||
        packet.size() != (linked ? (equipment ? 13u : 9u) : 5u))
        return;
    uint32 const low = linked ? packet.read<uint32>(5) : 0;
    uint32 const high = linked && equipment ? packet.read<uint32>(9) : 0;
    bool const unlocked = spec == 0 || player->HasSpell(AscensionFreePick::SpecializationSpells[spec]);
    if (!unlocked || (linked && (equipment ? !(low || high) : (!low || low > PresetCount(player)))))
    {
        SendSpecLink(player, spec, equipment);
        return;
    }
    player->UpdatePlayerSetting(equipment ? SpecEquipmentSetting : SpecPresetSetting,
        equipment ? spec * 2 : spec, low);
    if (equipment)
        player->UpdatePlayerSetting(SpecEquipmentSetting, spec * 2 + 1, high);
    player->SaveToDB(false, false);
    SendSpecLink(player, spec, equipment);
}

Slots ReadSlots(Player const* player)
{
    Slots slots{};
    if (auto const* settings = player->FindPlayerSettings(Setting(player)))
        for (std::size_t i = 0; i < std::min(slots.size(), settings->size()); ++i)
            slots[i] = (*settings)[i].value;
    return slots;
}

bool InvestmentMet(Player const* player, Entry const& entry, bool talent)
{
    auto const& requirements = talent ? entry.TalentInvestment : entry.AbilityInvestment;
    if (std::all_of(requirements.begin(), requirements.end(), [](auto value) { return !value; }))
        return true;
    for (std::size_t i = 0; i < requirements.size(); ++i)
        if (requirements[i] && requirements[i] <=
            AscensionFreePick::Investment(player, entry.Classes[i], entry.Tabs[i], talent))
            return true;
    return false;
}

void SendPresets(Player* player)
{
    auto const count = PresetCount(player);
    WorldPacket presets(0x5FF, 4 + count * 72);
    presets << count;
    for (std::uint32_t index = 0; index < count; ++index)
    {
        auto const* stored = player->FindPlayerSettings("core.freepick.mystic." + std::to_string(index));
        presets << std::uint32_t(17);
        for (std::uint32_t slot = 0; slot < 17; ++slot)
            presets << (stored && slot < stored->size() ? (*stored)[slot].value : std::uint32_t(0));
    }
    player->GetSession()->SendPacket(&presets);
    WorldPacket active(0x600, 4);
    active << ActivePreset(player);
    player->GetSession()->SendPacket(&active);
}

void SendSlots(Player* player)
{
    WorldPacket packet(0x5FD, 72);
    packet << std::uint32_t(17);
    for (auto spell : ReadSlots(player))
        packet << spell;
    player->GetSession()->SendPacket(&packet);
    SendPresets(player);
}

void SendKnown(Player* player, State const& state)
{
    WorldPacket packet(0x5F9, 4 + 4 * state.Known.size());
    packet << std::uint32_t(state.Known.size());
    for (auto spell : state.Known)
        packet << spell;
    player->GetSession()->SendPacket(&packet);
}

void SendResult(Player* player, std::uint16_t opcode, char const* result)
{
    WorldPacket packet(opcode, 64);
    packet << result;
    player->GetSession()->SendPacket(&packet);
}

bool NearAltar(Player* player)
{
    constexpr std::array<std::uint32_t, 24> altars = {0x30, 0x13914, 0x2B18A, 0x2B18B, 0x2B18D,
        0x3BD0E, 0x57390, 0xC354F, 0xF428F, 0x1D0B98, 0x1D0B99, 0x3174B8, 0x3182BE,
        0x3183CC, 0x3183CE, 0x3183ED, 0x31869E, 0x3188F0, 0x31956C, 0x31A0B9, 7100000,
        0x7A1233, 0x7A1234, 0x843F77};
    for (auto entry : altars)
        if (player->FindNearestGameObject(entry, 10.0f, true))
            return true;
    return false;
}

bool ValidateOwnedSlots(State const& state, Slots const& slots, std::uint32_t level)
{
    if (state.DraftEntitlement.empty())
        return ValidateSlots(Data, slots, level, state.Known, Connected);
    auto owned = state.DraftEntitlement;
    for (auto spell : slots)
        if (state.Known.contains(spell))
            owned.insert(spell);
    return ValidateSlots(Data, slots, level, owned, Connected);
}

void Synchronize(Player* player)
{
    auto* state = player->CustomData.Get<State>(StateKey);
    if (!state || !state->Loaded || !player->IsInWorld())
        return;
    if (!state->DraftResolved && player->GetPlayerSetting("core.area52.active_build", 4).value &&
        !player->GetPlayerSetting("core.area52.draft_progress", 2).value)
        return;
    std::map<std::uint32_t, std::uint32_t> wanted;
    auto slots = ReadSlots(player);
    auto arranged = slots;
    if (Applies(player) && player->IsAlive() && ArrangeSlots(Data, arranged) && arranged != slots &&
        ValidateOwnedSlots(*state, arranged, player->GetLevel()))
    {
        auto const setting = Setting(player);
        for (std::uint32_t i = 0; i < arranged.size(); ++i)
            player->UpdatePlayerSetting(setting, i, arranged[i]);
        slots = arranged;
        player->SaveToDB(false, false);
        SendSlots(player);
    }
    if (Applies(player) && player->IsAlive() && ValidateOwnedSlots(*state, slots, player->GetLevel()))
        for (auto spell : slots)
            if (spell && InvestmentMet(player, Data.BySpell.at(spell), false) &&
                InvestmentMet(player, Data.BySpell.at(spell), true))
                ++wanted[spell];
    for (auto spell : state->Applied)
        if (!wanted.contains(spell))
            player->RemoveAurasDueToSpell(spell, player->GetGUID());
    state->Applied.clear();
    for (auto const& [spell, count] : wanted)
    {
        Aura* aura = player->GetAura(spell, player->GetGUID());
        if (!aura)
            aura = player->AddAura(spell, player);
        if (aura)
        {
            aura->SetStackAmount(std::uint8_t(count));
            state->Applied.insert(spell);
        }
    }
    if (Applies(player) && !state->Applied.contains(982743))
        player->RemoveAurasDueToSpell(982745, player->GetGUID());
    if (Applies(player) && !state->Applied.contains(997115))
        for (uint32 helper : {997412u, 997539u, 984509u})
            player->RemoveAurasDueToSpell(helper, player->GetGUID());

    for (auto const& grant : SpellGrants)
    {
        bool const enabled = GrantsActive(player, grant.Enchant) &&
            ((grant.Enchant != 997115 && grant.Enchant != 997047) || state->Applied.contains(grant.Enchant)) &&
            ((grant.Enchant != 977831 && grant.Enchant != 982330 && grant.Enchant != 997047) || player->HasSpell(grant.Requires)) &&
            (grant.Enchant != 997220 || player->HasActiveSpell(grant.Requires)) &&
            (grant.Enchant != 81248 || player->HasActiveSpell(33831)) && GrantRequirementMet(player, grant.Requires);
        std::set<std::uint32_t> visited;
        for (auto rank = grant.Spell; rank && visited.insert(rank).second;
            rank = (grant.Enchant == 977831 || grant.Enchant == 997220 || grant.Enchant == 982330 || grant.Enchant == 997047) ? 0 : sSpellMgr->GetNextSpellInChain(rank))
        {
            SpellInfo const* info = sSpellMgr->GetSpellInfo(rank);
            bool const rankDue = (grant.Enchant == 982743 || (grant.Enchant == 979740 && grant.Spell == 826679)) ?
                info && GrantRankDue(rank == grant.Spell, player->HasSpell(rank), info->SpellLevel, player->GetLevel()) :
                rank == grant.Spell || player->HasSpell(rank);
            SetGrantedSpell(player, grant.Enchant, rank, enabled && info &&
                rankDue);
        }
        if (grant.Enchant == 81248)
            player->SetTemporarySpellReplacement(33831, enabled ? 81249 : 0);
        if (grant.Replaces)
            player->SetTemporarySpellReplacement(grant.Replaces, enabled ? grant.Spell : 0);
    }
    for (auto const& retired : RetiredSpellGrants)
        SetGrantedSpell(player, retired.Enchant, retired.Spell, false);
    RefreshEpicReplacements(player);
}

void Load(Player* player)
{
    if (!Applies(player))
        return;
    auto* state = player->CustomData.GetDefault<State>(StateKey);
    if (state->Busy)
        return;
    state->Busy = true;
    auto const guid = player->GetGUID();
    auto const generation = state->Generation;
    WorldSession* session = player->GetSession();
    auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_SEL_FREEPICK_MYSTIC_COLLECTION);
    statement->SetData(0, session->GetAccountId());
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(statement).WithPreparedCallback(
        [session, guid, generation](PreparedQueryResult result)
        {
            Player* owner = session->GetPlayer();
            if (!owner || owner->GetGUID() != guid)
                return;
            auto* current = owner->CustomData.Get<State>(StateKey);
            if (!current || current->Generation != generation)
                return;
            current->Known.clear();
            if (result)
                do
                {
                    auto const spell = result->Fetch()[0].Get<std::uint32_t>();
                    if (Data.BySpell.contains(spell))
                        current->Known.insert(spell);
                } while (result->NextRow());
            AddTestAccess(owner, current->Known);
            if (!current->Loaded)
                for (auto const& [spell, entry] : Data.BySpell)
                    if (auto const* aura = owner->GetAura(spell, owner->GetGUID()))
                        if (aura->GetCastItemGUID().IsEmpty())
                            current->Applied.insert(spell);
            current->Loaded = true;
            current->Busy = false;
            SendKnown(owner, *current);
            SendSlots(owner);
            Synchronize(owner);
        }));
}

bool CanChange(Player* player, State const* state)
{
    return Applies(player) && player->IsInWorld() && player->IsAlive() && state && state->Loaded &&
        !state->Busy && !player->IsInCombat() && !player->IsNonMeleeSpellCast(false) &&
        !player->InBattleground() && !player->InArena();
}

bool Equip(Player* player, Slots const& wanted, bool administrator,
    std::vector<std::uint32_t> const& paymentOrder = {}, char const** rejection = nullptr)
{
    auto reject = [&](char const* reason)
    {
        if (rejection)
            *rejection = reason;
        return false;
    };
    auto* state = player->CustomData.Get<State>(StateKey);
    if (player->IsNonMeleeSpellCast(false))
        return reject("RE_COLLECTION_REFORGE_NOT_WHILE_CASTING");
    if (!CanChange(player, state))
        return reject("RE_COLLECTION_REFORGE_DISABLED");
    auto const before = ReadSlots(player);
    if (before == wanted)
        return ValidateOwnedSlots(*state, wanted, player->GetLevel());
    if (!administrator && Area52Build::IsFeaturedDraft(player))
        return Area52Build::BeforeManualDraftChange(player);

    if (!administrator && !NearAltar(player))
        return reject("RE_COLLECTION_REFORGE_NO_MYSTIC_ALTAR");
    if (!IsRemovalOnly(before, wanted))
    {
        for (auto spell : wanted)
            if (spell && !state->Known.contains(spell) && !state->DraftEntitlement.contains(spell))
                return reject("RE_COLLECTION_REFORGE_NOT_KNOWN_ENCHANTMENT");
        if (!ValidateOwnedSlots(*state, wanted, player->GetLevel()))
            return reject("RE_COLLECTION_REFORGE_BAD_ENCHANTMENT");
        std::uint32_t worldforged = 0;
        for (auto spell : wanted)
            if (spell)
            {
                auto const& entry = Data.BySpell.at(spell);
                worldforged += entry.Worldforged && entry.Quality == 3;
            }
        if (worldforged > 3)
            return false;
        for (std::size_t i = 0; i < wanted.size(); ++i)
            for (std::size_t j = i + 1; j < wanted.size(); ++j)
                if (wanted[i] && wanted[j] && wanted[i] != wanted[j] &&
                    sSpellMgr->CheckSpellGroupStackRules(sSpellMgr->GetSpellInfo(wanted[i]),
                        sSpellMgr->GetSpellInfo(wanted[j])) != SPELL_GROUP_STACK_RULE_DEFAULT)
                    return false;
    }
    std::uint64_t money = 0;
    std::uint32_t marks = 0;
    if (!administrator)
        for (auto i : paymentOrder)
            if (wanted[i] && wanted[i] != before[i])
            {
                auto const quality = Data.BySpell.at(wanted[i]).Quality;
                if (player->GetItemCount(375250) >= marks + Tokens[quality])
                    marks += Tokens[quality];
                else
                    money += Prices[quality];
            }
    if (money > player->GetMoney())
        return reject("RE_COLLECTION_REFORGE_NO_MONEY");
    player->ModifyMoney(-int32(money));
    if (marks)
        player->DestroyItemCount(375250, marks, true);
    auto const setting = Setting(player);
    for (std::uint32_t i = 0; i < wanted.size(); ++i)
        player->UpdatePlayerSetting(setting, i, wanted[i]);
    Synchronize(player);
    player->SaveToDB(false, false);
    auto const equipped = ReadSlots(player);
    for (std::uint32_t i = 0; i < equipped.size(); ++i)
        if (equipped[i] != before[i])
        {
            WorldPacket update(0x5FE, 8);
            update << i << equipped[i];
            player->GetSession()->SendPacket(&update);
        }
    return true;
}

#include "Area52MysticScrolls.inc"

bool Handle(WorldSession* session, WorldPacket const& incoming)
{
    Player* player = session->GetPlayer();
    if (!Applies(player))
        return false;
    WorldPacket packet(incoming);
    auto const opcode = packet.GetOpcode();
    if (opcode == 0x739 || opcode == 0x73A)
    {
        HandleSpecLink(player, packet);
        return true;
    }
    if (opcode == 0x60C)
    {
        auto const* state = player->CustomData.Get<State>(StateKey);
        bool success = false;
        if (packet.size() == 4 && CanChange(player, state))
        {
            auto const preset = packet.read<std::uint32_t>(0);
            if (preset < PresetCount(player) && (preset == ActivePreset(player)
                || Area52Build::BeforeManualDraftChange(player)))
            {
                player->UpdatePlayerSetting(ActivePresetSetting, 0, preset);
                Synchronize(player);
                player->SaveToDB(false, false);
                success = true;
            }
        }
        SendSlots(player);
        SendResult(player, 0x60D, success ? "RE_PRESET_SET_ACTIVE_OK" : "RE_PRESET_SET_ACTIVE_UNKNOWN");
        return true;
    }
    if (opcode == 0x617)
    {
        bool success = false;
        if (packet.size() == 4)
        {
            auto const slot = packet.read<std::uint32_t>(0);
            if (slot < 17)
            {
                auto wanted = ReadSlots(player);
                wanted[slot] = 0;
                success = Equip(player, wanted, true);
            }
        }
        SendResult(player, 0x618, success ? "RE_DESTROY_OK" : "RE_DESTROY_BAD_SLOT");
        return true;
    }
    bool success = false;
    char const* rejection = "RE_COLLECTION_REFORGE_BAD_ENCHANTMENT";
    if (packet.size() >= 4)
    {
        std::uint32_t count;
        packet >> count;
        if (count && count <= 17 && packet.size() == 4 + count * 8)
        {
            auto wanted = ReadSlots(player);
            std::set<std::uint32_t> used;
            bool valid = true;
            std::vector<std::uint32_t> paymentOrder;
            for (std::uint32_t i = 0; i < count; ++i)
            {
                std::uint32_t slot, spell;
                packet >> slot >> spell;
                if (slot >= 17 || !spell || !used.insert(slot).second)
                    valid = false;
                else
                {
                    wanted[slot] = spell;
                    paymentOrder.push_back(slot);
                }
            }
            success = valid && Equip(player, wanted, false, paymentOrder, &rejection);
        }
    }
    SendResult(player, 0x606, success ? "RE_COLLECTION_REFORGE_OK" : rejection);
    return true;
}

bool HasEldritchEquipmentHandlers(uint32 spell)
{
    if (spell != 81116 || !sSpellMgr->GetSpellProcEntry(spell))
        return false;
    std::list<AuraScript*> auras;
    sScriptMgr->CreateAuraScripts(spell, auras);
    bool parent = false;
    for (auto* script : auras)
    {
        parent |= *script->_GetScriptName() == "aura_area52_eldritch_knight";
        delete script;
    }
    if (!parent)
        return false;
    for (uint32 helper : {983708u, 983712u, 983703u, 983704u, 983705u, 983706u, 983707u})
    {
        std::list<SpellScript*> scripts;
        sScriptMgr->CreateSpellScripts(helper, scripts);
        bool found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == (helper == 983708 || helper == 983712 ?
                "spell_area52_eldritch_missing_mana" : "spell_area52_eldritch_condemnation");
            delete script;
        }
        if (!found)
            return false;
    }
    return true;
}

bool HasSpiritualAttunementHandler(uint32 spell)
{
    if (spell != 965377 || !sSpellMgr->GetSpellProcEntry(spell) || !sSpellMgr->GetSpellInfo(31786))
        return false;
    std::list<AuraScript*> scripts;
    sScriptMgr->CreateAuraScripts(spell, scripts);
    bool found = false;
    for (auto* script : scripts)
    {
        found |= *script->_GetScriptName() == "spell_pal_spiritual_attunement";
        delete script;
    }
    return found;
}

bool HasBlessingOfEternalsHandler(uint32 spell)
{
    auto const* info = spell == 951554 ? sSpellMgr->GetSpellInfo(spell) : nullptr;
    if (!info || info->Effects[EFFECT_0].ApplyAuraName != SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL ||
        info->Effects[EFFECT_1].ApplyAuraName != SPELL_AURA_DUMMY || !sSpellMgr->GetSpellProcEntry(51940))
        return false;
    std::list<AuraScript*> scripts;
    sScriptMgr->CreateAuraScripts(51940, scripts);
    bool found = false;
    for (auto* script : scripts)
    {
        found |= *script->_GetScriptName() == "spell_sha_earthliving_weapon";
        delete script;
    }
    return found;
}

bool HasHopliteSupportHandlers(uint32 spell)
{
    if (spell != 782246 && spell != 912326)
        return false;
    std::vector<uint32> dependencies = spell == 782246 ?
        std::vector<uint32>{782247, 782200, 782201, 780999} :
        std::vector<uint32>{912327, 912328, 12328, 997604};
    for (uint32 id : dependencies)
        if (!sSpellMgr->GetSpellInfo(id))
            return false;
    if (!sSpellMgr->GetSpellProcEntry(spell))
        return false;
    std::set<std::string> expected = spell == 782246 ?
        std::set<std::string>{"aura_area52_draconic_sinister"} :
        std::set<std::string>{"spell_area52_hoplite_support_proc", "aura_area52_sweeping_swings"};
    std::list<AuraScript*> scripts;
    sScriptMgr->CreateAuraScripts(spell, scripts);
    for (auto* script : scripts)
    {
        expected.erase(*script->_GetScriptName());
        delete script;
    }
    return expected.empty();
}

bool HasDefiasHandlers()
{
    for (uint32 helper : {997050u, 997054u, 997430u, 997429u, 997416u, 997051u})
        if (!sSpellMgr->GetSpellInfo(helper))
            return false;
    uint32 script = sObjectMgr->GetScriptId("spell_area52_defias_weapon");
    if (!script || !ScriptRegistry<SpellScriptLoader>::GetScriptById(script))
        return false;
    for (auto const& grant : SpellGrants)
        if (grant.Enchant == 997047)
        {
            auto [first, last] = sObjectMgr->GetSpellScriptsBounds(grant.Spell);
            if (!sSpellMgr->GetSpellInfo(grant.Spell) ||
                std::none_of(first, last, [script](auto const& binding) { return binding.second == script; }))
                return false;
        }
    return true;
}

bool HasDarkRangerHandlers()
{
    for (uint32 spell : {997517u, 990236u, 997526u, 300073u, 998204u, 984510u, 984509u,
                         997412u, 997539u, 997742u, 17941u})
        if (!sSpellMgr->GetSpellInfo(spell))
            return false;
    for (auto [id, name] : {std::pair{998204u, "spell_area52_dark_ranger_artillery"},
                            std::pair{984510u, "spell_area52_darkfall_periodic"},
                            std::pair{997526u, "spell_area52_dark_ranger_shadow_arrow"}})
    {
        uint32 script = sObjectMgr->GetScriptId(name);
        auto [first, last] = sObjectMgr->GetSpellScriptsBounds(id);
        if (!script || !ScriptRegistry<SpellScriptLoader>::GetScriptById(script) ||
            std::none_of(first, last, [script](auto const& binding) { return binding.second == script; }))
            return false;
    }
    return true;
}

bool HasShieldFinesseHandlers()
{
    // This enchant reacts to casts through spell_area52_shield_tools.
    auto hasBinding = [](uint32 spell, uint32 script)
    {
        auto [first, last] = sObjectMgr->GetSpellScriptsBounds(spell);
        return script && ScriptRegistry<SpellScriptLoader>::GetScriptById(script) &&
            std::any_of(first, last, [script](auto const& binding) { return binding.second == script; });
    };
    auto const tools = sObjectMgr->GetScriptId("spell_area52_shield_tools");
    if (!sSpellMgr->GetSpellInfo(986126) || !hasBinding(986125, sObjectMgr->GetScriptId("aura_area52_bulwark_cleanup")) ||
        !hasBinding(871, tools) || !hasBinding(2565, tools))
        return false;
    for (uint32 rank = 23922; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
        if (!hasBinding(rank, tools))
            return false;
    return true;
}

bool HasSeedOfLifeHandlers()
{
    if (!sSpellMgr->GetSpellProcEntry(965937) || !sSpellMgr->GetSpellInfo(944827) ||
        !sSpellMgr->GetSpellBonusData(966753))
        return false;
    for (uint32 spell : {965939u, 966753u})
    {
        std::list<SpellScript*> scripts;
        sScriptMgr->CreateSpellScripts(spell, scripts);
        bool found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == (spell == 965939 ?
                "spell_area52_bloom" : "spell_area52_bloom_heal");
            delete script;
        }
        if (!found)
            return false;
    }
    return true;
}

bool HasEpicDruidHandlers(uint32 spell)
{
    uint32 const aura = spell == 81248 ? 81413 : spell;
    if ((spell == 81501 || spell == 415012) && !sSpellMgr->GetSpellProcEntry(spell))
        return false;
    if (spell == 415012)
    {
        std::list<SpellScript*> casts;
        sScriptMgr->CreateSpellScripts(18562, casts);
        bool swiftmend = false;
        for (auto* script : casts)
        {
            swiftmend |= *script->_GetScriptName() == "spell_area52_everliving_swiftmend";
            delete script;
        }
        std::list<AuraScript*> stacks;
        sScriptMgr->CreateAuraScripts(415013, stacks);
        bool modifier = false;
        for (auto* script : stacks)
        {
            modifier |= *script->_GetScriptName() == "aura_area52_everliving_stacks";
            delete script;
        }
        if (!swiftmend || !modifier)
            return false;
    }
    if (spell == 81248)
    {
        auto const* creature = sObjectMgr->GetCreatureTemplate(43286);
        if (!creature || creature->ScriptID != sObjectMgr->GetScriptId("npc_ascension_summoned_effect") ||
            !sSpellMgr->GetSpellInfo(81249) || !sSpellMgr->GetSpellBonusData(81413))
            return false;
        std::list<SpellScript*> casts;
        sScriptMgr->CreateSpellScripts(81249, casts);
        bool found = false;
        for (auto* script : casts)
        {
            found |= *script->_GetScriptName() == "spell_area52_call_fungarians";
            delete script;
        }
        if (!found)
            return false;
    }
    std::list<AuraScript*> scripts;
    sScriptMgr->CreateAuraScripts(aura, scripts);
    bool found = false;
    for (auto* script : scripts)
    {
        found |= *script->_GetScriptName() == (spell == 81248 ?
            "aura_area52_cloud_of_rot" : spell == 415012 ? "aura_area52_everliving" : "spell_area52_evergreen");
        delete script;
    }
    return found;
}

#include "Area52WorldforgedCatalog.h"
void LoadConnections()
{
    Active = false;
    Connected.clear();
    TestAccess.clear();
    if (!AscensionFreePick::Enabled() || !sConfigMgr->GetOption<bool>("CoA.FreePickMysticEnchants.Enable", false) ||
        !LoadCatalog(Data, GetClientDBCPath("MysticEnchant.dbc")))
        return;
    std::map<std::uint32_t, SpellNode> graph;
    std::vector<std::uint32_t> pending;
    for (auto const& [spell, entry] : Data.BySpell)
        pending.push_back(spell);
    for (auto const& grant : SpellGrants)
    {
        std::set<std::uint32_t> visited;
        for (auto rank = grant.Spell; rank && visited.insert(rank).second;
            rank = sSpellMgr->GetNextSpellInChain(rank))
            pending.push_back(rank);
    }
    while (!pending.empty())
    {
        auto const spell = pending.back();
        pending.pop_back();
        if (graph.contains(spell))
            continue;
        auto& node = graph[spell];
        auto const* info = sSpellMgr->GetSpellInfo(spell);
        if (!info)
            continue;
        node.Supported = true;
        for (auto const& effect : info->Effects)
        {
            if (!effect.IsEffect())
                continue;
            if (effect.Effect >= TOTAL_SPELL_EFFECTS ||
                SpellEffects[effect.Effect] == &Spell::EffectNULL ||
                SpellEffects[effect.Effect] == &Spell::EffectUnused ||
                effect.Effect == SPELL_EFFECT_DUMMY || effect.Effect == SPELL_EFFECT_SCRIPT_EFFECT)
                node.Supported = false;
            if (effect.ApplyAuraName)
            {
                auto const aura = effect.ApplyAuraName;
                if (aura >= TOTAL_AURAS || aura == SPELL_AURA_DUMMY || aura == SPELL_AURA_PERIODIC_DUMMY ||
                    aura == SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE || aura == SPELL_AURA_PROC_TRIGGER_DAMAGE ||
                    AuraEffectHandler[aura] == &AuraEffect::HandleNULL ||
                    AuraEffectHandler[aura] == &AuraEffect::HandleUnused)
                    node.Supported = false;
                if (aura == SPELL_AURA_PROC_TRIGGER_SPELL && !sSpellMgr->GetSpellProcEntry(spell))
                    node.Supported = false;
            }
            if ((effect.ApplyAuraName == SPELL_AURA_PROC_TRIGGER_SPELL ||
                effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL) && !effect.TriggerSpell)
                node.Supported = false;
            if (effect.TriggerSpell)
            {
                node.Dependencies.insert(effect.TriggerSpell);
                pending.push_back(effect.TriggerSpell);
            }
        }
    }
    for (auto const& [spell, entry] : Data.BySpell)
    {
        auto const* info = sSpellMgr->GetSpellInfo(spell);
        if (entry.Quality < 2 || entry.Quality > 5 || !info || !info->IsPassive() ||
            info->HasAttribute(SPELL_ATTR0_CU_FORCE_AURA_SAVING))
            continue;
        if (spell == 81279 || spell == 982743)
        {
            if (sSpellMgr->GetSpellProcEntry(spell) &&
                (spell == 81279 ? sObjectMgr->GetCreatureTemplate(375036) != nullptr : sSpellMgr->GetSpellInfo(982744) != nullptr))
                Connected.emplace(spell, 1);
            continue;
        }
        if (spell == 997047)
        {
            if (HasDefiasHandlers())
                Connected.emplace(spell, 1);
            continue;
        }
        if (spell == 997115)
        {
            if (HasDarkRangerHandlers())
                Connected.emplace(spell, 1);
            continue;
        }
        if (spell == 954634)
        {
            auto script = sObjectMgr->GetScriptId("aura_area52_blinding_dispersion");
            auto [begin, end] = sObjectMgr->GetSpellScriptsBounds(47585);
            if (sSpellMgr->GetSpellInfo(47585) && sSpellMgr->GetSpellInfo(954635)
                && script && ScriptRegistry<SpellScriptLoader>::GetScriptById(script)
                && std::any_of(begin, end, [script](auto const& row) { return row.second == script; }))
                Connected.emplace(spell, 1);
            continue;
        }
        if (spell == 81214)
        {
            auto bound = [](uint32 id, char const* name)
            {
                auto script = sObjectMgr->GetScriptId(name);
                auto [begin, end] = sObjectMgr->GetSpellScriptsBounds(id);
                return script && ScriptRegistry<SpellScriptLoader>::GetScriptById(script) &&
                    std::any_of(begin, end, [script](auto const& row) { return row.second == script; });
            };
            if (sSpellMgr->GetSpellProcEntry(spell) && sSpellMgr->GetSpellInfo(760198) &&
                sSpellMgr->GetSpellInfo(983846) && sSpellMgr->GetSpellInfo(983848) &&
                bound(spell, "aura_area52_new_moon") && bound(983848, "spell_area52_waxing_moon_explosion"))
                Connected.emplace(spell, 1);
            continue;
        }
        if (spell == 977782)
        {
            if (sSpellMgr->GetSpellInfo(48517) && sSpellMgr->GetSpellInfo(48518) &&
                sSpellMgr->GetSpellInfo(5570) && sSpellMgr->GetSpellInfo(8921))
                Connected.emplace(spell, 1);
            continue;
        }
        if (spell == 986125)
        {
            if (HasShieldFinesseHandlers())
                Connected.emplace(spell, 1);
            continue;
        }
        if (Area52Worldforged::Find(spell))
        {
            if (HasArea52EpicEquipmentHandlers(spell))
                Connected.emplace(spell, std::max<std::uint32_t>(1, info->StackAmount));
            continue;
        }
        if (spell == Area52Legendary::UnboundedDeathbringer)
        {
            if (HasArea52UnboundedDeathbringerHandler())
                Connected.emplace(spell, 1);
            continue;
        }
        if (spell == 81248 || spell == 81501 || spell == 415012)
        {
            if (HasEpicDruidHandlers(spell))
                Connected.emplace(spell, 1);
            continue;
        }
        if (spell == 965937)
        {
            if (HasSeedOfLifeHandlers())
                Connected.emplace(spell, 1);
            continue;
        }
        if (HasSpiritualAttunementHandler(spell))
        {
            Connected.emplace(spell, std::max<std::uint32_t>(1, info->StackAmount));
            continue;
        }
        if (HasBlessingOfEternalsHandler(spell))
        {
            Connected.emplace(spell, std::max<std::uint32_t>(1, info->StackAmount));
            continue;
        }
        if (Area52LegendaryBatch::Managed(spell))
        {
            if (HasArea52LegendaryBatchHandlers(spell))
                Connected.emplace(spell, 1);
            continue;
        }
        if (spell == 782246 || spell == 912326)
        {
            if (HasHopliteSupportHandlers(spell))
                Connected.emplace(spell, std::max<std::uint32_t>(1, info->StackAmount));
            continue;
        }
        if (HasArea52WhiteWalkerHandlers(spell))
        {
            Connected.emplace(spell, std::max<std::uint32_t>(1, info->StackAmount));
            continue;
        }
        if (HasEldritchEquipmentHandlers(spell) || HasArea52EpicEquipmentHandlers(spell) ||
            HasArea52HopliteHandlers(spell) || HasArea52BattlemageHandlers(spell) || HasArea52MarksmanHandlers(spell) ||
            HasArea52DragonWarriorHandlers(spell))
        {
            Connected.emplace(spell, 1);
            continue;
        }
        auto const grant = std::find_if(SpellGrants.begin(), SpellGrants.end(),
            [spell](SpellGrant const& value) { return value.Enchant == spell; });
        if (grant != SpellGrants.end())
        {
            bool supported = true;
            std::set<std::uint32_t> visited;
            for (auto rank = grant->Spell; rank && visited.insert(rank).second;
                rank = sSpellMgr->GetNextSpellInChain(rank))
                supported = supported && ResolveNative(rank, graph);
            if (supported)
                Connected.emplace(spell, 1);
            continue;
        }
        if (!ResolveNative(spell, graph))
            continue;
        bool const auraOnly = std::all_of(info->Effects.begin(), info->Effects.end(), [](auto const& effect)
            { return !effect.IsEffect() || effect.Effect == SPELL_EFFECT_APPLY_AURA; });
        if (auraOnly && std::any_of(info->Effects.begin(), info->Effects.end(),
            [](auto const& effect) { return effect.IsEffect(); }))
            Connected.emplace(spell, std::max<std::uint32_t>(1, info->StackAmount));
    }
    Active = true;
    LOG_INFO("coa", "Free Pick Mystic Enchants: {} connected candidates of {} catalog entries", Connected.size(),
        Data.BySpell.size());
}

void RefreshTestAccess()
{
    std::set<std::uint32_t> next;
    if (Active && sConfigMgr->GetOption<bool>("CoA.FreePickMysticEnchants.VerifiedTestAccess", false))
    {
        try
        {
            TestAccessExcludedAccounts = ParseTestAccessExcludedAccounts(sConfigMgr->GetOption<std::string>(
                "CoA.FreePickMysticEnchants.VerifiedTestAccessExcludedAccounts", ""));
            boost::property_tree::ptree report;
            boost::property_tree::read_json(sConfigMgr->GetOption<std::string>(
                "CoA.FreePickMysticEnchants.VerificationFile", ""), report);
            next = VerifiedTestAccess(report, [](uint32 id)
                { return Data.BySpell.contains(id) && Connected.contains(id); });
        }
        catch (std::exception const& error)
        {
            LOG_ERROR("coa", "Area 52 verified Mystic Enchant access disabled: {}", error.what());
        }
    }
    if (next != TestAccess)
    {
        TestAccess.swap(next);
        LOG_INFO("coa", "Area 52 temporary verified Mystic Enchant access: {} entries", TestAccess.size());
    }
}

void ClientConfig(AscensionClientConfig& config)
{
    if (!Active)
        return;
    std::map<std::string, int32> values = {
        {"CONFIG_MAX_RARE_RANDOM_ENCHANTS", 17}, {"CONFIG_MAX_EPIC_RANDOM_ENCHANTS", 4},
        {"CONFIG_MAX_LEGENDARY_RANDOM_ENCHANTS", 1}, {"CONFIG_MAX_ARTIFACT_RANDOM_ENCHANTS", 0}};
    values["CONFIG_MYSTIC_ENCHANT_REFORGE_TOKEN_COST"] =
        sConfigMgr->GetOption<uint32>("CoA.FreePickMysticEnchants.ReforgeRuneCost", 50);
    values["CONFIG_MYSTIC_ENCHANT_REFORGE_MONEY_COST"] =
        sConfigMgr->GetOption<uint32>("CoA.FreePickMysticEnchants.ReforgeMoneyCost", 25000);
    for (char const* quality : {"UNCOMMON", "RARE", "EPIC", "LEGENDARY", "ARTIFACT"})
        values[std::string("CONFIG_MYSTIC_ENCHANT_EXTRACT_") + quality + "_TOKEN_COST"] = 0;
    constexpr std::array<char const*, 4> names = {"UNCOMMON", "RARE", "EPIC", "LEGENDARY"};
    for (std::size_t i = 0; i < names.size(); ++i)
    {
        auto const prefix = std::string("CONFIG_MYSTIC_ENCHANT_COLLECTION_REFORGE_") + names[i];
        values[prefix + "_MONEY_COST"] = Prices[i + 2];
        values[prefix + "_TOKEN_COST"] = Tokens[i + 2];
    }
    for (auto const& [key, value] : values)
    {
        std::erase_if(config.Integers, [&](auto const& pair) { return pair.first == key; });
        config.Integers.emplace_back(key, value);
    }
}

class MysticWorld final : public WorldScript
{
public:
    MysticWorld() : WorldScript("FreePickMysticWorld") { }
    void OnStartup() override { LoadConnections(); RefreshTestAccess(); }
};

class item_area52_unidentified_mystic final : public ItemScript
{
public:
    item_area52_unidentified_mystic() : ItemScript("item_area52_unidentified_mystic") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        if (item->GetEntry() != 97866 || !Applies(player))
            return false;
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        auto* state = player->CustomData.Get<State>(StateKey);
        if (!CanChange(player, state) || Data.BySpell.empty())
            return true;
        if (Entry const* chosen = DrawScroll())
            ReplaceMysticScroll(player, item, chosen->Item, 0, 0);
        return true;
    }
};

class item_area52_identified_mystic final : public ItemScript
{
public:
    item_area52_identified_mystic() : ItemScript("item_area52_identified_mystic") { }
    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        if (!Applies(player) || !Data.ByItem.contains(item->GetEntry()))
            return false;
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        return true;
    }
};

class area52_mystic_scroll_loot final : public GlobalScript
{
public:
    area52_mystic_scroll_loot() : GlobalScript("area52_mystic_scroll_loot", {GLOBALHOOK_ON_ITEM_ROLL}) { }

    bool OnItemRoll(Player const* player, LootStoreItem const* item, float& chance,
        Loot&, LootStore const& store) override
    {
        if (item->itemid != 97866 || &store != &LootTemplates_Creature)
            return true;
        float const rate = std::clamp(sConfigMgr->GetOption<float>("CoA.FreePickMysticEnchants.ScrollDropChance", 25.0f), 0.0f, 100.0f);
        chance = 100.0f;
        return Applies(player) && roll_chance_f(rate);
    }
};

class spell_freepick_ancient_altar_scroll : public SpellScript
{
    PrepareSpellScript(spell_freepick_ancient_altar_scroll);

    bool Load() override
    {
        auto* player = GetCaster()->ToPlayer();
        auto* item = GetCastItem();
        return player && Applies(player) && item &&
            (item->GetEntry() == 1179126 || item->GetEntry() == 2200025);
    }

    SpellCastResult Check()
    {
        return sObjectMgr->GetGameObjectTemplate(80148) ? SPELL_CAST_OK : SPELL_FAILED_NOT_READY;
    }

    void Summon(SpellEffIndex index)
    {
        PreventHitDefaultEffect(index);
        auto* player = GetCaster()->ToPlayer();
        if (!player)
            return;
        player->SummonGameObject(80148, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
            player->GetOrientation(), 0, 0, 0, 1, 300);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_freepick_ancient_altar_scroll::Check);
        OnEffectHit += SpellEffectFn(spell_freepick_ancient_altar_scroll::Summon, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

bool IsRangedEnchantPrerequisite(uint32 spell)
{
    uint32 first = sSpellMgr->GetFirstSpellInChain(spell);
    return first == 689 || first == 172 || first == 686 || first == 1752 || first == 2643;
}

class MysticPlayer final : public PlayerScript
{
public:
    MysticPlayer() : PlayerScript("FreePickMysticPlayer", {PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_AFTER_SPEC_SLOT_CHANGED, PLAYERHOOK_ON_PLAYER_RESURRECT, PLAYERHOOK_ON_LEVEL_CHANGED,
        PLAYERHOOK_ON_UPDATE, PLAYERHOOK_ON_LOGOUT, PLAYERHOOK_ON_LEARN_SPELL,
        PLAYERHOOK_ON_FORGOT_SPELL}) { }
    void OnPlayerLearnSpell(Player* player, uint32 spell) override
    {
        if (spell == 33831 || spell == 6117 || IsRangedEnchantPrerequisite(spell) ||
            (sSpellMgr->GetFirstSpellInChain(spell) == 14914 || sSpellMgr->GetFirstSpellInChain(spell) == 8921))
            Synchronize(player);
        if (Applies(player) && std::find(AscensionFreePick::SpecializationSpells.begin(),
            AscensionFreePick::SpecializationSpells.end(), spell) != AscensionFreePick::SpecializationSpells.end())
            SendPresets(player);
    }
    void OnPlayerForgotSpell(Player* player, uint32 spell) override
    {
        if (spell == 33831 || spell == 6117 || IsRangedEnchantPrerequisite(spell) ||
            (sSpellMgr->GetFirstSpellInChain(spell) == 14914 || sSpellMgr->GetFirstSpellInChain(spell) == 8921))
            Synchronize(player);
    }
    void OnPlayerLogin(Player* player) override
    {
        Load(player);
        RefreshArea52MarksmanTalentHelpers(player);
        if (Applies(player))
        {
            SendScrollProgress(player);
            SendSpecLinks(player);
        }
    }
    void OnPlayerUpdate(Player* player, uint32) override
    {
        ProcessScrollRequests(player);
        ClearInvalidDefiasBuffs(player);
        RefreshArea52MarksmanTalentHelpers(player);
    }
    void OnPlayerLogout(Player* player) override
    {
        std::lock_guard<std::mutex> lock(ScrollRequestLock);
        ScrollRequests.erase(player->GetGUID());
    }
    void OnPlayerAfterSpecSlotChanged(Player* player, std::uint8_t) override
    {
        if (Applies(player))
        {
            Synchronize(player);
            SendSlots(player);
        }
    }
    void OnPlayerResurrect(Player* player, float, bool&) override { Synchronize(player); }
    void OnPlayerLevelChanged(Player* player, std::uint8_t) override { Synchronize(player); }
};

class MysticCommands final : public CommandScript
{
public:
    MysticCommands() : CommandScript("FreePickMysticCommands") { }
    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable const subcommands = {
            {"audit", Audit, SEC_ADMINISTRATOR, Console::Yes},
            {"grant", Grant, SEC_ADMINISTRATOR, Console::Yes},
            {"equip", AdminEquip, SEC_ADMINISTRATOR, Console::Yes},
            {"refresh", Refresh, SEC_ADMINISTRATOR, Console::No}};
        static ChatCommandTable const commands = {{"mystic", subcommands}};
        return commands;
    }
    static bool Audit(ChatHandler* handler, std::string path)
    {
        if (handler->GetSession())
            return false;
        bool const success = ExportAudit(Data, Connected, path);
        handler->SendSysMessage(success ? "Mystic audit exported." : "Mystic audit export failed.");
        return success;
    }
    static bool Refresh(ChatHandler* handler)
    {
        Load(handler->GetPlayer());
        return true;
    }
    static bool AdminEquip(ChatHandler* handler, std::uint32_t slot, std::uint32_t spell, Optional<Tail> name)
    {
        Player* player = name ? ObjectAccessor::FindPlayerByName(std::string(*name)) : handler->GetPlayer();
        if (!Applies(player) || !slot || slot > 17)
            return false;
        auto wanted = ReadSlots(player);
        wanted[slot - 1] = spell;
        bool const success = Equip(player, wanted, true);
        handler->SendSysMessage(success ? "Mystic Enchant equipped." : "Mystic Enchant selection rejected.");
        return success;
    }
    static bool Grant(ChatHandler* handler, std::uint32_t spell, Optional<Tail> name)
    {
        Player* player = name ? ObjectAccessor::FindPlayerByName(std::string(*name)) : handler->GetPlayer();
        if (!player)
            return false;
        auto* state = player->CustomData.Get<State>(StateKey);
        if (!CanChange(player, state) || (spell && !Connected.contains(spell)))
            return false;
        WorldSession* session = player->GetSession();
        auto const guid = player->GetGUID();
        auto const generation = state->Generation;
        auto transaction = CharacterDatabase.BeginTransaction();
        for (auto const& [id, stacks] : Connected)
            if (!spell || id == spell)
            {
                auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_INS_FREEPICK_MYSTIC_COLLECTION);
                statement->SetData(0, session->GetAccountId());
                statement->SetData(1, id);
                transaction->Append(statement);
            }
        state->Busy = true;
        session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(transaction)).AfterComplete(
            [session, guid, generation](bool success)
            {
                Player* owner = session->GetPlayer();
                if (!owner || owner->GetGUID() != guid)
                    return;
                auto* current = owner->CustomData.Get<State>(StateKey);
                if (!current || current->Generation != generation)
                    return;
                current->Busy = false;
                if (success)
                    Load(owner);
                ChatHandler(session).SendSysMessage(success ? "Mystic collection updated." : "Mystic grant failed.");
            });
        return true;
    }
};
}

#include "Area52PrestigeEnchantReset.inc"

bool GrantsActive(Player const* player, std::uint32_t enchant)
{
    if (!Applies(player) || !Connected.contains(enchant) || (enchant == 978724 && !player->HasSpell(2565)))
        return false;
    auto const slots = ReadSlots(player);
    if (std::find(slots.begin(), slots.end(), enchant) == slots.end())
        return false;
    auto const& entry = Data.BySpell.at(enchant);
    return InvestmentMet(player, entry, false) && InvestmentMet(player, entry, true);
}

void SetGrantedSpell(Player* player, std::uint32_t enchant, std::uint32_t spell, bool enabled, bool announce)
{
    if (!player || !player->GetSession() || player->GetSession()->PlayerLogout() || !Applies(player))
        return;
    std::string const source = "core.freepick.mystic.grant." + std::to_string(spell);
    auto const* existing = player->FindPlayerSettings(source);
    std::vector<std::uint32_t> owners;
    if (existing)
        for (std::size_t i = 1; i < existing->size(); ++i)
            if ((*existing)[i].value && (*existing)[i].value != enchant)
                owners.push_back((*existing)[i].value);
    if (enabled)
        owners.push_back(enchant);
    if (!existing && owners.empty())
        return;
    auto const it = player->GetSpellMap().find(spell);
    bool const temporary = it != player->GetSpellMap().end() && it->second->State == PLAYERSPELL_TEMPORARY;
    bool const preserve = existing && !existing->empty() ? (*existing)[0].value == 2 : player->HasSpell(spell) && !temporary;
    std::size_t const oldSize = existing ? existing->size() : 0;
    player->UpdatePlayerSetting(source, 0, preserve ? 2 : 1);
    for (std::size_t i = 0; i < std::max(oldSize, owners.size() + 1) - 1; ++i)
        player->UpdatePlayerSetting(source, uint32(i + 1), i < owners.size() ? owners[i] : 0);
    if (!owners.empty())
    {
        if (temporary)
            it->second->State = PLAYERSPELL_NEW;
        else if (!player->HasSpell(spell))
        {
            std::set<std::uint32_t> const allowed = {spell};
            AscensionFreePick::SpellLearningScope scope(player, allowed);
            player->learnSpell(spell, false, false, announce);
            auto const learned = player->GetSpellMap().find(spell);
            if (learned != player->GetSpellMap().end() && player->HasSpell(spell))
                learned->second->State = PLAYERSPELL_NEW;
        }
    }
    else if (!preserve)
        player->removeSpell(spell, SPEC_MASK_ALL, false);
}

bool DraftEquipmentReady(Player* player)
{
    auto const* state = player ? player->CustomData.Get<State>(StateKey) : nullptr;
    return state && CanChange(player, state) && !player->IsNonMeleeSpellCast(false);
}

bool CanApplyDraftEnchants(Player* player, std::vector<Area52Build::Enchant> const& enchants)
{
    if (!DraftEquipmentReady(player))
        return false;
    Slots slots{};
    std::set<std::uint32_t> owned;
    std::size_t next = 0;
    std::uint32_t worldforged = 0;
    for (auto const& enchant : enchants)
    {
        if (!enchant.Stacks || enchant.Stacks > slots.size() - next)
            return false;
        owned.insert(enchant.Id);
        for (std::uint32_t i = 0; i < enchant.Stacks; ++i)
            slots[next++] = enchant.Id;
    }
    if (!ArrangeSlots(Data, slots) || !ValidateSlots(Data, slots, player->GetLevel(), owned, Connected))
        return false;
    for (auto spell : slots)
        if (spell)
        {
            auto const& entry = Data.BySpell.at(spell);
            worldforged += entry.Worldforged && entry.Quality == 3;
        }
    if (worldforged > 3)
        return false;
    for (std::size_t i = 0; i < slots.size(); ++i)
        for (std::size_t j = i + 1; j < slots.size(); ++j)
            if (slots[i] && slots[j] && slots[i] != slots[j] &&
                sSpellMgr->CheckSpellGroupStackRules(sSpellMgr->GetSpellInfo(slots[i]),
                    sSpellMgr->GetSpellInfo(slots[j])) != SPELL_GROUP_STACK_RULE_DEFAULT)
                return false;
    return true;
}

bool ApplyDraftEnchants(Player* player, std::vector<Area52Build::Enchant> const& enchants)
{
    auto* state = player ? player->CustomData.Get<State>(StateKey) : nullptr;
    if (!state || !CanChange(player, state))
        return false;
    Slots wanted{};
    std::size_t slot = 0;
    std::set<std::uint32_t> entitlement;
    for (auto const& enchant : enchants)
    {
        if (!enchant.Stacks || enchant.Stacks > wanted.size() - slot)
            return false;
        entitlement.insert(enchant.Id);
        for (std::uint32_t i = 0; i < enchant.Stacks; ++i)
            wanted[slot++] = enchant.Id;
    }
    if (!ArrangeSlots(Data, wanted) || !ValidateSlots(Data, wanted, player->GetLevel(), entitlement, Connected))
        return false;
    auto const previous = state->DraftEntitlement;
    state->DraftEntitlement = std::move(entitlement);
    if (!Equip(player, wanted, true))
    {
        state->DraftEntitlement = previous;
        return false;
    }
    state->DraftResolved = true;
    Synchronize(player);
    return true;
}

bool ClearDraftEnchants(Player* player)
{
    auto* state = player ? player->CustomData.Get<State>(StateKey) : nullptr;
    if (!state || !CanChange(player, state) || !Equip(player, Slots{}, true))
        return false;
    state->DraftResolved = true;
    state->DraftEntitlement.clear();
    Synchronize(player);
    return true;
}

bool ReleaseDraftEnchants(Player* player, std::uint64_t build, std::uint32_t revision)
{
    auto* state = player ? player->CustomData.Get<State>(StateKey) : nullptr;
    if (!state || !CanChange(player, state) || !build || !revision)
        return false;

    // The character-wide rewarded flag can refer to a different build or revision.
    auto claim = CharacterDatabase.Query(
        "SELECT EXISTS(SELECT 1 FROM area52_draft_reward WHERE guid={} AND account={} AND build_id={} AND revision={})",
        player->GetGUID().GetCounter(), player->GetSession()->GetAccountId(), build, revision);
    if (!claim)
        return false;
    if (!claim->Fetch()[0].Get<std::uint32_t>())
        return ClearDraftEnchants(player);

    // Ownership must already be loaded before releasing the temporary entitlement.
    for (auto spell : ReadSlots(player))
        if (spell && !state->Known.contains(spell))
            return false;
    state->DraftEntitlement.clear();
    Synchronize(player);
    return true;
}
bool GrantDraftReward(Player* player, std::uint64_t build, std::uint32_t revision,
    std::vector<Area52Build::Enchant> const& enchants)
{
    auto* state = player ? player->CustomData.Get<State>(StateKey) : nullptr;
    if (!state || !CanChange(player, state) || !build || !revision)
        return false;
    std::set<std::uint32_t> spells;
    for (auto const& enchant : enchants)
    {
        if (!Data.BySpell.contains(enchant.Id))
            return false;
        spells.insert(enchant.Id);
    }
    if (!Area52DraftPetRewardReady(player, build, revision))
        return false;
    Area52DraftAppearanceRewards appearances;
    if (!PrepareArea52DraftVanityReward(player, build, revision, appearances))
        return false;
    auto const guid = player->GetGUID();
    auto const account = player->GetSession()->GetAccountId();
    auto const generation = state->Generation;
    auto* session = player->GetSession();
    auto transaction = CharacterDatabase.BeginTransaction();
    auto* claim = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_DRAFT_REWARD_GUARDED);
    claim->SetData(0, guid.GetCounter());
    claim->SetData(1, account);
    claim->SetData(2, build);
    claim->SetData(3, revision);
    auto const petContext = Area52Build::GetDraftVanityContext(player);
    claim->SetData(4, Area52Build::FeaturedHunterPet(petContext.Pet, petContext.Archetype));
    claim->SetData(5, guid.GetCounter());
    claim->SetData(6, build);
    claim->SetData(7, revision);
    transaction->Append(claim);
    auto trophy = std::make_shared<std::unique_ptr<Item>>(Item::CreateItem(1297306, 1, player));
    if (!*trophy)
        return false;
    uint32 mailId = sObjectMgr->GenerateMailID();
    uint32 itemGuid = (*trophy)->GetGUID().GetCounter();
    uint32 now = GameTime::GetGameTime().count();
    transaction->Append(
        "INSERT INTO area52_build_trophy_claim (account, build_id, completion_guid, completion_revision, mail_id) "
        "SELECT account, build_id, guid, revision, {} FROM area52_draft_reward "
        "WHERE guid={} AND account={} AND build_id={} AND revision={} "
        "ON DUPLICATE KEY UPDATE build_id=area52_build_trophy_claim.build_id",
        mailId, guid.GetCounter(), account, build, revision);
    // Item serialization and conditional delivery share the guarded completion transaction.
    (*trophy)->SaveToDB(transaction);
    transaction->Append("INSERT INTO mail (id,messageType,stationery,mailTemplateId,sender,receiver,subject,body,"
        "has_items,expire_time,deliver_time,money,cod,checked) SELECT mail_id,0,{},0,0,completion_guid,"
        "'Featured Draft completion','Your first completion of this Featured Draft earned one Build Master trophy.',"
        "1,{},{},0,0,0 FROM area52_build_trophy_claim WHERE account={} AND build_id={} AND mail_id={}",
        uint32(MAIL_STATIONERY_GM), now + 90 * DAY, now, account, build, mailId);
    transaction->Append("INSERT INTO mail_items (mail_id,item_guid,receiver) SELECT mail_id,{},completion_guid "
        "FROM area52_build_trophy_claim WHERE account={} AND build_id={} AND mail_id={}", itemGuid, account, build, mailId);
    transaction->Append("DELETE FROM item_instance WHERE guid={} AND NOT EXISTS "
        "(SELECT 1 FROM area52_build_trophy_claim WHERE account={} AND build_id={} AND mail_id={})",
        itemGuid, account, build, mailId);
    for (auto spell : spells)
    {
        auto* grant = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_DRAFT_REWARD_ENCHANT);
        grant->SetData(0, account);
        grant->SetData(1, spell);
        grant->SetData(2, guid.GetCounter());
        grant->SetData(3, account);
        grant->SetData(4, build);
        grant->SetData(5, revision);
        transaction->Append(grant);
    }
    auto* petReward = CharacterDatabase.GetPreparedStatement(CHAR_UPD_A52_DRAFT_PET_REWARD);
    petReward->SetData(0, guid.GetCounter());
    petReward->SetData(1, build);
    petReward->SetData(2, revision);
    petReward->SetData(3, account);
    transaction->Append(petReward);
    for (auto const& [appearance, item] : appearances)
    {
        auto* grant = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_DRAFT_REWARD_APPEARANCE);
        grant->SetData(0, account);
        grant->SetData(1, appearance);
        grant->SetData(2, item);
        grant->SetData(3, guid.GetCounter());
        grant->SetData(4, account);
        grant->SetData(5, build);
        grant->SetData(6, revision);
        transaction->Append(grant);
    }
    state->Busy = true;
    session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(transaction)).AfterComplete(
        [session, guid, account, generation, build, revision, spells, appearances, mailId, trophy, now](bool success)
        {
            auto* owner = session->GetPlayer();
            auto* current = owner && owner->GetGUID() == guid ? owner->CustomData.Get<State>(StateKey) : nullptr;
            if (!current || current->Generation != generation)
                return;
            if (!success)
            {
                current->Busy = false;
                return;
            }
            if (CharacterDatabase.Query("SELECT 1 FROM area52_build_trophy_claim WHERE mail_id={}", mailId))
            {
                sMailMgr->OnMailSent(guid.GetCounter());
                auto letter = std::make_unique<Mail>();
                letter->messageID = mailId;
                letter->messageType = MAIL_NORMAL;
                letter->stationery = MAIL_STATIONERY_GM;
                letter->receiver = guid.GetCounter();
                letter->subject = "Featured Draft completion";
                letter->body = "Your first completion of this Featured Draft earned one Build Master trophy.";
                letter->deliver_time = now;
                letter->expire_time = now + 90 * DAY;
                letter->state = MAIL_STATE_UNCHANGED;
                letter->AddItem((*trophy)->GetGUID().GetCounter(), (*trophy)->GetEntry());
                owner->AddNewMailDeliverTime(now);
                if (!owner->GetMail(mailId))
                    owner->AddMail(letter.release());
                if (!owner->GetMItem((*trophy)->GetGUID().GetCounter()))
                    owner->AddMItem(trophy->release());
            }
            auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_DRAFT_REWARD);
            query->SetData(0, guid.GetCounter());
            query->SetData(1, account);
            session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
                [session, guid, generation, build, revision, spells, appearances](PreparedQueryResult result)
                {
                    auto* player = session->GetPlayer();
                    auto* state = player && player->GetGUID() == guid
                        ? player->CustomData.Get<State>(StateKey) : nullptr;
                    if (!state || state->Generation != generation)
                        return;
                    state->Busy = false;
                    if (!result)
                        return;
                    if (result->Fetch()[0].Get<std::uint64_t>() == build
                        && result->Fetch()[1].Get<std::uint32_t>() == revision)
                    {
                        state->Known.insert(spells.begin(), spells.end());
                        SendKnown(player, *state);
                        ConfirmArea52DraftVanityReward(player, appearances);
                        ConfirmArea52DraftPetReward(player, build, revision);
                    }
                    player->UpdatePlayerSetting("core.area52.draft_progress", 1, 1);
                    player->SaveToDB(false, false);
                }));
        });
    return true;
}

bool CollectionLoaded(Player* player)
{
    auto const* state = player ? player->CustomData.Get<State>(StateKey) : nullptr;
    return state && state->Loaded;
}

void RefreshEquipped(Player* player)
{
    Synchronize(player);
}

float NonstackingCostCorrection(Unit const* caster, std::uint32_t schoolMask)
{
    if (!Active)
        return 0.0f;
    uint32 const primarySchool = 1u << GetFirstSchoolInMask(SpellSchoolMask(schoolMask));
    AuraEffect const* athletic = caster->GetAuraEffect(84561, EFFECT_0);
    AuraEffect const* shadows = caster->GetAuraEffect(84562, EFFECT_0);
    auto matches = [](AuraEffect const* effect, uint32 mask)
    {
        return effect && effect->GetAuraType() == SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT &&
            effect->GetAmount() < 0 && (effect->GetMiscValue() & mask);
    };
    if (!matches(athletic, primarySchool) && !matches(shadows, schoolMask))
        return 0.0f;

    int32 sum = 0;
    int32 strongest = 0;
    for (AuraEffect const* effect : caster->GetAuraEffectsByType(SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT))
    {
        if (!Data.BySpell.contains(effect->GetId()))
            continue;
        // The normal cost field already contains modifiers for the primary school.
        if (matches(effect, primarySchool))
        {
            sum += effect->GetAmount();
            strongest = std::min(strongest, effect->GetAmount());
        }
        else if (effect == shadows && matches(effect, schoolMask))
            strongest = std::min(strongest, effect->GetAmount());
    }
    return float(strongest - sum) / 100.0f;
}
}

#include "Area52EpicDruid.inc"
#include "Area52Everliving.inc"

namespace Area52SeedOfLife
{
constexpr char StateKey[] = "area52.seed-of-life.bloom";
constexpr uint32 Seed = 944827;
constexpr uint32 Enchant = 965937;
constexpr uint32 Heal = 966753;
constexpr float Radius = 40.0f;

struct State : DataMap::Base
{
    TaskScheduler Scheduler;
};

struct Chain
{
    ObjectGuid Caster;
    uint32 Map;
    uint32 Instance;
    uint32 Phase;
    std::set<ObjectGuid> Visited;
};

bool Eligible(Player* caster, Unit* target, Chain const& chain)
{
    return target && target->IsInWorld() && target->IsAlive() && caster->InSamePhase(target) &&
        caster->IsValidAssistTarget(target) && target->HasAura(Seed, caster->GetGUID()) &&
        !chain.Visited.count(target->GetGUID());
}

std::list<Unit*> Targets(Player* caster, Unit* center, Chain const& chain)
{
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck check(center, Radius);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> search(center, targets, check);
    Cell::VisitObjects(center, search, Radius);
    if (std::find(targets.begin(), targets.end(), center) == targets.end())
        targets.push_back(center);
    targets.remove_if([&](Unit* target)
    {
        return !Eligible(caster, target, chain) || !center->IsWithinDistInMap(target, Radius) ||
            !center->IsWithinLOSInMap(target);
    });
    targets.sort([center](Unit* left, Unit* right)
    {
        float a = center->GetDistance(left);
        float b = center->GetDistance(right);
        return a == b ? left->GetGUID() < right->GetGUID() : a < b;
    });
    return targets;
}

void Bloom(Player* caster, Unit* target, std::shared_ptr<Chain> chain)
{
    if (!Eligible(caster, target, *chain))
        return;
    ObjectGuid center = target->GetGUID();
    chain->Visited.insert(center);
    target->RemoveAurasDueToSpell(Seed, caster->GetGUID());
    caster->CastSpell(target, Heal, TRIGGERED_FULL_MASK);
    caster->CustomData.GetDefault<State>(StateKey)->Scheduler.Schedule(Milliseconds(500),
        [chain, center](TaskContext)
    {
        Player* owner = ObjectAccessor::FindPlayer(chain->Caster);
        if (!owner || !owner->IsInWorld() || !owner->IsAlive() || !owner->FindMap() ||
            !AscensionMysticEnchant::Applies(owner) || !owner->HasAura(Enchant) ||
            owner->GetMapId() != chain->Map || owner->GetInstanceId() != chain->Instance ||
            owner->GetPhaseMask() != chain->Phase)
            return;
        Unit* origin = ObjectAccessor::GetUnit(*owner, center);
        if (!origin || !origin->IsInWorld() || !origin->IsAlive() || !owner->InSamePhase(origin))
            return;
        auto targets = Targets(owner, origin, *chain);
        uint32 count = 0;
        for (Unit* next : targets)
        {
            if (count++ == 3)
                break;
            Bloom(owner, next, chain);
        }
    });
}
}

class spell_area52_bloom : public SpellScript
{
    PrepareSpellScript(spell_area52_bloom);

    bool Load() override
    {
        Player* player = GetCaster()->ToPlayer();
        return player && AscensionMysticEnchant::Applies(player);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({944827, 965937, 966753});
    }

    SpellCastResult Check()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (!caster || !caster->HasAura(965937))
            return SPELL_FAILED_CASTER_AURASTATE;
        Area52SeedOfLife::Chain chain{};
        return Area52SeedOfLife::Targets(caster, caster, chain).empty() ?
            SPELL_FAILED_BAD_TARGETS : SPELL_CAST_OK;
    }

    void Suppress(SpellEffIndex effect)
    {
        PreventHitDefaultEffect(effect);
    }

    void Start(SpellEffIndex effect)
    {
        PreventHitDefaultEffect(effect);
        Player* caster = GetCaster()->ToPlayer();
        if (!caster || !caster->IsInWorld() || !caster->FindMap() || !caster->HasAura(965937))
            return;
        auto chain = std::make_shared<Area52SeedOfLife::Chain>();
        chain->Caster = caster->GetGUID();
        chain->Map = caster->GetMapId();
        chain->Instance = caster->GetInstanceId();
        chain->Phase = caster->GetPhaseMask();
        auto targets = Area52SeedOfLife::Targets(caster, caster, *chain);
        if (!targets.empty())
            Area52SeedOfLife::Bloom(caster, targets.front(), chain);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_bloom::Check);
        OnEffectLaunch += SpellEffectFn(spell_area52_bloom::Suppress, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_bloom::Suppress, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectHitTarget += SpellEffectFn(spell_area52_bloom::Start, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_area52_bloom_heal : public SpellScript
{
    PrepareSpellScript(spell_area52_bloom_heal);

    bool Load() override
    {
        Player* player = GetCaster()->ToPlayer();
        return player && AscensionMysticEnchant::Applies(player);
    }

    void Amount(SpellEffIndex)
    {
        SetEffectValue(6 * GetCaster()->GetLevel());
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_bloom_heal::Amount, EFFECT_1, SPELL_EFFECT_HEAL);
    }
};

class area52_bloom_lifecycle : public PlayerScript
{
public:
    area52_bloom_lifecycle() : PlayerScript("area52_bloom_lifecycle",
        {PLAYERHOOK_ON_UPDATE, PLAYERHOOK_ON_LOGOUT}) { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (auto* state = player->CustomData.Get<Area52SeedOfLife::State>(Area52SeedOfLife::StateKey))
        {
            if (!player->IsInWorld() || !player->IsAlive() || !player->HasAura(965937))
                state->Scheduler.CancelAll();
            else
                state->Scheduler.Update(diff);
        }
    }

    void OnPlayerLogout(Player* player) override
    {
        player->CustomData.Erase(Area52SeedOfLife::StateKey);
    }
};

class area52_dark_arrows : public AllSpellScript
{
public:
    area52_dark_arrows() : AllSpellScript("area52_dark_arrows",
        {ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    // Preserve eligibility from launch through missile travel.
    enum : uint32 { OldBoltCharge = 0xDA520001, Spent = 0xDA520002 };

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player) || !player->HasAura(997115) || spell->IsTriggered())
            return;
        bool bolt = player->MatchesSpellInteraction(info->Id, 686);
        if (!bolt && !player->MatchesSpellInteraction(info->Id, 27243))
            return;
        Aura* old = player->GetAura(997412, player->GetGUID());
        if (bolt && old && old->GetCharges())
        {
            spell->SetScriptValue(OldBoltCharge, 1);
            old->ModCharges(-1);
        }
        if (Aura* arrows = player->AddAura(997412, player))
            arrows->SetCharges(6);
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32, uint32, bool) override
    {
        Player* player = spell->GetCaster()->ToPlayer();
        if (!AscensionFreePick::Applies(player) || !player->HasAura(997115) || spell->IsTriggered() ||
            !target || miss != SPELL_MISS_NONE || spell->GetScriptValue(Spent))
            return;
        uint32 id = spell->GetSpellInfo()->Id;
        if (!spell->GetScriptValue(OldBoltCharge))
        {
            if (player->MatchesSpellInteraction(id, 686) || player->MatchesSpellInteraction(id, 27243))
                return;
            SpellInfo const* rapid = sSpellMgr->GetSpellInfo(3045);
            auto isShot = [rapid](SpellInfo const* info)
            {
                return rapid && info && info->SpellFamilyName == SPELLFAMILY_HUNTER &&
                    bool(info->SpellFamilyFlags & rapid->Effects[EFFECT_1].SpellClassMask);
            };
            bool eligible = id == 997517 || id == 997526 || isShot(spell->GetSpellInfo());
            if (!eligible)
                for (uint32 origin : player->GetSpellInteractionOrigins(id))
                    if (origin == 997517 || origin == 997526 || isShot(sSpellMgr->GetSpellInfo(origin)))
                    {
                        eligible = true;
                        break;
                    }
            Aura* arrows = player->GetAura(997412, player->GetGUID());
            if (!eligible || !arrows || !arrows->GetCharges())
                return;
            arrows->ModCharges(-1);
        }
        spell->SetScriptValue(Spent, 1);
        player->CastSpell(target, 997742, true);
        player->CastSpell(player, 997539, true);
    }
};
class spell_area52_dark_ranger_shadow_arrow : public SpellScript
{
    PrepareSpellScript(spell_area52_dark_ranger_shadow_arrow);

    void Landed(SpellEffIndex)
    {
        Player* caster = GetCaster()->ToPlayer();
        if (!AscensionFreePick::Applies(caster) || !caster->HasAura(997115) ||
            !GetHitUnit())
            return;

        Aura* nightfall = caster->GetAura(18095, caster->GetGUID());
        SpellProcEntry const* proc = sSpellMgr->GetSpellProcEntry(18095);
        auto now = GameTime::SteadyNow();
        if (!nightfall || !proc || nightfall->IsProcOnCooldown(now))
            return;
        nightfall->AddProcCooldown(proc, now);
        caster->CastSpell(caster, 17941, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_dark_ranger_shadow_arrow::Landed, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};
class spell_area52_dark_ranger_artillery : public SpellScript
{
    PrepareSpellScript(spell_area52_dark_ranger_artillery);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetCaster()->ToPlayer());
    }

    SpellCastResult Check()
    {
        Unit* caster = GetCaster();
        Aura* shells = caster->GetAura(997539, caster->GetGUID());
        return caster->HasAura(997115) && shells && shells->GetStackAmount() >= 3
            ? SPELL_CAST_OK : SPELL_FAILED_CASTER_AURASTATE;
    }

    void Launched()
    {
        Unit* caster = GetCaster();
        if (Aura* shells = caster->GetAura(997539, caster->GetGUID()))
            shells->ModStackAmount(-3);
        caster->EnergizeBySpell(caster, GetSpellInfo()->Id, CalculatePct(caster->GetCreateMana(), 35), POWER_MANA);
    }

    void CycleDarkfall()
    {
        Unit* caster = GetCaster();
        Aura* previous = caster->GetAura(984509, caster->GetGUID());
        if (previous)
            if (AuraEffect* bonus = previous->GetEffect(EFFECT_0))
                GetSpell()->SetScriptValue(AscensionFreePick::DarkfallDamageKey, bonus->GetAmount());
        if (previous && previous->GetStackAmount() >= 3)
            previous->Remove();
        else
            caster->AddAura(984509, caster);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_dark_ranger_artillery::Check);
        OnCast += SpellCastFn(spell_area52_dark_ranger_artillery::Launched);
        OnCast += SpellCastFn(spell_area52_dark_ranger_artillery::CycleDarkfall);
    }
};

class spell_area52_darkfall_periodic : public SpellScript
{
    PrepareSpellScript(spell_area52_darkfall_periodic);

    void SnapshotTick()
    {
        int32 const bonus = GetSpellValue()->EffectBasePoints[EFFECT_2] - GetSpellInfo()->Effects[EFFECT_2].CalcBaseValue(0);
        if (!AscensionFreePick::Applies(GetCaster()->ToPlayer()) || bonus <= 0)
            return;
        if (Aura* aura = GetHitAura())
            if (AuraEffect* tick = aura->GetEffect(EFFECT_0))
                tick->SetAmount(tick->GetAmount() + CalculatePct(tick->GetAmount(), bonus));
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_area52_darkfall_periodic::SnapshotTick);
    }
};

#include "SpellAuraEffects.h"

class aura_area52_dark_arrow_mark : public AuraScript
{
    PrepareAuraScript(aura_area52_dark_arrow_mark);
    void Removed(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* caster = GetCaster();
        Unit* victim = GetTarget();
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH &&
            caster && AscensionFreePick::Applies(caster->ToPlayer()) && caster->HasAura(81279))
            caster->CastSpell(victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ(), 81280, true);
    }
    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_dark_arrow_mark::Removed, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_bone_arrow_charges : public AuraScript
{
    PrepareAuraScript(aura_area52_bone_arrow_charges);
    void Charge(AuraEffect const*, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* owner = GetTarget();
        if (!AscensionFreePick::Applies(owner->ToPlayer()) || event.GetActor() != owner ||
            !event.GetDamageInfo() || !event.GetDamageInfo()->GetDamage())
            return;
        Aura* old = owner->GetAura(982745, owner->GetGUID());
        uint8 next = old ? std::min<uint32>(3, old->GetCharges() + 1) : 1;
        if (Aura* charge = owner->AddAura(982745, owner))
            charge->SetCharges(next);
    }
    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(aura_area52_bone_arrow_charges::Charge, EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_bone_arrow : public SpellScript
{
    PrepareSpellScript(spell_area52_bone_arrow);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    SpellCastResult Check()
    {
        Unit* caster = GetCaster();
        Aura* charges = caster->GetAura(982745, caster->GetGUID());
        return caster->HasAura(982743) && charges && charges->GetCharges() ? SPELL_CAST_OK : SPELL_FAILED_CASTER_AURASTATE;
    }
    void Spend()
    {
        if (Aura* charges = GetCaster()->GetAura(982745, GetCaster()->GetGUID()))
            charges->ModCharges(-1);
    }
    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_bone_arrow::Check);
        OnCast += SpellCastFn(spell_area52_bone_arrow::Spend);
    }
};

class area52_bone_arrow_rapid_killing : public AllSpellScript
{
public:
    area52_bone_arrow_rapid_killing() : AllSpellScript("area52_bone_arrow_rapid_killing", {ALLSPELLHOOK_ON_HIT_RESULT}) { }
    void OnSpellHitResult(Spell* spell, Unit*, uint8 miss, uint32 damage, uint32, bool) override
    {
        Player* player = spell->GetCaster()->ToPlayer();
        if (miss != SPELL_MISS_NONE || !damage || !AscensionFreePick::Applies(player) ||
            !player->HasAura(982743) || sSpellMgr->GetFirstSpellInChain(spell->GetSpellInfo()->Id) != 982744 ||
            player->HasSpellCooldown(901080))
            return;
        for (uint32 talent : {834949u, 34949u, 34948u})
            if (player->GetAura(talent, player->GetGUID()))
            {
                SpellInfo const* buff = sSpellMgr->GetSpellInfo(talent == 834949 ? 835099 : talent == 34949 ? 35099 : 35098);
                if (!buff)
                    return;
                CustomSpellValues values;
                values.AddSpellMod(SPELLVALUE_BASE_POINT0, buff->Effects[EFFECT_0].CalcValue(player) / 2);
                values.AddSpellMod(SPELLVALUE_AURA_DURATION, buff->GetDuration() / 2);
                if (player->CastCustomSpell(901090, values, player, TRIGGERED_FULL_MASK) == SPELL_CAST_OK)
                    player->AddSpellCooldown(901080, 0, 10 * IN_MILLISECONDS);
                return;
            }
    }
};


class spell_area52_dark_ranger_auto_shot : public SpellScript
{
    PrepareSpellScript(spell_area52_dark_ranger_auto_shot);

    void StartAutoShot()
    {
        Player* player = GetCaster()->ToPlayer();
        Unit* target = GetExplTargetUnit();
        uint32 const enchant = sSpellMgr->GetFirstSpellInChain(GetSpellInfo()->Id) == 982744 ? 982743 : 997115;
        if (!AscensionFreePick::Applies(player) || !player->HasAura(enchant) ||
            !player->HasSpell(75) || !target || !player->IsValidAttackTarget(target))
            return;
        Spell* current = player->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
        if (current && current->GetSpellInfo()->Id == 75 &&
            current->m_targets.GetUnitTargetGUID() == target->GetGUID())
            return;
        player->CastSpell(target, 75, false);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_dark_ranger_auto_shot::StartAutoShot);
    }
};

namespace
{
bool DefiasWeaponReady(Player* player)
{
    if (!player || !AscensionFreePick::Applies(player) || !player->HasAura(997047) ||
        player->GetShapeshiftForm() != FORM_NONE ||
        player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
        return false;
    Item const* main = player->GetWeaponForAttack(BASE_ATTACK, true);
    if (!main)
        return false;
    uint32 type = main->GetTemplate()->InventoryType;
    return type == INVTYPE_WEAPON || type == INVTYPE_WEAPONMAINHAND;
}

bool IsDefiasPistol(Player* player, uint32 id)
{
    return player->MatchesSpellInteraction(id, 997048) || (id >= 998100 && id <= 998110);
}

bool IsDefiasGrape(Player* player, uint32 id)
{
    return player->MatchesSpellInteraction(id, 997052) || id == 300191 || (id >= 300147 && id <= 300153);
}

constexpr uint32 DefiasLandedKey = 997049;
class area52_defias_combat final : public AllSpellScript
{
public:
    area52_defias_combat() : AllSpellScript("area52_defias_combat",
        {ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (spell->IsTriggered() || !DefiasWeaponReady(player) ||
            (!IsDefiasPistol(player, info->Id) && !IsDefiasGrape(player, info->Id)))
            return;
        SpellInfo const* helper = sSpellMgr->GetSpellInfo(997416);
        if (!helper)
            return;
        for (SpellEffIndex i : {EFFECT_0, EFFECT_1})
        {
            SpellEffectInfo const& effect = helper->Effects[i];
            int32 delta = effect.CalcValue(player);
            if (delta >= 0)
                continue;
            for (auto const& entry : player->GetSpellMap())
                if (player->HasSpellCooldown(entry.first) &&
                    player->MatchesSpellInteraction(entry.first, effect.MiscValue))
                    player->ModifySpellCooldown(entry.first, -int32(std::min<uint32>(uint32(-delta), player->GetSpellCooldownDelay(entry.first))));
        }
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32, uint32, bool) override
    {
        Player* player = spell->GetCaster()->ToPlayer();
        if (spell->IsTriggered() || miss != SPELL_MISS_NONE || !target ||
            spell->GetScriptValue(DefiasLandedKey) || !DefiasWeaponReady(player) || target != spell->m_targets.GetUnitTarget())
            return;
        uint32 id = spell->GetSpellInfo()->Id;
        if (IsDefiasPistol(player, id))
        {
            spell->SetScriptValue(DefiasLandedKey, 1);
            player->CastSpell(player, 997050, true);
            player->CastSpell(player, 997054, true);
        }
        else if (player->MatchesSpellInteraction(id, 1495) && player->GetAura(997054, player->GetGUID()))
        {
            spell->SetScriptValue(DefiasLandedKey, 1);
            player->RemoveAurasDueToSpell(997054, player->GetGUID());
            player->CastSpell(target, 997430, true);
        }
    }
};
}


namespace
{
void ClearInvalidDefiasBuffs(Player* player)
{
    if (!AscensionFreePick::Applies(player))
        return;
    auto guid = player->GetGUID();
    if ((!player->GetAura(997050, guid) && !player->GetAura(997054, guid)) || DefiasWeaponReady(player))
        return;
    player->RemoveAurasDueToSpell(997050, guid);
    player->RemoveAurasDueToSpell(997054, guid);
}
}



class spell_area52_defias_weapon final : public SpellScript
{
    PrepareSpellScript(spell_area52_defias_weapon);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    SpellCastResult Check()
    {
        return DefiasWeaponReady(GetCaster()->ToPlayer()) ? SPELL_CAST_OK : SPELL_FAILED_CASTER_AURASTATE;
    }
    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_defias_weapon::Check);
    }
};

std::int32_t AscensionMysticEnchant::DefiasGlobalCooldownDelta(Player* player, std::uint32_t spell)
{
    if (!DefiasWeaponReady(player) || !player->MatchesSpellInteraction(spell, 1495))
        return 0;
    SpellInfo const* tactics = sSpellMgr->GetSpellInfo(997051);
    return tactics ? tactics->Effects[EFFECT_1].CalcValue(player) : 0;
}
bool AscensionMysticEnchant::DefiasPistolProcEligible(Player* player, std::uint32_t spellId)
{
    return DefiasWeaponReady(player) && IsDefiasPistol(player, spellId);
}
bool AscensionMysticEnchant::DefiasGrapeShotEligible(Player* player, std::uint32_t spellId)
{
    return DefiasWeaponReady(player) && IsDefiasGrape(player, spellId);
}

#include "CellImpl.h"
#include "GridNotifiersImpl.h"
class aura_area52_frenzied_cleave : public AuraScript
{
    PrepareAuraScript(aura_area52_frenzied_cleave);

    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({954584}); }

    bool Load() override
    {
        return AscensionFreePick::Applies(GetCaster() ? GetCaster()->ToPlayer() : nullptr);
    }

    void Cleave(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* pet = GetTarget();
        DamageInfo* hit = event.GetDamageInfo();
        if (!hit || !hit->GetDamage() || hit->GetSpellInfo() || event.GetActor() != pet ||
            !(event.GetTypeMask() & PROC_FLAG_DONE_MELEE_AUTO_ATTACK) ||
            !GetCaster() || GetCaster()->GetGuardianPet() != pet)
            return;

        std::list<Unit*> nearby;
        Acore::AnyUnfriendlyNoTotemUnitInObjectRangeCheck check(pet, pet, NOMINAL_MELEE_RANGE);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyNoTotemUnitInObjectRangeCheck> search(pet, nearby, check);
        Cell::VisitObjects(pet, search, NOMINAL_MELEE_RANGE);
        nearby.remove_if([&](Unit* unit)
        {
            return unit == hit->GetVictim() || !pet->IsValidAttackTarget(unit) || !pet->IsWithinLOSInMap(unit);
        });
        Acore::Containers::RandomResize(nearby, 5);
        std::vector<ObjectGuid> targets;
        for (Unit* unit : nearby)
            targets.push_back(unit->GetGUID());

        CustomSpellValues values;
        values.AddSpellMod(SPELLVALUE_BASE_POINT0, hit->GetUnmitigatedDamage());
        values.AddSpellMod(SPELLVALUE_FORCED_CRIT_RESULT, int32(event.GetHitMask() & PROC_EX_CRITICAL_HIT));
        for (ObjectGuid guid : targets)
            if (Unit* unit = ObjectAccessor::GetUnit(*pet, guid))
                pet->CastCustomSpell(954584, values, unit, TRIGGERED_FULL_MASK, nullptr, effect);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(aura_area52_frenzied_cleave::Cleave, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_void_eruption : public SpellScript
{
    PrepareSpellScript(spell_area52_void_eruption);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({589, 760185, 760186});
    }

    bool Load() override
    {
        return AscensionFreePick::Applies(GetCaster()->ToPlayer());
    }

    void Landed(SpellEffIndex)
    {
        Player* caster = GetCaster()->ToPlayer();
        Unit* enemy = GetHitUnit();
        if (!enemy || !caster->IsValidAttackTarget(enemy))
            return;

        // The default launch effect has already granted this cast's personal buff.
        if (Aura* buff = caster->GetAura(760185, caster->GetGUID()))
        {
            int32 extension = sSpellMgr->GetSpellInfo(760186)->Effects[EFFECT_0].CalcValue();
            buff->SetDuration(buff->GetDuration() + extension);
        }

        // The original tooltip exposes Worgen Form only with the Wildcard rune.
        if (caster->GetShapeshiftForm() != FORM_SHADOW &&
            !(caster->HasSpell(973827) && caster->GetShapeshiftForm() == FORM_CAT &&
              (caster->HasAura(275737) || caster->HasAura(982807) || caster->HasAura(1575492))))
            return;

        for (uint32 rank = sSpellMgr->GetLastSpellInChain(589); rank; rank = sSpellMgr->GetPrevSpellInChain(rank))
            if (caster->HasActiveSpell(rank))
            {
                // Apply to the already-hit target, preserving aura immunities and caster ownership.
                caster->AddAura(rank, enemy);
                break;
            }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_void_eruption::Landed, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

void AddSC_AscensionMysticEnchant()
{
    if (!Area52MysticRealm::UsesFreePickEngine())
        return;
    new area52_defias_combat();
    RegisterSpellScript(spell_area52_defias_weapon);
    RegisterSpellScript(aura_area52_dark_arrow_mark);
    RegisterSpellScript(aura_area52_bone_arrow_charges);
    RegisterSpellScript(spell_area52_dark_ranger_auto_shot);
    RegisterSpellScript(aura_area52_frenzied_cleave);
    RegisterSpellScript(spell_area52_void_eruption);
    RegisterSpellScript(spell_area52_bone_arrow);
    new area52_bone_arrow_rapid_killing();
    new area52_dark_arrows();
    RegisterSpellScript(spell_area52_dark_ranger_shadow_arrow);
    RegisterSpellScript(spell_area52_dark_ranger_artillery);
    RegisterSpellScript(spell_area52_darkfall_periodic);
    RegisterSpellScript(spell_area52_everliving_swiftmend);
    RegisterSpellScript(aura_area52_everliving);
    RegisterSpellScript(aura_area52_everliving_stacks);
    RegisterSpellScript(spell_area52_evergreen);
    RegisterSpellScript(spell_area52_call_fungarians);
    RegisterSpellScript(aura_area52_cloud_of_rot);
    RegisterSpellScript(spell_area52_bloom);
    RegisterSpellScript(spell_area52_bloom_heal);
    new area52_bloom_lifecycle();
    RegisterSpellScript(spell_area52_hoplite_support_proc);
    RegisterSpellScript(aura_area52_sweeping_swings);
    RegisterSpellScript(spell_area52_sweeping_swings_damage);
    new area52_relentless_finishers();
    new area52_hoplite_remaining_metadata();
    RegisterSpellScript(spell_area52_hunger_for_blood);
    RegisterSpellScript(aura_area52_wind_slam);
    RegisterSpellScript(spell_area52_wind_slam_damage);
    RegisterSpellScript(aura_area52_draconic_sinister);
    RegisterSpellScript(spell_area52_shattering_throw);
    RegisterSpellScript(spell_area52_dependency_proc);
    RegisterSpellScript(spell_area52_deep_freeze);
    RegisterSpellScript(spell_area52_mana_barrier);
    RegisterSpellScript(aura_area52_missing_mana_surge);
    RegisterSpellScript(spell_area52_siegebreaker);
    RegisterSpellScript(aura_area52_siegebreaker_collision);
    new area52_dependency_damage();
    new area52_dependency_metadata();
    new area52_improved_sprint();
    new area52_dependency_casts();
    RegisterSpellScript(spell_area52_corrupted_bear_instant);
    new area52_corrupted_bear_casts();
    RegisterSpellScript(spell_area52_corrupted_bear);
    RegisterSpellScript(spell_area52_corrupted_bear_form);
    RegisterSpellScript(spell_area52_felfire_equipment);
    RegisterSpellScript(spell_area52_pure_chaos_bolt);
    new area52_felfire_player();
    new area52_felfire_casts();
    RegisterSpellScript(spell_area52_felfire_emberstorm);
    RegisterSpellScript(spell_area52_overloaded_mind);
    RegisterSpellScript(spell_area52_froststorm_echo);
    RegisterSpellScript(spell_area52_legendary_batch);
    RegisterSpellScript(spell_area52_slinger_consumption);
    RegisterSpellScript(spell_area52_bone_chill_vulnerability);
    new area52_legendary_batch_metadata();
    RegisterSpellScript(spell_area52_battlemage);
    RegisterSpellScript(spell_area52_battlemage_damage);
    RegisterSpellScript(spell_area52_compound_power);
    RegisterSpellScript(spell_area52_compound_power_slice_and_dice);
    RegisterSpellScript(spell_area52_sniper);
    RegisterSpellScript(spell_area52_compound_shot);
    RegisterSpellScript(aura_area52_compound_shot_recast);
    RegisterSpellScript(spell_area52_combo_shot_volley);
    RegisterSpellScript(spell_area52_syndicate_sniper_training);
    RegisterSpellScript(spell_area52_syndicate_capstone);
    RegisterSpellScript(spell_area52_syndicate_sniper_shot);
    RegisterSpellScript(aura_area52_dragon_charge);
    RegisterSpellScript(spell_area52_dragon_charge_impact);
    RegisterSpellScript(spell_area52_dragon_roar_stun);
    new area52_unstoppable_force();
    new area52_pr37_metadata();
    RegisterSpellScript(spell_area52_skull_crusher_cooldown);
    RegisterSpellScript(aura_area52_archetype_cleanup);
    RegisterSpellScript(spell_area52_legendary_proc);
    RegisterSpellScript(spell_area52_unbounded_deathbringer);
    new area52_legendary_metadata();
    new area52_cast_movement_metadata();
    RegisterSpellScript(spell_area52_purification_hit);
    RegisterSpellScript(aura_area52_purification_ground);
    RegisterSpellScript(spell_area52_zealot_hit);
    RegisterSpellScript(spell_area52_advancement_proc);
    RegisterSpellScript(spell_area52_followup_proc);
    new area52_followup_casts();
    RegisterSpellScript(spell_area52_flame_throwing);
    RegisterSpellScript(spell_area52_shared_might);
    using namespace AscensionMysticEnchant;
    AscensionCompatOpcodes::Claim(0x60C, Handle);
    AscensionCompatOpcodes::Claim(0x739, Handle);
    AscensionCompatOpcodes::Claim(0x73A, Handle);
    AscensionCompatOpcodes::Claim(0x60F, Handle);
    AscensionCompatOpcodes::Claim(0x617, Handle);
    RegisterAscensionClientConfig(ClientConfig);
    new MysticWorld();
    new MysticPlayer();
    new MysticCommands();
    new item_area52_unidentified_mystic();
    new item_area52_identified_mystic();
    AscensionCompatOpcodes::Claim(0x611, QueueScrollRequest);
    AscensionCompatOpcodes::Claim(0x603, QueueScrollRequest);
    AscensionCompatOpcodes::Claim(0x607, QueueScrollRequest);
    AscensionCompatOpcodes::Claim(0x733, QueueScrollRequest);
    new area52_mystic_scroll_loot();
    RegisterSpellScript(spell_freepick_ancient_altar_scroll);
}

void RestoreArea52MysticSpellsForLogin(Player* player, PreparedQueryResult result)
{
    using namespace AscensionMysticEnchant;
    if (!Applies(player))
        return;
    std::set<std::uint32_t> known;
    if (result)
        do
        {
            known.insert(result->Fetch()[0].Get<std::uint32_t>());
        } while (result->NextRow());
    auto const slots = ReadSlots(player);
    AddTestAccess(player, known);
    if (!ValidateSlots(Data, slots, player->GetLevel(), known, Connected))
        return;
    for (auto const& grant : SpellGrants)
        if (std::find(slots.begin(), slots.end(), grant.Enchant) != slots.end())
        {
            auto const& entry = Data.BySpell.at(grant.Enchant);
            if (InvestmentMet(player, entry, false) && InvestmentMet(player, entry, true) &&
                (grant.Enchant != 978724 || player->HasSpell(2565)) &&
                ((grant.Enchant != 977831 && grant.Enchant != 982330 && grant.Enchant != 997047) || player->HasSpell(grant.Requires)) &&
            (grant.Enchant != 997220 || player->HasActiveSpell(grant.Requires)) &&
                (grant.Enchant != 81248 || player->HasActiveSpell(33831)) &&
                GrantRequirementMet(player, grant.Requires))
                SetGrantedSpell(player, grant.Enchant, grant.Spell, true);
        }
    constexpr std::array<std::uint32_t, 6> spellGrantingEnchants = {81116, 92580, 926573, 954081, 978217, 983240};
    for (auto enchant : spellGrantingEnchants)
    {
        if (std::find(slots.begin(), slots.end(), enchant) == slots.end())
            continue;
        auto const& entry = Data.BySpell.at(enchant);
        if (InvestmentMet(player, entry, false) && InvestmentMet(player, entry, true))
            player->AddAura(enchant, player);
    }
}

void Area52Worldforged::RefreshAccess()
{
    AscensionMysticEnchant::LoadConnections();
    AscensionMysticEnchant::RefreshTestAccess();
}

bool Area52Worldforged::OwnerReady(Player* player)
{
    auto state = player ? player->CustomData.Get<AscensionMysticEnchant::State>(AscensionMysticEnchant::StateKey) : nullptr;
    return state && state->Loaded;
}
