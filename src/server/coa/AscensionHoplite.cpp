/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Config.h"
#include "Player.h"
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

bool Applies(Unit* unit)
{
    Player* player = unit ? unit->ToPlayer() : nullptr;
    if (!Enabled || !player || player->getClass() != CLASS_HERO || !player->HasAura(Hoplite))
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~uint32(0x400));
}

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
    RegisterSpellScript(spell_area52_hoplite_shield_strike);
    RegisterSpellScript(aura_area52_hoplite_flurry);
    RegisterSpellScript(spell_area52_hoplite_flurry_damage);
}
