/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AscensionHeroPriest.h"
#include "Area52CrusaderAbilityRules.h"
#include "Area52MysticRealm.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "Unit.h"

namespace
{
enum HeroPriestSpells : uint32
{
    LesserFrozenLightProc = 300023,
    VoidEruptionBuff      = 760185,
    VoidTouchedExtend     = 833215,
    VoidTouchedStack      = 833216
};

constexpr uint32 MIND_BLAST_FLAG0 = 0x00002000;
constexpr uint32 MIND_FLAY_TICK_FLAG0 = 0x00800000;

constexpr float REACH_CAPSTONE_YARDS = 20.0f;
}

class aura_hero_priest_reach_capstone : public AuraScript
{
    PrepareAuraScript(aura_hero_priest_reach_capstone);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* target = eventInfo.GetProcTarget();
        return target && target != GetTarget() && !GetTarget()->IsWithinDist(target, REACH_CAPSTONE_YARDS);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_priest_reach_capstone::CheckProc);
    }
};

class spell_hero_priest_halo_frozen_light : public SpellScript
{
    PrepareSpellScript(spell_hero_priest_halo_frozen_light);

    void HandleFrozenLight(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->HasAura(LesserFrozenLightProc))
            PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_hero_priest_halo_frozen_light::HandleFrozenLight, EFFECT_2,
            SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectLaunchTarget += SpellEffectFn(spell_hero_priest_halo_frozen_light::HandleFrozenLight, EFFECT_2,
            SPELL_EFFECT_TRIGGER_SPELL);
    }
};

class aura_hero_priest_void_touched_mind : public AuraScript
{
    PrepareAuraScript(aura_hero_priest_void_touched_mind);

    static bool IsMindBlast(SpellInfo const* spellInfo)
    {
        return spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && (spellInfo->SpellFamilyFlags[0] & MIND_BLAST_FLAG0);
    }

    static bool IsMindFlay(SpellInfo const* spellInfo)
    {
        return spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && (spellInfo->SpellFamilyFlags[0] & MIND_FLAY_TICK_FLAG0);
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo || !GetTarget()->HasAura(VoidEruptionBuff))
            return false;
        return IsMindBlast(spellInfo) || IsMindFlay(spellInfo);
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* priest = GetTarget();
        priest->CastSpell(priest, IsMindBlast(eventInfo.GetSpellInfo()) ? VoidTouchedExtend : VoidTouchedStack, true,
            nullptr, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_priest_void_touched_mind::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_hero_priest_void_touched_mind::HandleProc, EFFECT_1,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class aura_hero_priest_void_eruption_buff : public AuraScript
{
    PrepareAuraScript(aura_hero_priest_void_eruption_buff);

    void AfterRemove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(VoidTouchedStack);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_hero_priest_void_eruption_buff::AfterRemove, EFFECT_2,
            SPELL_AURA_OVERRIDE_CLASS_SCRIPTS, AURA_EFFECT_HANDLE_REAL);
    }
};

void ApplyArea52CrusaderAbilityContracts(SpellInfo* spellInfo, bool area52)
{
    if (!area52)
        return;
    if (spellInfo->Id == 642)
        spellInfo->Effects[EFFECT_0].MiscValue = SPELL_SCHOOL_MASK_ALL;
    if (Area52CrusaderAbility::IsHaloRecipient(spellInfo->Id))
    {
        spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_HEALING_RECEIVED;
        spellInfo->Effects[EFFECT_1].BasePoints = 7;
    }
    if (Area52CrusaderAbility::IsHaloMissile(spellInfo->Id))
        spellInfo->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(TARGET_UNIT_CASTER_AREA_RAID);
}

void ApplyAscensionHeroPriestContracts(SpellInfo* spellInfo)
{
    ApplyArea52CrusaderAbilityContracts(spellInfo, Area52MysticRealm::UsesFreePickEngine());
    if (spellInfo->Id == 53005 || spellInfo->Id == 53006 || spellInfo->Id == 53007)
        spellInfo->AttributesEx2 |= SPELL_ATTR2_ALLOW_WHILE_NOT_SHAPESHIFTED;
    else if (spellInfo->Id == 48156)
    {
        for (SpellEffectInfo& effect : spellInfo->Effects)
            if (effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE && effect.Amplitude == 10000)
                effect.Amplitude = 1000;
    }
}

#include "Area52CrusaderBlinding.inc"

void AddSC_AscensionHeroPriest()
{
    RegisterSpellScript(spell_area52_crusader_blinding_light);
    RegisterSpellScript(aura_hero_priest_reach_capstone);
    RegisterSpellScript(spell_hero_priest_halo_frozen_light);
    RegisterSpellScript(aura_hero_priest_void_touched_mind);
    RegisterSpellScript(aura_hero_priest_void_eruption_buff);
}
