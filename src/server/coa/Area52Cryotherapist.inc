/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

bool CryotherapistActive(Player* player)
{
    return Applies(player) && player->IsAlive() && AscensionMysticEnchant::GrantsActive(player, 86576);
}

struct SpiritFrostState : DataMap::Base
{
    uint64 generation = 0;
};

SpiritFrostState& SpiritState(Player* player)
{
    return *player->CustomData.GetDefault<SpiritFrostState>("Area52SpiritFrost");
}

bool SpiritFrostHeal(Player* player, uint32 spell)
{
    return player && (player->MatchesSpellInteraction(spell, 2061) ||
        player->MatchesSpellInteraction(spell, 2060) || player->MatchesSpellInteraction(spell, 596));
}

Unit* FrozenLightTarget(Player* player)
{
    if (!CryotherapistActive(player) || !player->IsInWorld())
        return nullptr;
    for (Aura* aura : player->GetSingleCastAuras())
    {
        if (aura->GetId() != 92752 || aura->GetCasterGUID() != player->GetGUID())
            continue;
        Unit* target = aura->GetUnitOwner();
        if (target && target->IsAlive() && target->IsInWorld() && target->GetMap() == player->GetMap() &&
            player->IsValidAssistTarget(target))
            return target;
    }
    return nullptr;
}

class CryotherapistContracts final : public GlobalScript
{
public:
    CryotherapistContracts() : GlobalScript("Area52CryotherapistContracts") { }

    bool OnIsAffectedBySpellModCheck(SpellInfo const* source, SpellInfo const* spell,
        SpellModifier const* modifier) override
    {
        if (!Enabled || source->Id != 92753 || !modifier->ownerAura)
            return true;
        Unit* caster = modifier->ownerAura->GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        return !(CryotherapistActive(player) && SpiritFrostHeal(player, spell->Id));
    }

    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!Enabled)
            return;
        if (info->Id == 92752)
        {
            info->AttributesEx &= ~SPELL_ATTR1_EXCLUDE_CASTER;
            info->AttributesEx5 |= SPELL_ATTR5_LIMIT_N;
            info->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_HEALING_RECEIVED;
        }
        if (info->Id == 300024)
        {
            info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_MOD_HEALING_RECEIVED;
            info->ExcludeTargetAuraSpell = 0;
        }
        if (info->Id == 86577)
        {
            info->Effects[EFFECT_0].Amplitude = 1000;
            info->Effects[EFFECT_0].DieSides = 0;
            info->Effects[EFFECT_0].RealPointsPerLevel = 0;
            info->AttributesEx5 |= SPELL_ATTR5_EXTRA_INITIAL_PERIOD;
            info->AttributesEx5 &= ~SPELL_ATTR5_SPELL_HASTE_AFFECTS_PERIODIC;
            info->AttributesEx2 |= SPELL_ATTR2_CANT_CRIT;
        }
        if (info->Id == 92753)
        {
            info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
            info->Effects[EFFECT_0].TriggerSpell = 0;
            info->Effects[EFFECT_1].BasePoints = -54;
            info->Effects[EFFECT_2].BasePoints = -11;
        }
        if (info->Id == 92762)
        {
            info->Effects[EFFECT_0].Effect = SPELL_EFFECT_DUMMY;
            info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_NONE;
        }
        if (info->Id == 271306 || info->Id == 271307)
        {
            info->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_TARGET_ALLY);
            info->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(0);
            info->Effects[EFFECT_1].Effect = 0;
            info->MaxAffectedTargets = 1;
            info->AttributesEx2 |= SPELL_ATTR2_CANT_CRIT;
        }
        if (info->Id == 92766)
        {
            info->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_DEST_AREA_ALLY);
            info->Effects[EFFECT_0].TargetB = SpellImplicitTargetInfo(0);
            info->Effects[EFFECT_1].Effect = 0;
            info->MaxAffectedTargets = 5;
        }
    }
};

class spell_area52_cryotherapist_gate : public SpellScript
{
    PrepareSpellScript(spell_area52_cryotherapist_gate);

    SpellCastResult Check()
    {
        return CryotherapistActive(GetCaster()->ToPlayer()) ? SPELL_CAST_OK : SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_cryotherapist_gate::Check);
    }
};

class spell_area52_icy_penance_bolt : public AllSpellScript
{
public:
    spell_area52_icy_penance_bolt() : AllSpellScript("spell_area52_icy_penance_bolt") { }

    bool CanPrepare(Spell* spell, SpellCastTargets const*, AuraEffect const*) override
    {
        Player* player = spell->GetCaster()->ToPlayer();
        if (!spell->IsTriggered() && CryotherapistActive(player) && SpiritFrostHeal(player, spell->GetSpellInfo()->Id))
            if (Aura* stacks = player->GetAura(92753, player->GetGUID()))
            {
                spell->SetScriptValue(92753, Area52Cryotherapist::ConsumedHealingStacks(stacks->GetStackAmount()));
                spell->SetScriptValue(92764, SpiritState(player).generation);
            }
        return true;
    }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const*, bool) override
    {
        Player* player = caster->ToPlayer();
        uint32 consumed = spell->GetScriptValue(92753);
        spell->SetScriptValue(92753, 0);
        if (consumed && CryotherapistActive(player) && spell->GetScriptValue(92764) == SpiritState(player).generation)
            if (Aura* stacks = player->GetAura(92753, player->GetGUID()))
                stacks->ModStackAmount(-int32(consumed));
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32 healing, bool critical) override
    {
        if (!spell || !target || miss != SPELL_MISS_NONE)
            return;
        uint32 id = spell->GetSpellInfo()->Id;
        Player* player = spell->GetCaster()->ToPlayer();
        if (!CryotherapistActive(player))
            return;
        if (healing && (spell->GetSpellInfo()->GetSchoolMask() & SPELL_SCHOOL_MASK_HOLY) &&
            spell->GetSpellInfo()->HasEffect(SPELL_EFFECT_HEAL) && id != 271306 && id != 271307 && id != 92766)
        {
            if (!target->HasAura(92752, player->GetGUID()))
                player->CastSpell(target, 300024, true);
            if (!spell->GetScriptValue(92768))
            {
                spell->SetScriptValue(92768, 1);
                player->CastSpell(player, 92768, true);
            }
        }
        if (!damage)
            return;
        if (player->MatchesSpellInteraction(id, 116) || player->MatchesSpellInteraction(id, 30455))
        {
            player->CastSpell(player, 92753, true);
            std::list<Unit*> allies;
            Acore::AnyFriendlyUnitInObjectRangeCheck check(target, player, 10.0f);
            Acore::UnitListSearcher<Acore::AnyFriendlyUnitInObjectRangeCheck> searcher(target, allies, check);
            Cell::VisitObjects(target, searcher, 10.0f);
            allies.remove_if([player](Unit* ally)
            {
                return !ally->IsAlive() || !player->IsValidAssistTarget(ally);
            });
            allies.sort([](Unit* left, Unit* right)
            {
                return left->GetHealthPct() == right->GetHealthPct() ? left->GetGUID() < right->GetGUID() :
                    left->GetHealthPct() < right->GetHealthPct();
            });
            if (allies.size() > 5)
                allies.resize(5);
            int32 pulse = Area52Cryotherapist::BaseHealingPulse(damage);
            for (Unit* ally : allies)
                player->CastCustomSpell(ally, 86577, &pulse, nullptr, nullptr,
                    TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_NO_PERIODIC_RESET));
        }
        if (!((id >= 92758 && id <= 92761) || (id >= 302043 && id <= 302046)))
            return;
        if (!FrozenLightTarget(player))
            return;
        int32 value = critical ? 1 : 0;
        player->CastCustomSpell(player, 92762, &value, nullptr, nullptr, true);
    }
};

class aura_area52_cryotherapy : public AuraScript
{
    PrepareAuraScript(aura_area52_cryotherapy);

    bool Load() override
    {
        return Enabled;
    }

    void Amount(AuraEffect const* effect, int32& amount, bool& recalculate)
    {
        amount = std::max(0, effect->GetBaseAmount());
        recalculate = false;
    }

    void Tick(AuraEffect const*)
    {
        if (!CryotherapistActive(GetCaster() ? GetCaster()->ToPlayer() : nullptr))
        {
            PreventDefaultAction();
            Remove();
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_area52_cryotherapy::Amount,
            EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_cryotherapy::Tick,
            EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
    }
};

class aura_area52_spirit_frost : public AuraScript
{
    PrepareAuraScript(aura_area52_spirit_frost);

    bool Load() override
    {
        Unit* owner = GetUnitOwner();
        Player* player = owner ? owner->ToPlayer() : nullptr;
        if (!Applies(player))
            return false;
        ++SpiritState(player).generation;
        return true;
    }

    void Modifier(AuraEffect const* effect, SpellModifier*& modifier)
    {
        if (modifier)
        {
            modifier->value = -int32(Area52Cryotherapist::ConsumedHealingStacks(GetStackAmount()) *
                (effect->GetEffIndex() == EFFECT_1 ? 53 : 10));
        }
    }

    void Register() override
    {
        DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(aura_area52_spirit_frost::Modifier,
            EFFECT_1, SPELL_AURA_ADD_PCT_MODIFIER);
        DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(aura_area52_spirit_frost::Modifier,
            EFFECT_2, SPELL_AURA_ADD_PCT_MODIFIER);
    }
};

class CryotherapyHealingCap final : public UnitScript
{
public:
    CryotherapyHealingCap() : UnitScript("Area52CryotherapyHealingCap") { }

    void ModifyHealReceived(Unit* target, Unit* healer, uint32& heal, SpellInfo const* info) override
    {
        if (info && info->Id == 86577 && target && healer && Applies(healer->ToPlayer()))
            heal = std::min(heal, uint32(uint64(target->GetMaxHealth()) * 45 / 100));
    }
};

class aura_area52_fingers_of_light_consumption : public AuraScript
{
    PrepareAuraScript(aura_area52_fingers_of_light_consumption);

    void Consume(ProcEventInfo&)
    {
        Player* player = GetTarget()->ToPlayer();
        if (CryotherapistActive(player) && player->HasAura(74396, player->GetGUID()))
            player->RemoveAurasDueToSpell(92768, player->GetGUID());
    }

    void Register() override
    {
        AfterProc += AuraProcFn(aura_area52_fingers_of_light_consumption::Consume);
    }
};

class aura_area52_frozen_light : public AuraScript
{
    PrepareAuraScript(aura_area52_frozen_light);

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        if (CryotherapistActive(GetCaster() ? GetCaster()->ToPlayer() : nullptr))
            GetTarget()->RemoveAurasDueToSpell(300024, GetCasterGUID());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_frozen_light::Apply,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};
class spell_area52_frozen_light_route : public SpellScript
{
    PrepareSpellScript(spell_area52_frozen_light_route);

    void Route(SpellEffIndex)
    {
        Player* player = GetCaster()->ToPlayer();
        Unit* target = FrozenLightTarget(player);
        if (!target)
            return;
        player->CastSpell(target, GetEffectValue() ? 271306 : 271307, true);
        if (player->HasAura(92753, player->GetGUID()))
            player->CastSpell(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 92766, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_frozen_light_route::Route, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_area52_icy_spirit : public SpellScript
{
    PrepareSpellScript(spell_area52_icy_spirit);

    void Select(std::list<WorldObject*>& targets)
    {
        Player* player = GetCaster()->ToPlayer();
        if (!CryotherapistActive(player) || !FrozenLightTarget(player) || !player->HasAura(92753, player->GetGUID()))
        {
            targets.clear();
            return;
        }
        targets.remove_if([player](WorldObject* object)
        {
            Unit* unit = object->ToUnit();
            return !unit || !unit->IsAlive() || !player->IsValidAssistTarget(unit) || !player->IsInRaidWith(unit);
        });
        targets.sort([](WorldObject* left, WorldObject* right)
        {
            float a = left->ToUnit()->GetHealthPct(), b = right->ToUnit()->GetHealthPct();
            return a == b ? left->GetGUID() < right->GetGUID() : a < b;
        });
        if (targets.size() > 5)
            targets.resize(5);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_area52_icy_spirit::Select,
            EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
    }
};

class aura_area52_icy_penance_channel : public AuraScript
{
    PrepareAuraScript(aura_area52_icy_penance_channel);
    bool ticked = false;

    void Tick(AuraEffect const*)
    {
        ticked = true;
    }

    void End(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!ticked || !CryotherapistActive(player))
            return;
        if (Aura* stacks = player->GetAura(92753, player->GetGUID()))
            stacks->ModStackAmount(-1);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_icy_penance_channel::Tick,
            EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_icy_penance_channel::End,
            EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};
