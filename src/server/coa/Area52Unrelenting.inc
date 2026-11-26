namespace
{
uint32 Area52UnrelentingRank(Player const* player)
{
    if (!AscensionFreePick::Applies(player))
        return 0;
    for (uint32 rank = 3; rank; --rank)
        if (player->HasAura(30663 + rank))
            return rank;
    return 0;
}

bool Area52UnrelentingCast(Player const* player, uint32 spell)
{
    return player->MatchesSpellInteraction(spell, 403) || player->MatchesSpellInteraction(spell, 324) ||
        player->MatchesSpellInteraction(spell, 51505) || player->MatchesSpellInteraction(spell, 954807);
}

void ConfigureArea52Unrelenting(SpellInfo* info)
{
    if (Area52WindMetadataEnabled && (info->Id == 30675 || info->Id == 30678 || info->Id == 30679))
    {
        info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
        info->Effects[EFFECT_0].Amplitude = 0;
    }
    if (Area52WindMetadataEnabled && (info->Id == 45284 || info->Id == 45297))
    {
        info->AttributesEx3 |= SPELL_ATTR3_IGNORE_CASTER_MODIFIERS | SPELL_ATTR3_SUPPRESS_CASTER_PROCS;
        info->AttributesEx |= SPELL_ATTR1_NO_THREAT;
        info->Effects[EFFECT_0].RealPointsPerLevel = 0;
    }

    if (Area52WindMetadataEnabled && info->Id >= 30664 && info->Id <= 30666)
        info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_DUMMY;
}
}

class area52_unrelenting_cast final : public AllSpellScript
{
public:
    area52_unrelenting_cast() : AllSpellScript("area52_unrelenting_cast", {ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_CALCULATED_TARGET, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        uint32 const rank = Area52UnrelentingRank(player);
        if (rank == 3 && spell && info && !spell->IsTriggered() &&
            player->MatchesSpellInteraction(info->Id, 421))
            if (Aura* stacks = player->GetAura(412002, player->GetGUID()); stacks && stacks->GetStackAmount() == 3)
                spell->SetScriptValue(30666, 1);
        if (!rank || !spell || !info || spell->IsTriggered() ||
            !Area52UnrelentingCast(player, info->Id) || spell->GetScriptValue(30664))
            return;
        spell->SetScriptValue(30664, 1);
        player->CastSpell(player, 411999 + rank, true);
    }

    void OnSpellCalculatedTarget(Spell* spell, Unit*, TargetInfo& info) override
    {
        if (spell->GetScriptValue(30666) && info.damage > 0)
        {
            info.damage = CalculatePct(info.damage, 115);
            info.damageBeforeTakenMods = CalculatePct(info.damageBeforeTakenMods, 115);
        }
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player) || !target || miss != SPELL_MISS_NONE || !damage)
            return;
        if (spell->GetScriptValue(30666))
            player->CastSpell(target, 412004, true);
        if (player->HasAura(30679) &&
            (player->MatchesSpellInteraction(spell->GetSpellInfo()->Id, 8042) ||
                player->MatchesSpellInteraction(spell->GetSpellInfo()->Id, 8050) ||
                player->MatchesSpellInteraction(spell->GetSpellInfo()->Id, 8056)))
            if (target->GetAura(412004, player->GetGUID()))
            {
                target->RemoveAurasDueToSpell(412004, player->GetGUID());
                player->CastSpell(target, 412005, true);
            }
    }
};

class area52_unrelenting_cleanup final : public UnitScript
{
public:
    area52_unrelenting_cleanup() : UnitScript("area52_unrelenting_cleanup", {UNITHOOK_ON_AURA_REMOVE}) { }

    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player) || !application)
            return;
        uint32 const id = application->GetBase()->GetId();
        if (id >= 30664 && id <= 30666)
            player->RemoveAurasDueToSpell(412000 + id - 30664, player->GetGUID());
    }
};
