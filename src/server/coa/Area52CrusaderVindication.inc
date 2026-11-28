void Area52VindicatedHit(Unit* caster, Unit* target, uint32 damage, SpellInfo const* info)
{
    Player* player = caster ? caster->ToPlayer() : nullptr;
    if (!AscensionFreePick::Applies(player) || !target || !damage || !info || info->Id == 200067 ||
        !(info->GetSchoolMask() & SPELL_SCHOOL_MASK_HOLY) || !player->HasAura(209452) ||
        !target->HasAura(67))
        return;
    player->CastSpell(player, 200067, true);
}

void Area52VindicatedRemove(Unit* unit, AuraApplication* application)
{
    Player* player = unit ? unit->ToPlayer() : nullptr;
    if (AscensionFreePick::Applies(player) && application && application->GetBase()->GetId() == 209452 &&
        !player->HasSpell(200067))
        player->RemoveAurasDueToSpell(200067, player->GetGUID());
}

class Area52VindicationCasts final : public AllSpellScript
{
public:
    Area52VindicationCasts() : AllSpellScript("Area52VindicationCasts", {ALLSPELLHOOK_ON_HIT_RESULT}) { }
    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        if (spell && miss == SPELL_MISS_NONE)
            Area52VindicatedHit(spell->GetCaster(), target, damage, spell->GetSpellInfo());
    }
};

class Area52VindicationAuras final : public UnitScript
{
public:
    Area52VindicationAuras() : UnitScript("Area52VindicationAuras", true,
        {UNITHOOK_ON_PERIODIC_DAMAGE_RESULT, UNITHOOK_ON_AURA_REMOVE}) { }
    void OnPeriodicDamageResult(Unit* target, Unit* attacker, uint32 damage, SpellInfo const* info) override
    {
        Area52VindicatedHit(attacker, target, damage, info);
    }
    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Area52VindicatedRemove(unit, application);
    }
};
