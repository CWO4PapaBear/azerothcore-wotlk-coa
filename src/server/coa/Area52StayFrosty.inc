void ClearStayFrosty(Unit* owner)
{
    owner->RemoveAurasDueToSpell(414033);
    owner->RemoveAurasDueToSpell(414051);
    owner->RemoveAurasDueToSpell(997621);
}

void ConfigureStayFrosty(SpellInfo* info)
{
    if (info->Id == 414032)
    {
        info->ProcFlags = 0;
        info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
        info->Effects[EFFECT_0].TriggerSpell = 0;
        info->Effects[EFFECT_2].Effect = 0;
        info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_NONE;
        info->Effects[EFFECT_2].TriggerSpell = 0;
    }
    if (info->Id == 414033)
    {
        info->DurationEntry = sSpellDurationStore.LookupEntry(21);
        info->Effects[EFFECT_0].SpellClassMask = flag96(0, 0, 64);
    }
}

class StayFrostyCasts final : public AllSpellScript
{
public:
    StayFrostyCasts() : AllSpellScript("Area52StayFrostyCasts", {ALLSPELLHOOK_ON_BEFORE_EFFECTS}) { }

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!Applies(player) || spell->IsTriggered() || !Area52Holy::DamagingSpell(info) ||
            !AscensionMysticEnchant::GrantsActive(player, 414032) ||
            (!player->HasAura(44543) && !player->HasAura(44545)))
            return;
        player->CastSpell(player, 414033, true);
    }
};

class StayFrostyProcs final : public UnitScript
{
public:
    StayFrostyProcs() : UnitScript("Area52StayFrostyProcs", true,
        {UNITHOOK_ON_AURA_APPLY, UNITHOOK_ON_AURA_REMOVE}) { }

    void OnAuraApply(Unit* unit, Aura* aura) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (!Applies(player) || !aura || aura->GetId() != 74396 ||
            !AscensionMysticEnchant::GrantsActive(player, 414032) ||
            (!player->HasAura(44543) && !player->HasAura(44545)))
            return;
        player->RemoveAurasDueToSpell(414033);
        player->RemoveAurasDueToSpell(414051);
        player->CastSpell(player, 997621, true);
    }

    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (!Applies(player) || !application)
            return;
        uint32 const id = application->GetBase()->GetId();
        if (id == 44543 || id == 44545)
            ClearStayFrosty(player);
    }
};

class aura_area52_stay_frosty final : public AuraScript
{
    PrepareAuraScript(aura_area52_stay_frosty);

    void Removed(AuraEffect const*, AuraEffectHandleModes)
    {
        if (Applies(GetTarget()->ToPlayer()))
            ClearStayFrosty(GetTarget());
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_stay_frosty::Removed,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};
