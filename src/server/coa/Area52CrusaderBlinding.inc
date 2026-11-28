class spell_area52_crusader_blinding_light : public SpellScript
{
    PrepareSpellScript(spell_area52_crusader_blinding_light);

    bool Area52Caster()
    {
        return GetCaster() && AscensionFreePick::Applies(GetCaster()->ToPlayer());
    }

    bool Cleric()
    {
        return GetCaster()->HasAura(983590) && GetCaster()->HasAura(84867);
    }

    void HandleControl(SpellEffIndex effect)
    {
        if (Area52Caster() && Cleric())
        {
            PreventHitAura();
            PreventHitDefaultEffect(effect);
        }
    }

    void HandleHeal(SpellEffIndex effect)
    {
        if (Area52Caster() && !Cleric())
            PreventHitDefaultEffect(effect);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_crusader_blinding_light::HandleControl,
            EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        OnEffectHitTarget += SpellEffectFn(spell_area52_crusader_blinding_light::HandleControl,
            EFFECT_1, SPELL_EFFECT_APPLY_AURA);
        OnEffectLaunch += SpellEffectFn(spell_area52_crusader_blinding_light::HandleHeal,
            EFFECT_2, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_crusader_blinding_light::HandleHeal,
            EFFECT_2, SPELL_EFFECT_TRIGGER_SPELL);
    }
};
