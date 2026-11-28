/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AscensionHeroPaladin.h"
#include "Area52MysticRealm.h"
#include "AscensionFreePick.h"
#include "Spell.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "DBCStores.h"
#include "Area52ConsecratedGround.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Unit.h"
#include <algorithm>
#include <array>

namespace
{
enum HeroPaladinSpells : uint32
{
    BlindingLight     = 954500,
    FanaticismDebuff  = 31882,
    ShieldOfRighteous = 53600,
    SharedMightAlly   = 920042,
    SharedMightSelf   = 920043,
    ImprovedJudgementsCapstone = 25958,
    Unsealed          = 825958
};

struct ExecutionSentenceRank
{
    uint32 Active;
    uint32 Sentence;
    uint32 SentenceBurst;
    uint32 Stay;
    uint32 StayBurst;
};

constexpr std::array<ExecutionSentenceRank, 7> EXECUTION_SENTENCE_RANKS = { {
    { 954817, 954818, 954827, 954819, 954828 },
    { 954972, 954979, 954992, 954985, 954998 },
    { 954973, 954980, 954993, 954986, 954999 },
    { 954974, 954981, 954994, 954987, 955000 },
    { 954975, 954982, 954995, 954988, 955001 },
    { 954976, 954983, 954996, 954989, 955002 },
    { 954977, 954984, 954997, 954991, 955003 } } };

ExecutionSentenceRank const* FindExecutionSentenceRank(uint32 spellId)
{
    auto const rank = std::find_if(EXECUTION_SENTENCE_RANKS.begin(), EXECUTION_SENTENCE_RANKS.end(),
        [spellId](ExecutionSentenceRank const& candidate)
        {
            return candidate.Active == spellId || candidate.Sentence == spellId || candidate.Stay == spellId;
        });
    return rank == EXECUTION_SENTENCE_RANKS.end() ? nullptr : &*rank;
}

constexpr int32 FANATICISM_PCT = 20;
flag96 const FANATICISM_MASK(0x02800400,
    0x20008800, 0);
}

class aura_hero_paladin_touched_by_the_light : public AuraScript
{
    PrepareAuraScript(aura_hero_paladin_touched_by_the_light);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* healer = eventInfo.GetActor();
        HealInfo* heal = eventInfo.GetHealInfo();
        return healer && healer != GetTarget() && healer->IsPlayer() && heal && heal->GetEffectiveHeal();
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_paladin_touched_by_the_light::CheckProc);
    }
};

class aura_hero_paladin_sanctified_light : public AuraScript
{
    PrepareAuraScript(aura_hero_paladin_sanctified_light);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && spellInfo->Id == BlindingLight;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_paladin_sanctified_light::CheckProc);
    }
};

class aura_hero_paladin_benediction : public AuraScript
{
    PrepareAuraScript(aura_hero_paladin_benediction);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* victim = eventInfo.GetActionTarget();
        Player* player = GetTarget()->ToPlayer();
        return victim && player && player->isHonorOrXPTarget(victim);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_paladin_benediction::CheckProc);
    }
};

class aura_hero_paladin_improved_blessing_of_might : public AuraScript
{
    PrepareAuraScript(aura_hero_paladin_improved_blessing_of_might);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* ally = eventInfo.GetProcTarget();
        return ally && ally != GetTarget();
    }

    void HandleAlly(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* paladin = GetTarget();
        Unit* ally = eventInfo.GetProcTarget();
        if (Unit* previous = ObjectAccessor::GetUnit(*paladin, _ally))
            if (previous != ally)
                previous->RemoveAurasDueToSpell(SharedMightAlly, paladin->GetGUID());
        _ally = ally->GetGUID();
        paladin->CastSpell(ally, SharedMightAlly, true, nullptr, aurEff);
    }

    void HandleSelf(AuraEffect const* aurEff, ProcEventInfo&)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), SharedMightSelf, true, nullptr, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_paladin_improved_blessing_of_might::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_hero_paladin_improved_blessing_of_might::HandleAlly, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL);
        OnEffectProc += AuraEffectProcFn(aura_hero_paladin_improved_blessing_of_might::HandleSelf, EFFECT_1,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }

    ObjectGuid _ally;
};

class spell_hero_paladin_judgement_unsealed : public SpellScript
{
    PrepareSpellScript(spell_hero_paladin_judgement_unsealed);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (caster->HasAura(ImprovedJudgementsCapstone))
            caster->CastSpell(caster, Unsealed, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_hero_paladin_judgement_unsealed::HandleAfterCast);
    }
};

class aura_hero_paladin_fanaticism : public AuraScript
{
    PrepareAuraScript(aura_hero_paladin_fanaticism);

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        if (AscensionFreePick::Applies(GetTarget()->ToPlayer()))
            return;
        if (Unit* target = eventInfo.GetProcTarget())
        {
            int32 pct = FANATICISM_PCT;
            GetTarget()->CastCustomSpell(target, FanaticismDebuff, &pct, nullptr, nullptr, true, nullptr, aurEff);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(aura_hero_paladin_fanaticism::HandleProc, EFFECT_1,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_hero_paladin_shield_of_righteousness : public SpellScript
{
    PrepareSpellScript(spell_hero_paladin_shield_of_righteousness);

    void HandleDamage(SpellEffIndex)
    {
        Unit* caster = GetCaster();
        uint8 const level = caster->GetLevel();
        uint32 const blockValue = caster->GetShieldBlockValue(uint32(float(level) * 29.5f), uint32(float(level) * 34.5f));
        int32 const stockPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
        SetEffectValue(GetEffectValue() + int32(CalculatePct(blockValue, 100 - stockPct)));
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_hero_paladin_shield_of_righteousness::HandleDamage, EFFECT_1,
            SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_hero_paladin_execution_sentence : public SpellScript
{
    PrepareSpellScript(spell_hero_paladin_execution_sentence);

    void HandleDummy(SpellEffIndex)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        ExecutionSentenceRank const* rank = FindExecutionSentenceRank(GetSpellInfo()->Id);
        if (!rank || !target)
            return;
        if (caster->IsFriendlyTo(target))
            caster->CastSpell(target, rank->Stay, true);
        else if (caster->IsValidAttackTarget(target))
            caster->CastSpell(target, rank->Sentence, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hero_paladin_execution_sentence::HandleDummy, EFFECT_0,
            SPELL_EFFECT_DUMMY);
    }
};

class aura_hero_paladin_execution_sentence_burst : public AuraScript
{
    PrepareAuraScript(aura_hero_paladin_execution_sentence_burst);

    void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes)
    {
        AuraRemoveMode const mode = GetTargetApplication()->GetRemoveMode();
        if (mode != AURA_REMOVE_BY_EXPIRE && mode != AURA_REMOVE_BY_ENEMY_SPELL)
            return;
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        ExecutionSentenceRank const* rank = FindExecutionSentenceRank(GetId());
        if (!caster || !rank || !target->IsAlive())
            return;
        caster->CastSpell(target, GetId() == rank->Sentence ? rank->SentenceBurst : rank->StayBurst, true, nullptr,
            aurEff);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_hero_paladin_execution_sentence_burst::HandleRemove, EFFECT_0,
            SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_vindication : public AuraScript
{
    PrepareAuraScript(aura_area52_vindication);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* owner = GetTarget();
        if (!owner || !AscensionFreePick::Applies(owner->ToPlayer()))
            return true;
        SpellInfo const* spell = eventInfo.GetSpellInfo();
        DamageInfo const* damage = eventInfo.GetDamageInfo();
        return eventInfo.GetActor() == owner && eventInfo.GetActionTarget() && spell && damage &&
            damage->GetDamage() && ((spell->GetSchoolMask() & SPELL_SCHOOL_MASK_HOLY) ||
                (spell->SpellFamilyName == SPELLFAMILY_PALADIN && spell->DmgClass == SPELL_DAMAGE_CLASS_MELEE));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_vindication::CheckProc);
    }
};

class aura_area52_swift_retribution_party : public AuraScript
{
    PrepareAuraScript(aura_area52_swift_retribution_party);

    uint32 Helper() const
    {
        return GetId() == 53379 ? 853379 : GetId() == 53484 ? 853484 : 853648;
    }

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* owner = GetTarget();
        if (owner && AscensionFreePick::Applies(owner->ToPlayer()))
            owner->CastSpell(owner, Helper(), true);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* owner = GetTarget();
        if (owner && AscensionFreePick::Applies(owner->ToPlayer()) && !owner->HasSpell(Helper()))
            owner->RemoveAurasDueToSpell(Helper(), owner->GetGUID());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_swift_retribution_party::Apply, EFFECT_1,
            SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_swift_retribution_party::Remove, EFFECT_1,
            SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_power_of_light : public AuraScript
{
    PrepareAuraScript(aura_area52_power_of_light);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* owner = GetTarget();
        if (!owner || !AscensionFreePick::Applies(owner->ToPlayer()))
            return true;
        Spell const* cast = eventInfo.GetProcSpell();
        HealInfo const* heal = eventInfo.GetHealInfo();
        return (owner->HasAura(84864) || owner->HasAura(84865)) && eventInfo.GetActor() == owner &&
            eventInfo.GetActionTarget() == owner && heal && heal->GetHeal() && cast &&
            !cast->IsTriggered() && cast->GetCastTime() > 0;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_power_of_light::CheckProc);
    }
};

void ApplyAscensionHeroPaladinContracts(SpellInfo* spellInfo)
{
    if (Area52MysticRealm::UsesFreePickEngine() && spellInfo->Id == 965903)
    {
        spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_APPLY_AREA_AURA_FRIEND;
        spellInfo->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(14);
        spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(31);
        spellInfo->AttributesEx3 |= SPELL_ATTR3_DOT_STACKING_RULE;
    }

    if (Area52MysticRealm::UsesFreePickEngine() && spellInfo->Id == 31890)
    {
        spellInfo->Effects[EFFECT_1].Effect = 0;
        spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_NONE;
    }

    if (Area52MysticRealm::UsesFreePickEngine() && (spellInfo->Id == 64127 || spellInfo->Id == 64129))
    {
        spellInfo->Effects[EFFECT_0].SpellClassMask = flag96(1, 0, 0x00800000);
        spellInfo->Effects[EFFECT_1].SpellClassMask = flag96(0, 1, 0);
    }

    if (Area52MysticRealm::UsesFreePickEngine() && spellInfo->Id >= 853510 && spellInfo->Id <= 853512)
    {
        spellInfo->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER;
        spellInfo->Effects[EFFECT_0].MiscValue = SPELL_SCHOOL_MASK_ALL;
        spellInfo->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_WEAPON_CRIT_PERCENT;
        if (spellInfo->Id == 853512)
        {
            spellInfo->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_PROC_TRIGGER_SPELL;
            spellInfo->Effects[EFFECT_2].TriggerSpell = 853513;
        }
    }
    if (!Area52MysticRealm::UsesFreePickEngine() && spellInfo->Id == FanaticismDebuff)
        spellInfo->Effects[EFFECT_0].SpellClassMask = FANATICISM_MASK;
}

#include "Area52CrusaderFanaticism.inc"
#include "Area52CrusaderVindication.inc"
#include "Area52ConsecratedGround.inc"

void AddSC_AscensionHeroPaladin()
{
    RegisterCreatureAI(npc_area52_consecrated_ground);
    Area52ConsecratedGroundRegistered = true;
    new Area52FanaticismCasts();
    new Area52VindicationCasts();
    new Area52VindicationAuras();
    RegisterSpellScript(aura_area52_power_of_light);
    RegisterSpellScript(aura_area52_vindication);
    RegisterSpellScript(aura_area52_swift_retribution_party);
    RegisterSpellScript(aura_hero_paladin_touched_by_the_light);
    RegisterSpellScript(aura_hero_paladin_sanctified_light);
    RegisterSpellScript(aura_hero_paladin_benediction);
    RegisterSpellScript(aura_hero_paladin_fanaticism);
    RegisterSpellScript(aura_hero_paladin_improved_blessing_of_might);
    RegisterSpellScript(spell_hero_paladin_judgement_unsealed);
    RegisterSpellScript(spell_hero_paladin_shield_of_righteousness);
    RegisterSpellScript(spell_hero_paladin_execution_sentence);
    RegisterSpellScript(aura_hero_paladin_execution_sentence_burst);
}
