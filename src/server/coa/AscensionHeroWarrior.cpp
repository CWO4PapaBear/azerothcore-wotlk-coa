/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AscensionHeroWarrior.h"
#include "AscensionFreePick.h"
#include "Area52DragonReviewRules.h"
#include "Config.h"
#include "Player.h"
#include "WorldSession.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Unit.h"

#include <array>
#include <algorithm>

namespace
{
enum HeroWarriorSpells : uint32
{
    EnrageRank1 = 12317,
    EnrageRank5 = 13048,
    RagingBlow = 954836,
    RageCharge = 954881,
    Victorious = 965664,
    WreckingCrewEnrage = 57520,
    WreckingCrewEnrageRank1 = 1157520,
    ImprovedBerserkerStanceRank1 = 29759,
    ImprovedBerserkerStanceCapstone = 29761,
    RecklessnessExtension = 986820,
    Bloodsurge = 46916
};

constexpr uint32 COMMANDING_SHOUT_FLAG1 = 0x80;

constexpr uint32 ENRAGE_TRIGGER_FLAGS = 0x10000110;
constexpr float ENRAGE_CAPSTONE_HEALTH_PCT = 50.0f;
constexpr float DANCE_OF_DEATH_HEALTH_PCT = 75.0f;
constexpr uint8 RAGE_CHARGE_LIMIT = 2;
constexpr uint32 RAGE_CHARGE_SPACING_MS = 1500;

constexpr std::array<uint32, 9> RAGE_CHARGE_SPENDERS = { RagingBlow, 286391, 980705, 980708, 980709, 980710, 980711,
    980712, 980713 };

class aura_hero_warrior_enrage : public AuraScript
{
    PrepareAuraScript(aura_hero_warrior_enrage);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (sSpellMgr->GetSpellTwinSource(GetId()) != GetId())
            return (eventInfo.GetTypeMask() & TAKEN_HIT_PROC_FLAG_MASK) && eventInfo.GetDamageInfo() &&
                eventInfo.GetDamageInfo()->GetDamage();

        if (SpellInfo const* spell = eventInfo.GetSpellInfo(); spell &&
            (eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS))
            return spell->SpellFamilyName == SPELLFAMILY_WARRIOR && (spell->SpellFamilyFlags[0] & ENRAGE_TRIGGER_FLAGS);

        if (!(eventInfo.GetTypeMask() & TAKEN_HIT_PROC_FLAG_MASK) || !eventInfo.GetDamageInfo() ||
            !eventInfo.GetDamageInfo()->GetDamage())
            return false;
        Unit* target = GetTarget();
        return sSpellMgr->GetSpellRank(GetId()) == sSpellMgr->GetSpellRank(EnrageRank5) &&
            target->HealthBelowPctDamaged(int32(ENRAGE_CAPSTONE_HEALTH_PCT), eventInfo.GetDamageInfo()->GetDamage());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_warrior_enrage::CheckProc);
    }
};

class aura_hero_warrior_dance_of_death : public AuraScript
{
    PrepareAuraScript(aura_hero_warrior_dance_of_death);

    void HandlePeriodic(AuraEffect const*)
    {
        if (!GetTarget()->HealthBelowPct(int32(DANCE_OF_DEATH_HEALTH_PCT)))
            PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_hero_warrior_dance_of_death::HandlePeriodic, EFFECT_0,
            SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
    }
};

class aura_hero_warrior_commanding_presence : public AuraScript
{
    PrepareAuraScript(aura_hero_warrior_commanding_presence);

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        SpellInfo const* spell = eventInfo.GetSpellInfo();
        if (!spell || !(spell->SpellFamilyFlags[1] & COMMANDING_SHOUT_FLAG1))
            return;
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), aurEff->GetSpellInfo()->Effects[EFFECT_0].TriggerSpell + 1, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(aura_hero_warrior_commanding_presence::HandleProc, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class aura_hero_warrior_improved_victory_rush : public AuraScript
{
    PrepareAuraScript(aura_hero_warrior_improved_victory_rush);

    void HandleProc(ProcEventInfo&)
    {
        if (!GetSpellInfo()->HasAura(SPELL_AURA_PROC_TRIGGER_SPELL))
            GetTarget()->CastSpell(GetTarget(), Victorious, true);
    }

    void Register() override
    {
        OnProc += AuraProcFn(aura_hero_warrior_improved_victory_rush::HandleProc);
    }
};

bool KnowsRageChargeSpender(Player const* player)
{
    for (uint32 spender : RAGE_CHARGE_SPENDERS)
        if (player->HasSpell(spender))
            return true;
    return false;
}

bool IsEnrageEffect(SpellInfo const* spell)
{
    return spell && spell->Dispel == DISPEL_ENRAGE && spell->IsPositive() && spell->Id != WreckingCrewEnrage &&
        spell->Id != WreckingCrewEnrageRank1;
}

bool IsEnraged(Unit const* unit, Aura const* ignore = nullptr)
{
    for (auto const& [id, aurApp] : unit->GetAppliedAuras())
        if (aurApp->GetBase() != ignore && IsEnrageEffect(aurApp->GetBase()->GetSpellInfo()))
            return true;
    return false;
}

class aura_hero_warrior_improved_berserker_stance : public AuraScript
{
    PrepareAuraScript(aura_hero_warrior_improved_berserker_stance);

    void CalculateAmount(AuraEffect const*, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = true;
        if (IsEnraged(GetUnitOwner(), _ignore))
            amount *= 2;
    }

public:
    void Recalculate(Aura const* ignore)
    {
        _ignore = ignore;
        if (AuraEffect* effect = GetEffect(EFFECT_0))
            effect->RecalculateAmount();
        _ignore = nullptr;
    }

private:
    Aura const* _ignore = nullptr;

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_hero_warrior_improved_berserker_stance::CalculateAmount,
            EFFECT_0, SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE);
    }
};

void RecalculateImprovedBerserkerStance(Unit* unit, Aura const* ignore)
{
    for (uint32 rank = ImprovedBerserkerStanceRank1; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
        if (Aura* aura = unit->GetAura(rank))
            if (auto* script = aura->GetScript<aura_hero_warrior_improved_berserker_stance>(
                    "aura_hero_warrior_improved_berserker_stance"))
                script->Recalculate(ignore);
}

class hero_warrior_enrage_effects : public UnitScript
{
public:
    hero_warrior_enrage_effects() : UnitScript("hero_warrior_enrage_effects", true,
        {UNITHOOK_ON_AURA_APPLY, UNITHOOK_ON_AURA_REMOVE}) { }

    void OnAuraApply(Unit* unit, Aura* aura) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (!player || !aura || !IsEnrageEffect(aura->GetSpellInfo()))
            return;

        RecalculateImprovedBerserkerStance(player, nullptr);
        if (!KnowsRageChargeSpender(player))
            return;

        Aura* charges = player->GetAura(RageCharge);
        if (!charges)
        {
            if ((charges = player->AddAura(RageCharge, player)))
                charges->SetCharges(1);
            return;
        }
        if (charges->GetMaxDuration() - charges->GetDuration() < int32(RAGE_CHARGE_SPACING_MS) ||
            charges->GetCharges() >= RAGE_CHARGE_LIMIT)
            return;
        charges->SetCharges(charges->GetCharges() + 1);
        charges->RefreshDuration();
    }

    void OnAuraRemove(Unit* unit, AuraApplication* aurApp, AuraRemoveMode) override
    {
        if (unit && unit->IsPlayer() && aurApp && IsEnrageEffect(aurApp->GetBase()->GetSpellInfo()))
            RecalculateImprovedBerserkerStance(unit, aurApp->GetBase());
    }
};

class spell_hero_warrior_bloodsurge_bloodthirst : public SpellScript
{
    PrepareSpellScript(spell_hero_warrior_bloodsurge_bloodthirst);

    bool _bloodsurgeAtCast = false;

    void Capture()
    {
        _bloodsurgeAtCast = GetCaster()->HasAura(Bloodsurge);
    }

    void RefreshBleed()
    {
        if (!AscensionFreePick::Applies(GetCaster()->ToPlayer()) || !_bloodsurgeAtCast || GetHitDamage() <= 0)
            return;
        if (Unit* target = GetHitUnit())
            if (Aura* bleed = target->GetAura(954815, GetCaster()->GetGUID()))
                bleed->RefreshDuration();
    }

    void HandleAfterCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || !player->HasAura(Bloodsurge))
            return;
        player->RemoveSpellCooldown(GetSpellInfo()->Id, true);
        player->RemoveAurasDueToSpell(Bloodsurge);
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_hero_warrior_bloodsurge_bloodthirst::Capture);
        AfterHit += SpellHitFn(spell_hero_warrior_bloodsurge_bloodthirst::RefreshBleed);
        AfterCast += SpellCastFn(spell_hero_warrior_bloodsurge_bloodthirst::HandleAfterCast);
    }
};

class spell_hero_warrior_spend_rage_charge : public SpellScript
{
    PrepareSpellScript(spell_hero_warrior_spend_rage_charge);

    void HandleAfterCast()
    {
        if (GetSpellInfo()->Id == RagingBlow && GetCaster()->HasAura(ImprovedBerserkerStanceCapstone))
            GetCaster()->CastSpell(GetCaster(), RecklessnessExtension, true);

        Aura* charges = GetCaster()->GetAura(RageCharge);
        if (!charges)
            return;
        if (charges->GetCharges() > 1)
            charges->SetCharges(charges->GetCharges() - 1);
        else
            charges->Remove();
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_hero_warrior_spend_rage_charge::HandleAfterCast);
    }
};
}

constexpr int32 VICTORY_RUSH_STOCK_PERCENT = 45;
constexpr uint32 VICTORY_RUSH = 34428;

class spell_hero_warrior_victory_rush : public SpellScript
{
    PrepareSpellScript(spell_hero_warrior_victory_rush);

    bool Load() override
    {
        return GetSpellInfo()->Id == VICTORY_RUSH;
    }

    void HandleDamage(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        int32 spellPower = caster->SpellBaseDamageBonusDone(SpellSchoolMask(1 << SPELL_SCHOOL_HOLY));
        for (uint8 school = SPELL_SCHOOL_FIRE; school < MAX_SPELL_SCHOOL; ++school)
            spellPower = std::min(spellPower, caster->SpellBaseDamageBonusDone(SpellSchoolMask(1 << school)));
        Unit* target = GetHitUnit();
        uint32 bonus = CalculatePct(uint32(std::max(spellPower, 0)), VICTORY_RUSH_STOCK_PERCENT);
        bonus = caster->SpellDamageBonusDone(target, GetSpellInfo(), bonus, SPELL_DIRECT_DAMAGE, effIndex);
        bonus = target->SpellDamageBonusTaken(caster, GetSpellInfo(), bonus, SPELL_DIRECT_DAMAGE);
        SetHitDamage(GetHitDamage() + int32(bonus));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hero_warrior_victory_rush::HandleDamage, EFFECT_0,
            SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

void ApplyAscensionHeroWarriorContracts(SpellInfo* spellInfo)
{
    switch (spellInfo->Id)
    {
        case VICTORY_RUSH:
            if (!spellInfo->Effects[EFFECT_0].BasePoints)
                spellInfo->Effects[EFFECT_0].BasePoints = VICTORY_RUSH_STOCK_PERCENT - 1;
            break;
        case 12880:
        case 14201:
        case 14202:
        case 14203:
        case 14204:
            if (!spellInfo->Effects[EFFECT_0].Effect &&
                spellInfo->Effects[EFFECT_0].ApplyAuraName == SPELL_AURA_MOD_DAMAGE_PERCENT_DONE)
                spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AURA;
            break;
        case 812316:
        case 812317:
        case 812318:
        case 812319:
            spellInfo->BaseLevel = 0;
            break;
        default:
            break;
    }
}

class area52_dragon_review_metadata final : public GlobalScript
{
public:
    area52_dragon_review_metadata() : GlobalScript("area52_dragon_review_metadata",
        {GLOBALHOOK_ON_LOAD_SPELL_CUSTOM_ATTR}) { }

    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!sConfigMgr->GetOption<bool>("CoA.Enable", true) ||
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live" ||
            (sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) & ~uint32(0x400)))
            return;
        if (info->Id == 46867 || info->Id == 56611 || info->Id == 56612)
            info->ProcFlags &= ~uint32(PROC_FLAG_DONE_MELEE_AUTO_ATTACK);
        if (info->Id == 49189 || info->Id == 50149 || info->Id == 50150)
        {
            info->Attributes |= SPELL_ATTR0_PASSIVE;
            info->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AURA;
            info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
            info->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
            info->Effects[EFFECT_1].Effect = 0;
        }
        if (info->Id == 820503)
            info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_DUMMY;
        if (info->Id == 20502)
            info->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AURA;
        if (info->Id == 954806)
            info->Effects[EFFECT_1].BasePoints = 99;
        if (info->Id == 760096)
        {
            info->Effects[EFFECT_0].BasePoints = 760094;
            info->Effects[EFFECT_0].DieSides = 0;
        }
    }
};

class aura_area52_burning_rush final : public AuraScript
{
    PrepareAuraScript(aura_area52_burning_rush);

    bool Load() override { return AscensionFreePick::Applies(GetUnitOwner()->ToPlayer()); }

    void Drain(AuraEffect const*)
    {
        PreventDefaultAction();
        Unit* target = GetTarget();
        Unit::DealDamage(target, target, Area52DragonReview::BurningRushDrain(target->GetMaxHealth()),
            nullptr, NODAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_burning_rush::Drain, EFFECT_2,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

class spell_area52_grappling_followup final : public SpellScript
{
    PrepareSpellScript(spell_area52_grappling_followup);

    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }

    SpellCastResult Check()
    {
        return GetCaster()->HasAura(760096) && GetCaster()->ToPlayer()->HasSpell(760056) ?
            SPELL_CAST_OK : SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
    }

    void Consume() { GetCaster()->RemoveAurasDueToSpell(760096); }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_grappling_followup::Check);
        AfterCast += SpellCastFn(spell_area52_grappling_followup::Consume);
    }
};

void RefreshArea52TwoHandMastery(Player* player)
{
    if (!player || !AscensionFreePick::Applies(player) || !player->IsInWorld() ||
        (player->GetSession() && player->GetSession()->PlayerLogout()))
        return;
    uint32 wanted = 0;
    if (player->IsAlive() && !player->IsInFeralForm() && player->IsTwoHandUsed() &&
        !player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
        wanted = player->HasAura(829633) && AscensionFreePick::CanLearnSpell(player, 829633, false, false) ? 829634 :
            player->HasAura(29623) && AscensionFreePick::CanLearnSpell(player, 29623, false, false) ? 829623 : 0;
    for (uint32 helper : {829623u, 829634u})
    {
        if (helper != wanted)
        {
            if (!player->HasSpell(helper))
                player->RemoveAurasDueToSpell(helper, player->GetGUID());
        }
        else if (!player->GetAura(helper, player->GetGUID()))
            player->CastSpell(player, helper, true);
    }
}

void RefreshArea52DragonTalentHelpers(Player* player)
{
    if (!player || !AscensionFreePick::Applies(player) || !player->IsInWorld() ||
        (player->GetSession() && player->GetSession()->PlayerLogout()))
        return;
    struct Grant { uint32 Helper; std::array<uint32, 3> Parents; };
    constexpr Grant grants[] = {
        {57518, {46867, 0, 0}}, {57519, {56611, 0, 0}}, {57520, {56612, 0, 0}},
        {29841, {29834, 0, 0}}, {29842, {29838, 0, 0}}, {829839, {829838, 0, 0}},
        {46916, {46913, 46914, 46915}}, {52437, {29723, 29725, 29724}}, {820504, {820503, 0, 0}}
    };
    for (auto const& grant : grants)
    {
        Aura* aura = player->GetAura(grant.Helper, player->GetGUID());
        if (!aura || player->HasSpell(grant.Helper))
            continue;
        bool authorized = false;
        for (uint32 parent : grant.Parents)
            authorized |= parent && AscensionFreePick::IsTalentRank(player, parent) &&
                AscensionFreePick::CanLearnSpell(player, parent, false, false);
        if (auto source = aura->GetTriggeredByAuraSpellInfo(); source &&
            std::find(grant.Parents.begin(), grant.Parents.end(), source->Id) == grant.Parents.end())
            authorized |= player->HasAura(source->Id);
        if (!authorized)
            player->RemoveAurasDueToSpell(grant.Helper, player->GetGUID());
    }
}

void RefreshArea52Necropolis(Player* player)
{
    if (!player || !AscensionFreePick::Applies(player) || !player->IsInWorld() ||
        (player->GetSession() && player->GetSession()->PlayerLogout()))
        return;
    constexpr std::array<uint32, 3> parents = {49189, 50149, 50150};
    constexpr std::array<uint32, 3> helpers = {52284, 52285, 52286};
    uint32 wanted = 0;
    if (player->IsAlive())
        for (std::size_t i = 0; i < parents.size(); ++i)
            if (player->HasAura(parents[i]) &&
                AscensionFreePick::CanLearnSpell(player, parents[i], false, false))
                wanted = helpers[i];
    for (uint32 helper : helpers)
        if (helper == wanted)
        {
            if (!player->GetAura(helper, player->GetGUID()))
                player->CastSpell(player, helper, true);
        }
        else if (!player->HasSpell(helper))
            player->RemoveAurasDueToSpell(helper, player->GetGUID());
}

class area52_two_hand_mastery final : public PlayerScript
{
public:
    area52_two_hand_mastery() : PlayerScript("area52_two_hand_mastery",
        {PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_UPDATE, PLAYERHOOK_ON_EQUIP, PLAYERHOOK_ON_UNEQUIP_ITEM}) { }
    void OnPlayerLogin(Player* player) override
    {
        RefreshArea52TwoHandMastery(player);
        RefreshArea52Necropolis(player);
    }
    void OnPlayerUpdate(Player* player, uint32) override
    {
        RefreshArea52TwoHandMastery(player);
        RefreshArea52DragonTalentHelpers(player);
        RefreshArea52Necropolis(player);
    }
    void OnPlayerEquip(Player* player, Item*, uint8, uint8, bool) override { RefreshArea52TwoHandMastery(player); }
    void OnPlayerUnequip(Player* player, Item*) override { RefreshArea52TwoHandMastery(player); }
};

void AddSC_AscensionHeroWarrior()
{
    new area52_dragon_review_metadata();
    new area52_two_hand_mastery();
    RegisterSpellScript(aura_area52_burning_rush);
    RegisterSpellScript(spell_area52_grappling_followup);
    RegisterSpellScript(aura_hero_warrior_enrage);
    RegisterSpellScript(aura_hero_warrior_dance_of_death);
    RegisterSpellScript(spell_hero_warrior_spend_rage_charge);
    RegisterSpellScript(aura_hero_warrior_commanding_presence);
    RegisterSpellScript(aura_hero_warrior_improved_victory_rush);
    RegisterSpellScript(aura_hero_warrior_improved_berserker_stance);
    RegisterSpellScript(spell_hero_warrior_bloodsurge_bloodthirst);
    RegisterSpellScript(spell_hero_warrior_victory_rush);
    new hero_warrior_enrage_effects();
}
