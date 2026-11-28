bool ConsecratedStrikesRegistered = false;

bool ConsecratedStrikesReady()
{
    if (!Enabled || !ConsecratedStrikesRegistered || !ConsecratedWeaponReady() ||
        !HasArea52ConsecratedGroundHandlers())
        return false;
    for (uint32 id : {278745u, 278746u, 965321u, 965903u, 977847u, 977848u, 977849u})
        if (!sSpellMgr->GetSpellInfo(id))
            return false;
    return true;
}

void ConfigureConsecratedStrikes(SpellInfo* info)
{
    if (info->Id != 965295)
        return;
    info->ProcFlags = 0;
    info->ProcCharges = 0;
    info->StackAmount = 1;
    info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
    info->Effects[EFFECT_0].TriggerSpell = 0;
}

class ConsecratedStrikesCasts final : public AllSpellScript
{
public:
    ConsecratedStrikesCasts() : AllSpellScript("Area52ConsecratedStrikesCasts", {ALLSPELLHOOK_ON_HIT_RESULT}) { }
    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!Applies(player) || !target || !damage || miss != SPELL_MISS_NONE || spell->IsTriggered() ||
            !player->HasAura(965295) || !AscensionMysticEnchant::GrantsActive(player, 965295) ||
            spell->GetScriptValue(965295))
            return;
        uint32 const id = spell->GetSpellInfo()->Id;
        if (!player->MatchesSpellInteraction(id, 35395) && !player->MatchesSpellInteraction(id, 53385) &&
            !player->MatchesSpellInteraction(id, 53595))
            return;
        spell->SetScriptValue(965295, 1);
        if (roll_chance_i(50))
            player->CastSpell(player, 965321, true);
    }
};
