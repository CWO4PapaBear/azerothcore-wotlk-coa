/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
void ConfigureWizardFrostbolt(SpellInfo* info)
{
    if (info->Id == Area52WizardFrostbolt::Power || info->Id == Area52WizardFrostbolt::Cooldown)
    {
        info->AttributesEx4 |= SPELL_ATTR4_AURA_EXPIRES_OFFLINE;
        if (info->Id == Area52WizardFrostbolt::Cooldown)
            info->AttributesCu |= SPELL_ATTR0_CU_FORCE_AURA_SAVING;
    }
    if (info->Id != Area52WizardFrostbolt::Enchant)
        return;
    for (auto& effect : info->Effects)
        if (effect.Effect == SPELL_EFFECT_APPLY_AURA)
        {
            effect.ApplyAuraName = SPELL_AURA_DUMMY;
            effect.SpellClassMask = flag96(0, 0, 0);
        }
}

void RefreshWizardFrostbolt(Player* player)
{
    using namespace Area52WizardFrostbolt;
    Aura* buff = player->GetAura(Power, player->GetGUID());
    if (!buff)
        return;
    auto const* source = buff->GetTriggeredByAuraSpellInfo();
    if (!source || source->Id != Enchant)
        return;
    Aura* enchant = player->GetAura(Enchant, player->GetGUID());
    if (!Applies(player) || !AscensionMysticEnchant::GrantsActive(player, Enchant) || !enchant)
    {
        buff->Remove();
        return;
    }
    if (AuraEffect* effect = buff->GetEffect(EFFECT_0))
        effect->ChangeAmount(int32(SpellPower(player->GetLevel(), enchant->GetStackAmount())));
}

class WizardFrostboltCasts final : public AllSpellScript
{
public:
    WizardFrostboltCasts() : AllSpellScript("Area52WizardFrostboltCasts", {ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        using namespace Area52WizardFrostbolt;
        if (!spell || !target)
            return;
        Player* player = spell->GetCaster()->ToPlayer();
        if (!player || !Eligible(Applies(player), AscensionMysticEnchant::GrantsActive(player, Enchant),
            player->IsAlive(), spell->IsTriggered(), player->MatchesSpellInteraction(spell->GetSpellInfo()->Id, 116),
            damage != 0, miss != SPELL_MISS_NONE, player->HasAura(Cooldown), false))
            return;
        Aura* enchant = player->GetAura(Enchant, player->GetGUID());
        SpellInfo const* companion = sSpellMgr->GetSpellInfo(Companion);
        if (!enchant || !companion)
            return;
        if (!roll_chance_i(companion->ProcChance))
            return;
        uint32 const copies = enchant->GetStackAmount();
        int32 power = int32(SpellPower(player->GetLevel(), copies));
        player->CastSpell(player, Cooldown, true);
        player->CastCustomSpell(player, Power, &power, nullptr, nullptr, true, nullptr, enchant->GetEffect(EFFECT_0));
        player->EnergizeBySpell(player, Mana, ManaReturn(player->GetCreateMana(), copies), POWER_MANA);
    }
};

class WizardFrostboltCleanup final : public UnitScript
{
public:
    WizardFrostboltCleanup() : UnitScript("Area52WizardFrostboltCleanup", true, {UNITHOOK_ON_AURA_REMOVE}) { }

    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (!Applies(player) || !application || application->GetBase()->GetId() != Area52WizardFrostbolt::Enchant)
            return;
        if (Aura* buff = player->GetAura(Area52WizardFrostbolt::Power, player->GetGUID()))
            if (auto const* source = buff->GetTriggeredByAuraSpellInfo(); source && source->Id == Area52WizardFrostbolt::Enchant)
                buff->Remove();
    }
};
