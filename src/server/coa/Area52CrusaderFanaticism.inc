bool Area52FanaticismSentence(Player* player, Unit* target)
{
    for (auto const& rank : EXECUTION_SENTENCE_RANKS)
        if (target->HasAura(rank.Sentence, player->GetGUID()))
            return true;
    return false;
}

bool Area52FanaticismJudgement(Player* player, SpellInfo const* info)
{
    if (info->IsAffected(SPELLFAMILY_PALADIN, flag96(0x00800000, 0, 0)))
        return true;
    for (uint32 original : player->GetSpellInteractionOrigins(info->Id))
        if (SpellInfo const* ancestor = sSpellMgr->GetSpellInfo(original))
            if (ancestor->IsAffected(SPELLFAMILY_PALADIN, flag96(0x00800000, 0, 0)))
                return true;
    return false;
}

bool Area52FanaticismSeal(Player* player, SpellInfo const* info)
{
    flag96 const mask(0x02000000, 0x20000800, 0);
    if (info->IsAffected(SPELLFAMILY_PALADIN, mask))
        return true;
    for (uint32 original : player->GetSpellInteractionOrigins(info->Id))
        if (SpellInfo const* ancestor = sSpellMgr->GetSpellInfo(original))
            if (ancestor->IsAffected(SPELLFAMILY_PALADIN, mask))
                return true;
    return false;
}

bool Area52FanaticismMarkAffected(Unit* caster, SpellInfo const* info)
{
    Player* player = caster ? caster->ToPlayer() : nullptr;
    return AscensionFreePick::Applies(player) && info && Area52FanaticismSeal(player, info);
}

uint32 Area52FanaticismPeriodicBonus(Unit* caster, Unit* target, SpellInfo const* info)
{
    Player* player = caster ? caster->ToPlayer() : nullptr;
    if (!AscensionFreePick::Applies(player) || !target || !info ||
        !Area52FanaticismSentence(player, target) || !Area52FanaticismSeal(player, info))
        return 0;
    return player->HasAura(31881) ? 20 : player->HasAura(31880) ? 14 : player->HasAura(31879) ? 7 : 0;
}

class Area52FanaticismCasts final : public AllSpellScript
{
public:
    Area52FanaticismCasts() : AllSpellScript("Area52FanaticismCasts",
        {ALLSPELLHOOK_ON_CALCULATED_TARGET, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellCalculatedTarget(Spell* spell, Unit* target, TargetInfo& hit) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player) || !target || hit.damage <= 0 ||
            !Area52FanaticismSentence(player, target))
            return;
        uint32 percent = player->HasAura(31881) ? 20 : player->HasAura(31880) ? 14 : player->HasAura(31879) ? 7 : 0;
        SpellInfo const* info = spell->GetSpellInfo();
        if (!percent || (!player->MatchesSpellInteraction(info->Id, 35395) &&
            !Area52FanaticismJudgement(player, info) && !Area52FanaticismSeal(player, info)))
            return;
        hit.damage = int32(int64(hit.damage) * (100 + percent) / 100);
        hit.damageBeforeTakenMods = int32(int64(hit.damageBeforeTakenMods) * (100 + percent) / 100);
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player) || !target || miss != SPELL_MISS_NONE || !damage ||
            !player->HasAura(31881))
            return;
        SpellInfo const* info = spell->GetSpellInfo();
        if (player->MatchesSpellInteraction(info->Id, 35395))
            player->CastSpell(target, FanaticismDebuff, true);
        if (!target->IsCreature() || target->GetCharmerOrOwnerPlayerOrPlayerItself() ||
            !Area52FanaticismJudgement(player, info) ||
            (!target->HasAura(FanaticismDebuff, player->GetGUID()) && !Area52FanaticismSentence(player, target)))
            return;
        player->CastSpell(player, 31890, true);
        for (auto const& [id, entry] : player->GetSpellMap())
            if (entry->State != PLAYERSPELL_REMOVED && player->MatchesSpellInteraction(id, 954817) &&
                player->GetSpellCooldownDelay(id))
                player->ModifySpellCooldown(id, -10000);
    }
};
