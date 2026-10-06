/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Config.h"
#include "Player.h"
#include "Item.h"
#include "SpellMgr.h"
#include "WorldSession.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include <algorithm>

namespace
{
bool Enabled = false;
constexpr uint32 Hoplite = 978217;
constexpr uint32 SpearMastery = 901203;
constexpr char ChannelKey[] = "coa.area52.hoplite.flurry";

bool InArea52(Player const* player)
{
    if (!Enabled || !player || player->getClass() != CLASS_HERO)
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~uint32(0x400));
}

bool Applies(Unit* unit)
{
    Player* player = unit ? unit->ToPlayer() : nullptr;
    return InArea52(player) && player->HasAura(Hoplite);
}

void ReplaceChain(Player* player, uint32 root, uint32 replacement, bool enabled)
{
    bool wanted = false;
    for (uint32 original = root; original; original = sSpellMgr->GetNextSpellInChain(original))
        wanted |= enabled && player->HasActiveSpell(original);
    if (wanted && !player->HasSpell(replacement))
        player->learnSpell(replacement, true);
    for (uint32 original = root; original; original = sSpellMgr->GetNextSpellInChain(original))
    {
        if (wanted && player->HasActiveSpell(original))
            player->SetTemporarySpellReplacement(original, replacement);
        else if (player->GetTemporarySpellReplacement(original) == replacement)
            player->SetTemporarySpellReplacement(original, 0);
    }
    if (!wanted)
        player->removeSpell(replacement, SPEC_MASK_ALL, true);
}

void SynchronizeHoplite(Player* player, bool removing = false)
{
    if (!player || !player->GetSession() || player->GetSession()->PlayerLogout())
        return;
    bool active = !removing && Applies(player);
    ReplaceChain(player, 23922, 978533, active);
    ReplaceChain(player, 1752, 978569, active);
    ReplaceChain(player, 78, 982250, active);
    ReplaceChain(player, 954876, 901200, active);
    if (active && player->HasAura(2457))
    {
        if (!player->HasAura(997743))
            player->AddAura(997743, player);
    }
    else
    {
        player->RemoveAurasDueToSpell(997743);
        player->RemoveAurasDueToSpell(978539);
        player->RemoveAurasDueToSpell(978537);
    }
    if (!active)
    {
        player->RemoveAurasDueToSpell(901200);
        player->RemoveAurasDueToSpell(SpearMastery);
    }
}

class aura_area52_hoplite_equipment : public AuraScript
{
    PrepareAuraScript(aura_area52_hoplite_equipment);
    bool Load() override
    {
        Player* player = GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr;
        return InArea52(player);
    }
    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({978533, 978569, 982250, 901200, 997743});
    }
    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        SynchronizeHoplite(GetUnitOwner()->ToPlayer());
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetUnitOwner()->ToPlayer();
        if (player->GetSession()->PlayerLogout())
            return;
        SynchronizeHoplite(player, true);
        Item* main = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* off = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if (main && off && main->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM &&
            off->GetTemplate()->InventoryType == INVTYPE_SHIELD)
            player->AutoUnequipOffhandIfNeed(true);
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_hoplite_equipment::Apply,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_hoplite_equipment::Remove,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class HoplitePlayer final : public PlayerScript
{
public:
    HoplitePlayer() : PlayerScript("Area52HoplitePlayer") { }
    void OnPlayerLearnSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
    void OnPlayerForgotSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
    void Refresh(Player* player, uint32 spell)
    {
        uint32 root = sSpellMgr->GetFirstSpellInChain(spell);
        if (Applies(player) && (root == 23922 || root == 1752 || root == 78 || root == 954876))
            SynchronizeHoplite(player);
    }
    void OnPlayerUpdate(Player* player, uint32) override
    {
        if (Applies(player) && !player->GetSession()->PlayerLogout() &&
            player->HasAura(2457) != player->HasAura(997743))
            SynchronizeHoplite(player);
    }
};

class HopliteWorld final : public WorldScript
{
public:
    HopliteWorld() : WorldScript("Area52HopliteWorld") { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};

struct FlurryState : DataMap::Base
{
    bool Empowered = false;
};

int32 ShieldStrikeDamage(uint8 points, float attackPower)
{
    points = std::min<uint8>(points, 5);
    return int32(18.0f + 221.0f * points + (0.82f + 0.04f * points) * attackPower);
}

class spell_area52_hoplite_requirements : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_requirements);
    bool Load() override { return InArea52(GetCaster()->ToPlayer()); }
    SpellCastResult Check()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!Applies(player) || !player->HasAura(2457))
            return SPELL_FAILED_ONLY_SHAPESHIFT;
        Item* main = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* off = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if (!main || !off || main->GetTemplate()->Class != ITEM_CLASS_WEAPON ||
            main->GetTemplate()->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM ||
            off->GetTemplate()->InventoryType != INVTYPE_SHIELD)
            return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        return SPELL_CAST_OK;
    }
    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_hoplite_requirements::Check);
    }
};

class spell_area52_hoplite_shield_strike : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_shield_strike);
    uint8 Points = 0;
    bool Resolved = false;

    bool Load() override { return Applies(GetCaster()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({2687, SpearMastery}); }
    void Capture() { Points = std::min<uint8>(GetCaster()->GetComboPoints(), 5); }
    void SuppressCooldown(SpellEffIndex effect) { PreventHitDefaultEffect(effect); }
    void Damage(SpellEffIndex)
    {
        SetEffectValue(ShieldStrikeDamage(Points, GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK)));
    }
    void Hit()
    {
        if (Resolved || !Points || !GetHitUnit() || GetHitDamage() <= 0)
            return;
        Resolved = true;
        Player* player = GetCaster()->ToPlayer();
        player->ModifySpellCooldown(2687, -2000 * int32(Points));
        if (roll_chance_i(15))
            player->CastSpell(player, SpearMastery, true);
    }
    void Register() override
    {
        BeforeCast += SpellCastFn(spell_area52_hoplite_shield_strike::Capture);
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_hoplite_shield_strike::SuppressCooldown,
            EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE);
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_hoplite_shield_strike::Damage,
            EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterHit += SpellHitFn(spell_area52_hoplite_shield_strike::Hit);
    }
};

class aura_area52_hoplite_flurry : public AuraScript
{
    PrepareAuraScript(aura_area52_hoplite_flurry);
    bool Load() override { return Applies(GetUnitOwner()); }
    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetUnitOwner()->ToPlayer();
        auto* state = player->CustomData.GetDefault<FlurryState>(ChannelKey);
        state->Empowered = player->HasAura(SpearMastery);
        if (state->Empowered)
            player->RemoveAurasDueToSpell(SpearMastery);
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetUnitOwner()->ToPlayer()->CustomData.Erase(ChannelKey);
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_hoplite_flurry::Apply,
            EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_hoplite_flurry::Remove,
            EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_hoplite_flurry_damage : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_flurry_damage);
    bool Load() override { return Applies(GetCaster()); }
    void Damage(SpellEffIndex)
    {
        Player* player = GetCaster()->ToPlayer();
        auto const* state = player->CustomData.Get<FlurryState>(ChannelKey);
        Unit* target = GetHitUnit();
        if (state && state->Empowered && player->HasAura(901200) && target &&
            target->IsCreature() && !target->GetCharmerOrOwnerPlayerOrPlayerItself())
            SetEffectValue(GetEffectValue() + CalculatePct(GetEffectValue(), 50));
    }
    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_hoplite_flurry_damage::Damage,
            EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
}

void AddAscensionHopliteScripts()
{
    new HopliteWorld();
    new HoplitePlayer();
    RegisterSpellScript(spell_area52_hoplite_requirements);
    RegisterSpellScript(aura_area52_hoplite_equipment);
    RegisterSpellScript(spell_area52_hoplite_shield_strike);
    RegisterSpellScript(aura_area52_hoplite_flurry);
    RegisterSpellScript(spell_area52_hoplite_flurry_damage);
}

bool CanArea52HopliteUseShield(Player const* player)
{
    return InArea52(player) && player->HasAura(Hoplite);
}
