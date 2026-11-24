/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "GameTime.h"
#include "AscensionFreePick.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Spell.h"
#include "Unit.h"
#include <algorithm>
#include <iterator>
#include <utility>

namespace
{
enum TalentDataProcSpells : uint32
{
    SPELL_ECLIPSE_SOLAR = 48517,
    SPELL_ECLIPSE_LUNAR = 48518,
    SPELL_REVITALIZE_ENERGIZE = 48542
};

constexpr Milliseconds ECLIPSE_COOLDOWN = 30s;
constexpr uint32 REVITALIZE_DIRECT_CHANCE[] = {33, 67, 100};

constexpr uint32 DRUID_HEALING_TOUCH = 0x20;
constexpr uint32 DRUID_REGROWTH = 0x40;
constexpr uint32 DRUID_REJUVENATION = 0x10;
constexpr uint32 DRUID_WILD_GROWTH = 0x4000000;
constexpr uint32 DRUID_EFFLORESCENCE = 0x10000000;

int32 PctOfMaxMana(Unit const* unit, uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    return info ? CalculatePct(int32(unit->GetMaxPower(POWER_MANA)), info->Effects[EFFECT_0].CalcValue()) : 0;
}

class aura_ascension_eclipse : public AuraScript
{
    PrepareAuraScript(aura_ascension_eclipse);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({SPELL_ECLIPSE_SOLAR, SPELL_ECLIPSE_LUNAR});
    }

    bool InEclipse() const
    {
        return GetTarget()->HasAura(SPELL_ECLIPSE_SOLAR) || GetTarget()->HasAura(SPELL_ECLIPSE_LUNAR);
    }

    bool Ready(ProcEventInfo& event, SpellSchoolMask school, Milliseconds readyAt) const
    {
        if (AscensionFreePick::Applies(GetTarget()->ToPlayer()))
        {
            auto const* spell = event.GetSpellInfo();
            uint32 const required = school == SPELL_SCHOOL_MASK_ARCANE ? 2912 : 5176;
            if (!spell || spell->GetFirstRankSpell()->Id != required)
                return false;
        }
        return (event.GetSchoolMask() & school) && GameTime::GetGameTimeMS() >= readyAt && !InEclipse();
    }

    bool CheckSolar(AuraEffect const*, ProcEventInfo& event)
    {
        return Ready(event, SPELL_SCHOOL_MASK_ARCANE, _solarReady);
    }

    bool CheckLunar(AuraEffect const*, ProcEventInfo& event)
    {
        return Ready(event, SPELL_SCHOOL_MASK_NATURE, _lunarReady);
    }

    void Enter(AuraEffect const* effect, uint32 spellId, Milliseconds& readyAt)
    {
        PreventDefaultAction();
        if (InEclipse())
            return;
        readyAt = GameTime::GetGameTimeMS() + ECLIPSE_COOLDOWN;
        GetTarget()->CastSpell(GetTarget(), spellId, true, nullptr, effect);
    }

    void Solar(AuraEffect const* effect, ProcEventInfo&)
    {
        Enter(effect, SPELL_ECLIPSE_SOLAR, _solarReady);
    }

    void Lunar(AuraEffect const* effect, ProcEventInfo&)
    {
        Enter(effect, SPELL_ECLIPSE_LUNAR, _lunarReady);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(aura_ascension_eclipse::CheckSolar, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL);
        DoCheckEffectProc += AuraCheckEffectProcFn(aura_ascension_eclipse::CheckLunar, EFFECT_1,
            SPELL_AURA_PROC_TRIGGER_SPELL);
        OnEffectProc += AuraEffectProcFn(aura_ascension_eclipse::Solar, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        OnEffectProc += AuraEffectProcFn(aura_ascension_eclipse::Lunar, EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL);
    }

    Milliseconds _solarReady = 0ms;
    Milliseconds _lunarReady = 0ms;
};

class aura_area52_new_moon : public AuraScript
{
    PrepareAuraScript(aura_area52_new_moon);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({8921, 760198, 983846, 983848});
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        auto* caster = GetTarget()->ToPlayer();
        auto* target = event.GetActionTarget();
        auto* damage = event.GetDamageInfo();
        auto* info = event.GetSpellInfo();
        if (!AscensionFreePick::Applies(caster) || !target || event.GetActor() != caster ||
            !info || !damage || !damage->GetDamage() || info->Id == 983848)
            return;
        bool const moonfire = caster->MatchesSpellInteraction(info->Id, 8921);
        if (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC)
        {
            if (moonfire)
                caster->CastSpell(caster, 760198, true);
            return;
        }
        auto* cast = event.GetProcSpell();
        if (moonfire && (!cast || target == cast->m_targets.GetUnitTarget()))
        {
            Aura* waxing = caster->GetAura(983846, caster->GetGUID());
            if (waxing && waxing->GetStackAmount() >= 5)
            {
                waxing->Remove();
                caster->CastCustomSpell(983848, SPELLVALUE_MAX_TARGETS, 6, target, true);
            }
        }
        if (!cast || cast->GetCastTime() <= 0)
            return;
        bool extended = false;
        for (AuraEffect const* effect : target->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE))
        {
            if (effect->GetCasterGUID() != caster->GetGUID() ||
                !caster->MatchesSpellInteraction(effect->GetId(), 8921))
                continue;
            Aura* dot = effect->GetBase();
            int32 duration = dot->GetDuration();
            if (duration > 0 && duration <= INT32_MAX - 3000)
            {
                dot->SetMaxDuration(std::max(dot->GetMaxDuration(), duration + 3000));
                dot->SetDuration(duration + 3000);
                extended = true;
            }
        }
        if (extended)
            caster->CastSpell(caster, 983846, true);
    }

    void Register() override { OnProc += AuraProcFn(aura_area52_new_moon::Proc); }
};

class spell_area52_waxing_moon_explosion : public SpellScript
{
    PrepareSpellScript(spell_area52_waxing_moon_explosion);

    void Select(std::list<WorldObject*>& targets)
    {
        auto* primary = GetExplTargetUnit();
        if (!primary || std::find(targets.begin(), targets.end(), primary) == targets.end())
            return;
        targets.remove(primary);
        while (targets.size() > 5)
            targets.pop_back();
        targets.push_front(primary);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_area52_waxing_moon_explosion::Select,
            EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

class aura_area52_total_eclipse_bonus : public AuraScript
{
    PrepareAuraScript(aura_area52_total_eclipse_bonus);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({965282, SPELL_ECLIPSE_SOLAR, SPELL_ECLIPSE_LUNAR, 986116, 986117});
    }

    void Refresh(bool applying)
    {
        Unit* owner = GetTarget();
        if (!AscensionFreePick::Applies(owner->ToPlayer()))
            return;
        for (auto [phase, helper] : {std::pair<uint32,uint32>{SPELL_ECLIPSE_SOLAR, 986117},
                                     {SPELL_ECLIPSE_LUNAR, 986116}})
        {
            uint32 const changed = GetId();
            if (changed != 965282 && changed != phase)
                continue;
            bool const equipped = changed == 965282 ? applying : owner->HasAura(965282);
            bool const active = changed == phase ? applying : owner->HasAura(phase);
            if (equipped && active)
            {
                if (!owner->HasAura(helper, owner->GetGUID()))
                    owner->CastSpell(owner, helper, true);
            }
            else
                owner->RemoveAurasDueToSpell(helper, owner->GetGUID());
        }
    }

    void Gain(AuraEffect const*, AuraEffectHandleModes) { Refresh(true); }
    void Lose(AuraEffect const*, AuraEffectHandleModes) { Refresh(false); }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_total_eclipse_bonus::Gain, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_total_eclipse_bonus::Lose, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_total_eclipse_window : public AuraScript
{
    PrepareAuraScript(aura_area52_total_eclipse_window);

    bool pending = false;

    void Gain(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* owner = GetTarget();
        if (AscensionFreePick::Applies(owner->ToPlayer()) && owner->HasAura(965282))
        {
            pending = true;
            owner->CastSpell(owner, 48391, true); // Owlkin Frenzy
        }
    }

public:
    bool Consume()
    {
        return std::exchange(pending, false);
    }

    void Register() override
    {
        // Solar and Lunar have different effect-0 aura types. Only a real gain arms the window.
        AfterEffectApply += AuraEffectApplyFn(aura_area52_total_eclipse_window::Gain, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_total_eclipse_hit : public SpellScript
{
    PrepareSpellScript(spell_area52_total_eclipse_hit);

    void Hit()
    {
        Player* player = GetCaster()->ToPlayer();
        Unit* target = GetHitUnit();
        if (!AscensionFreePick::Applies(player) || !target || !player->HasAura(965282))
            return;

        uint32 const root = GetSpellInfo()->GetFirstRankSpell()->Id;
        for (uint32 phase : {uint32(SPELL_ECLIPSE_SOLAR), uint32(SPELL_ECLIPSE_LUNAR)})
        {
            bool const solar = phase == SPELL_ECLIPSE_SOLAR;
            if (root != 954800 && root != (solar ? 5176u : 2912u))
                continue;
            Aura* aura = player->GetAura(phase);
            if (!aura)
                continue;
            auto* window = aura->GetScript<aura_area52_total_eclipse_window>("aura_area52_total_eclipse_window");
            if (!window)
                continue;
            uint32 dot = 0;
            for (uint32 rank = solar ? 5570 : 8921; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
                if (player->HasSpell(rank))
                    dot = rank;
            if (dot && window->Consume())
            {
                player->CastSpell(target, dot, true);
                return;
            }
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_area52_total_eclipse_hit::Hit);
    }
};

class aura_ascension_revitalize : public AuraScript
{
    PrepareAuraScript(aura_ascension_revitalize);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({SPELL_REVITALIZE_ENERGIZE});
    }

    bool CheckProc(ProcEventInfo& event)
    {
        SpellInfo const* spell = event.GetSpellInfo();
        if (!spell || spell->SpellFamilyName != SPELLFAMILY_DRUID || !event.GetProcTarget())
            return false;
        flag96 const& flags = spell->SpellFamilyFlags;
        bool const periodic = event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC;
        if (!periodic && (flags[0] & (DRUID_HEALING_TOUCH | DRUID_REGROWTH)))
        {
            uint8 const rank = std::clamp<uint8>(GetSpellInfo()->GetRank(), 1,
                uint8(std::size(REVITALIZE_DIRECT_CHANCE)));
            return roll_chance_i(REVITALIZE_DIRECT_CHANCE[rank - 1]);
        }
        if ((flags[0] & DRUID_REJUVENATION) || (flags[1] & DRUID_WILD_GROWTH) || (flags[2] & DRUID_EFFLORESCENCE))
            return roll_chance_i(GetSpellInfo()->ProcChance);
        return false;
    }

    void Restore(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* target = event.GetProcTarget();
        GetTarget()->CastCustomSpell(SPELL_REVITALIZE_ENERGIZE, SPELLVALUE_BASE_POINT0,
            PctOfMaxMana(target, SPELL_REVITALIZE_ENERGIZE), target, true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_ascension_revitalize::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_ascension_revitalize::Restore, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

}

void AddSC_AscensionTalentDataProcs()
{
    RegisterSpellScript(aura_ascension_eclipse);
    RegisterSpellScript(aura_ascension_revitalize);
    RegisterSpellScript(aura_area52_total_eclipse_bonus);
    RegisterSpellScript(aura_area52_total_eclipse_window);
    RegisterSpellScript(aura_area52_new_moon);
    RegisterSpellScript(spell_area52_waxing_moon_explosion);
    RegisterSpellScript(spell_area52_total_eclipse_hit);
}
