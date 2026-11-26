/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "DBCStores.h"
#include "AscensionFreePick.h"
#include "Area52WindEquipment.h"
#include "Config.h"
#include "SpellAuras.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Unit.h"

namespace
{
enum HeroShamanSpells : uint32
{
    ImprovedEarthShieldCapstone = 851561,
    ImprovedEarthShieldTidal    = 851562,
    TidalWavesFinalRank         = 51564,
    ElectrocuteDamage           = 272034,
    ElectrocuteIncapacitate     = 272053,
    ElectrocuteIncapacitateLock = 272056,
    ConduitBuff                 = 278599
};

constexpr float ELECTROCUTE_DAMAGE_PER_LEVEL = 2.6f;

constexpr float ELEMENTAL_REACH_YARDS = 20.0f;
constexpr uint32 IMPROVED_EARTH_SHIELD_COOLDOWN_MS = 5000;
constexpr uint32 LIGHTNING_SHIELD_FAMILY_FLAG0 = 1024;
constexpr int32 CONDUIT_MS_PER_LIGHTNING_SHIELD_CHARGE = 1000;
}

class aura_hero_shaman_elemental_reach : public AuraScript
{
    PrepareAuraScript(aura_hero_shaman_elemental_reach);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* target = eventInfo.GetProcTarget();
        return target && target != GetTarget() && !GetTarget()->IsWithinDist(target, ELEMENTAL_REACH_YARDS);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_shaman_elemental_reach::CheckProc);
    }
};

class aura_hero_shaman_enhancing_totems : public AuraScript
{
    PrepareAuraScript(aura_hero_shaman_enhancing_totems);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;
        for (SpellEffectInfo const& effect : spellInfo->Effects)
        {
            if (effect.Effect != SPELL_EFFECT_SUMMON)
                continue;
            SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(effect.MiscValueB);
            if (properties && properties->Slot >= SUMMON_SLOT_TOTEM_FIRE && properties->Slot <= SUMMON_SLOT_TOTEM_AIR)
                return true;
        }
        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_shaman_enhancing_totems::CheckProc);
    }
};

bool Area52DualWieldReady(Player* player)
{
    return player && !player->IsInFeralForm() && player->GetWeaponForAttack(BASE_ATTACK, true) &&
        player->GetWeaponForAttack(OFF_ATTACK, true);
}

bool Area52DualWieldBonus(uint32 id)
{
    return id == 913715 || id == 30816 || id == 30818 || id == 30819;
}

void RefreshArea52DualWield(Player* player)
{
    if (!AscensionFreePick::Applies(player))
        return;
    for (uint32 id : {913715u, 30816u, 30818u, 30819u})
        if (AuraEffect* effect = player->GetAuraEffect(id, EFFECT_0, player->GetGUID()))
            effect->RecalculateAmount(player);
}

class area52_dual_wield_amount final : public UnitScript
{
public:
    area52_dual_wield_amount() : UnitScript("area52_dual_wield_amount", true,
        {UNITHOOK_ON_AFTER_AURA_EFFECT_CALCULATE_AMOUNT}) { }

    void OnAfterAuraEffectCalculateAmount(AuraEffect const* effect, Unit*, int32& amount) override
    {
        if (!Area52DualWieldBonus(effect->GetId()) || effect->GetEffIndex() != EFFECT_0)
            return;
        Unit* owner = effect->GetBase()->GetUnitOwner();
        Player* player = owner ? owner->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player))
            return;
        if (!Area52DualWieldReady(player))
            amount = 0;
        else if (effect->GetId() == 913715)
            amount = 2;
    }
};

class area52_dual_wield_cleanup final : public UnitScript
{
public:
    area52_dual_wield_cleanup() : UnitScript("area52_dual_wield_cleanup", {UNITHOOK_ON_AURA_REMOVE}) { }

    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player) || !application)
            return;
        uint32 const id = application->GetBase()->GetId();
        uint32 const helper = id == 30816 ? 965970 : id == 30818 ? 966150 : id == 30819 ? 966151 :
            id == 16256 ? 16257 : id == 16281 ? 16277 : id == 16282 ? 16278 : 0;
        if (helper)
            player->RemoveAurasDueToSpell(helper, player->GetGUID());
    }
};

class area52_dual_wield_equipment final : public PlayerScript
{
public:
    area52_dual_wield_equipment() : PlayerScript("area52_dual_wield_equipment",
        {PLAYERHOOK_ON_EQUIP, PLAYERHOOK_ON_UNEQUIP_ITEM, PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_AFTER_UPDATE_ATTACK_POWER_AND_DAMAGE}) { }

    void OnPlayerEquip(Player* player, Item*, uint8, uint8, bool) override { RefreshArea52DualWield(player); }
    void OnPlayerUnequip(Player* player, Item*) override { RefreshArea52DualWield(player); }
    void OnPlayerLogin(Player* player) override { RefreshArea52DualWield(player); }
    void OnPlayerAfterUpdateAttackPowerAndDamage(Player* player, float&, float&, float&, float&, bool) override
    {
        RefreshArea52DualWield(player);
    }
};

bool Area52WindMetadataEnabled = false;

class area52_wind_configuration final : public WorldScript
{
public:
    area52_wind_configuration() : WorldScript("area52_wind_configuration") { }
    void OnAfterConfigLoad(bool) override
    {
        Area52WindMetadataEnabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};

void ConfigureArea52DualWield(SpellInfo* info)
{
    if (!Area52WindMetadataEnabled || info->Id != 913715)
        return;
    info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_MOD_CRIT_PCT;
    info->Effects[EFFECT_0].BasePoints = 1;
    info->StackAmount = 1;
}

#include "Area52WindTalents.inc"
#include "Area52Desolate.inc"
#include "Area52WindScaling.h"
#include "Area52WindRager.inc"
#include "Area52Zephyr.inc"
#include "Area52Unrelenting.inc"

class area52_wind_metadata final : public GlobalScript
{
public:
    area52_wind_metadata() : GlobalScript("area52_wind_metadata", {GLOBALHOOK_ON_LOAD_SPELL_CUSTOM_ATTR}) { }
    void OnLoadSpellCustomAttr(SpellInfo* info) override { ConfigureArea52DualWield(info); ConfigureArea52WindTalents(info); ConfigureArea52Zephyr(info); ConfigureArea52Unrelenting(info); ConfigureArea52Desolate(info); ConfigureArea52WindRager(info); }
};

class aura_hero_shaman_dual_wield_specialization : public AuraScript
{
    PrepareAuraScript(aura_hero_shaman_dual_wield_specialization);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damage = eventInfo.GetDamageInfo();
        if (AscensionFreePick::Applies(GetTarget()->ToPlayer()))
            return damage && damage->GetDamage() && damage->GetAttackType() == OFF_ATTACK &&
                !eventInfo.GetSpellInfo() && eventInfo.GetActor() == GetTarget() &&
                Area52DualWieldReady(GetTarget()->ToPlayer());
        return damage && damage->GetAttackType() == OFF_ATTACK;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_shaman_dual_wield_specialization::CheckProc);
    }
};

class spell_hero_shaman_earth_shield_heal : public SpellScript
{
    PrepareSpellScript(spell_hero_shaman_earth_shield_heal);

    void AfterHeal()
    {
        Player* shaman = ObjectAccessor::GetPlayer(*GetCaster(), GetOriginalCaster() ? GetOriginalCaster()->GetGUID() :
            GetCaster()->GetGUID());
        if (!shaman || !shaman->HasAura(ImprovedEarthShieldCapstone) || !shaman->HasSpell(TidalWavesFinalRank) ||
            shaman->HasSpellCooldown(ImprovedEarthShieldTidal))
            return;
        shaman->CastSpell(shaman, ImprovedEarthShieldTidal, true);
        shaman->AddSpellCooldown(ImprovedEarthShieldTidal, 0, IMPROVED_EARTH_SHIELD_COOLDOWN_MS);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_hero_shaman_earth_shield_heal::AfterHeal);
    }
};

class aura_hero_shaman_doused : public AuraScript
{
    PrepareAuraScript(aura_hero_shaman_doused);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({ ElectrocuteDamage, ElectrocuteIncapacitate, ElectrocuteIncapacitateLock });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damage = eventInfo.GetDamageInfo();
        return eventInfo.GetActor() && damage && damage->GetDamage();
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        Unit* attacker = eventInfo.GetActor();
        Unit* target = GetTarget();
        if (!attacker || !target->IsAlive())
            return;

        SpellInfo const* electrocute = sSpellMgr->AssertSpellInfo(ElectrocuteDamage);
        int32 damage = electrocute->Effects[EFFECT_0].CalcValue(attacker) +
            int32(float(attacker->GetLevel()) * ELECTROCUTE_DAMAGE_PER_LEVEL);
        attacker->CastCustomSpell(target, ElectrocuteDamage, &damage, nullptr, nullptr, true, nullptr, aurEff);
        if (target->IsAlive() && !target->HasAura(ElectrocuteIncapacitateLock))
            attacker->CastSpell(target, ElectrocuteIncapacitate, true, nullptr, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_shaman_doused::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_hero_shaman_doused::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_hero_shaman_sundering : public SpellScript
{
    PrepareSpellScript(spell_hero_shaman_sundering);

    void HandleDamage(SpellEffIndex)
    {
        Unit* caster = GetCaster();
        int32 const power = int32(caster->GetTotalAttackPowerValue(BASE_ATTACK)) +
            caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask());
        SetEffectValue(GetEffectValue() + power / 2);
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_hero_shaman_sundering::HandleDamage, EFFECT_0,
            SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_hero_shaman_conduit : public SpellScript
{
    PrepareSpellScript(spell_hero_shaman_conduit);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({ ConduitBuff });
    }

    void ConsumeLightningShield(SpellEffIndex)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;
        Aura* conduit = caster->GetAura(ConduitBuff, caster->GetGUID());
        if (!conduit)
            return;
        AuraEffect* shield = caster->GetAuraEffect(SPELL_AURA_PROC_TRIGGER_SPELL, SPELLFAMILY_SHAMAN,
            LIGHTNING_SHIELD_FAMILY_FLAG0, 0, 0, caster->GetGUID());
        if (!shield)
            return;
        int32 const charges = shield->GetBase()->GetCharges();
        if (charges <= 0)
            return;
        caster->RemoveAura(shield->GetBase()->GetId(), caster->GetGUID());
        int32 const extension = charges * CONDUIT_MS_PER_LIGHTNING_SHIELD_CHARGE;
        conduit->SetMaxDuration(conduit->GetMaxDuration() + extension);
        conduit->SetDuration(conduit->GetDuration() + extension);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hero_shaman_conduit::ConsumeLightningShield, EFFECT_1,
            SPELL_EFFECT_DUMMY);
    }
};

void AddSC_AscensionHeroShaman()
{
    new area52_wind_configuration();
    new area52_wind_metadata();
    new area52_unrelenting_cast();
    new area52_unrelenting_cleanup();
    new area52_zephyr_hits();
    new area52_zephyr_cleanup();
    Area52ZephyrRegistered = true;
    new area52_desolate_hits();
    new area52_desolate_cleanup();
    Area52DesolateRegistered = true;
    new area52_wind_rager_casts();
    new area52_wind_rager_amount();
    new area52_wind_rager_cleanup();
    RegisterSpellScript(aura_area52_wind_rager);
    Area52WindRagerRegistered = true;
    new area52_dual_wield_amount();
    new area52_dual_wield_cleanup();
    new area52_dual_wield_equipment();
    RegisterSpellScript(spell_hero_shaman_sundering);
    RegisterSpellScript(aura_hero_shaman_elemental_reach);
    RegisterSpellScript(aura_hero_shaman_enhancing_totems);
    RegisterSpellScript(aura_hero_shaman_dual_wield_specialization);
    RegisterSpellScript(spell_hero_shaman_conduit);
    RegisterSpellScript(spell_hero_shaman_earth_shield_heal);
    RegisterSpellScript(aura_hero_shaman_doused);
}
