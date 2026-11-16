/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AscensionFreePick.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "Unit.h"

namespace
{
enum PerforatingShotsSpells : uint32
{
    SPELL_PIERCING_SHOTS_R1 = 53234,
    SPELL_PIERCING_SHOTS_R2 = 53237,
    SPELL_PIERCING_SHOTS_R3 = 53238,
    SPELL_PERFORATING_SHOTS_R1 = 853236,
    SPELL_PERFORATING_SHOTS_R2 = 853237,
    SPELL_PERFORATING_SHOTS_R3 = 853238
};
}

class aura_ascension_perforating_shots : public AuraScript
{
    PrepareAuraScript(aura_ascension_perforating_shots);

    void CalculateReduction(AuraEffect const*, int32& amount, bool& canBeRecalculated)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->ToPlayer() || !AscensionFreePick::Applies(caster->ToPlayer()))
            return;

        canBeRecalculated = false;
        amount = 0;
        for (uint32 talent : { SPELL_PIERCING_SHOTS_R3, SPELL_PIERCING_SHOTS_R2, SPELL_PIERCING_SHOTS_R1 })
        {
            AuraEffect const* rank = caster->GetAuraEffect(talent, EFFECT_0);
            if (!rank)
                continue;
            amount = rank->GetSpellInfo()->Effects[EFFECT_1].CalcValue(caster);
            return;
        }
    }

    void HandleApply(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        for (uint32 talent : { SPELL_PERFORATING_SHOTS_R3, SPELL_PERFORATING_SHOTS_R2, SPELL_PERFORATING_SHOTS_R1 })
        {
            AuraEffect const* rank = caster->GetAuraEffect(talent, EFFECT_0);
            if (!rank)
                continue;
            if (uint32 armorTear = rank->GetSpellInfo()->Effects[EFFECT_0].TriggerSpell)
                caster->CastSpell(GetTarget(), armorTear, true);
            return;
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_ascension_perforating_shots::CalculateReduction,
            EFFECT_1, SPELL_AURA_ASCENSION_MOD_ABSORB_AMOUNT_PCT);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_ascension_perforating_shots::CalculateReduction,
            EFFECT_2, SPELL_AURA_MOD_HOT_PCT);
        AfterEffectApply += AuraEffectApplyFn(aura_ascension_perforating_shots::HandleApply, EFFECT_0,
            SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

void AddSC_AscensionPerforatingShots()
{
    RegisterSpellScript(aura_ascension_perforating_shots);
}
