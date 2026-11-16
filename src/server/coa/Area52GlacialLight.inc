bool GlacialLightSpell(uint32 spell)
{
    return spell >= 997221 && spell <= 997231;
}

bool GlacialLightPresent(Unit const* target, ObjectGuid caster = ObjectGuid::Empty)
{
    if (!target)
        return false;
    for (uint32 spell = 997221; spell <= 997231; ++spell)
        if (target->HasAura(spell, caster))
            return true;
    return false;
}

void ConsumeGlacialLightFingers(Spell* spell)
{
    Player* player = spell->GetCaster()->ToPlayer();
    if (Applies(player) && !spell->IsTriggered() && GlacialLightSpell(spell->GetSpellInfo()->Id) &&
        AscensionMysticEnchant::GrantsActive(player, 997220))
        if (Aura* fingers = player->GetAura(74396))
            fingers->DropCharge(AURA_REMOVE_BY_EXPIRE);
}

class GlacialLightSpells final : public AllSpellScript
{
public:
    GlacialLightSpells() : AllSpellScript("Area52GlacialLightSpells",
        {ALLSPELLHOOK_ON_CRIT_CHANCE, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellCritChance(Spell* spell, Unit* target, float& chance) override
    {
        Player* player = spell->GetCaster()->ToPlayer();
        if (!Applies(player) || !target || !GlacialLightSpell(spell->GetSpellInfo()->Id) ||
            !AscensionMysticEnchant::GrantsActive(player, 997220))
            return;
        if (!target->HasAuraState(AURA_STATE_FROZEN) && !player->HasAura(74396))
            return;
        for (uint32 talent : {12983u, 12982u, 11170u})
            if (AuraEffect const* effect = player->GetAuraEffect(talent, EFFECT_0))
            {
                chance += effect->GetAmount();
                break;
            }
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        Player* player = spell->GetCaster()->ToPlayer();
        if (!Applies(player) || miss != SPELL_MISS_NONE || !damage ||
            !(spell->GetSpellInfo()->GetSchoolMask() & (SPELL_SCHOOL_MASK_HOLY | SPELL_SCHOOL_MASK_FROST)) ||
            !AscensionMysticEnchant::GrantsActive(player, 997220) ||
            !player->HasAura(12983) || !player->HasAura(44545) ||
            !GlacialLightPresent(target, player->GetGUID()))
            return;
        SpellInfo const* reduction = sSpellMgr->GetSpellInfo(997233);
        if (reduction)
            player->ModifySpellCooldown(12472, std::min<int32>(0, reduction->Effects[EFFECT_0].CalcValue(player)));
    }
};

class GlacialLightAbsorbs final : public UnitScript
{
public:
    GlacialLightAbsorbs() : UnitScript("Area52GlacialLightAbsorbs", true,
        {UNITHOOK_ON_AFTER_AURA_EFFECT_CALCULATE_AMOUNT}) { }

    void OnAfterAuraEffectCalculateAmount(AuraEffect const* effect, Unit*, int32& amount) override
    {
        if (!Enabled || amount <= 0 || (effect->GetAuraType() != SPELL_AURA_SCHOOL_ABSORB &&
            effect->GetAuraType() != SPELL_AURA_MANA_SHIELD))
            return;
        if (GlacialLightPresent(effect->GetBase()->GetUnitOwner()))
            amount = int32(int64(amount) * 70 / 100);
    }
};
