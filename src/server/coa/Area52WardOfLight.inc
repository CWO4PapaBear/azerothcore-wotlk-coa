bool WardOfLightRegistered = false;
constexpr uint32 WardOfLight = 977900;
constexpr uint32 WardOfLightProtection = 977902;

bool WardOfLightActive(Player* player)
{
    return Applies(player) && AscensionMysticEnchant::GrantsActive(player, WardOfLight);
}

void ConfigureWardOfLight(SpellInfo* info)
{
    if (info->Id != WardOfLight)
        return;
    info->ProcFlags = 0;
    info->ProcCharges = 0;
    info->StackAmount = 1;
    info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
    info->Effects[EFFECT_0].TriggerSpell = 0;
    for (auto index : {EFFECT_1, EFFECT_2})
    {
        info->Effects[index].Effect = 0;
        info->Effects[index].ApplyAuraName = SPELL_AURA_NONE;
    }
}

void WardOfLightReset(Unit* target, Unit* attacker, uint32 damage, SpellInfo const* info)
{
    Player* player = target ? target->ToPlayer() : nullptr;
    if (!damage || !info || !attacker || attacker == target || !WardOfLightActive(player) || !roll_chance_i(20))
        return;
    for (auto const& [id, entry] : player->GetSpellMap())
        if (entry->State != PLAYERSPELL_REMOVED && player->MatchesSpellInteraction(id, 53385) &&
            player->GetSpellCooldownDelay(id))
            player->RemoveSpellCooldown(id, true);
}

class WardOfLightCasts final : public AllSpellScript
{
public:
    WardOfLightCasts() : AllSpellScript("Area52WardOfLightCasts",
        {ALLSPELLHOOK_ON_CALCULATED_TARGET, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellCalculatedTarget(Spell* spell, Unit* target, TargetInfo& hit) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!target || hit.damage <= 0 || !WardOfLightActive(player) ||
            !player->MatchesSpellInteraction(spell->GetSpellInfo()->Id, 53385))
            return;
        uint32 const percent = target->IsPlayer() ? 105 : 120;
        hit.damage = int32(int64(hit.damage) * percent / 100);
        hit.damageBeforeTakenMods = int32(int64(hit.damageBeforeTakenMods) * percent / 100);
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        if (!spell || !target || miss != SPELL_MISS_NONE || !damage)
            return;
        Player* player = spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (WardOfLightActive(player) && player->MatchesSpellInteraction(spell->GetSpellInfo()->Id, 53385))
            player->CastSpell(player, WardOfLightProtection, true);
        WardOfLightReset(target, spell->GetCaster(), damage, spell->GetSpellInfo());
    }
};

class WardOfLightAuras final : public UnitScript
{
public:
    WardOfLightAuras() : UnitScript("Area52WardOfLightAuras", true,
        {UNITHOOK_ON_PERIODIC_DAMAGE_RESULT, UNITHOOK_ON_AURA_REMOVE}) { }

    void OnPeriodicDamageResult(Unit* target, Unit* attacker, uint32 damage, SpellInfo const* info) override
    {
        WardOfLightReset(target, attacker, damage, info);
    }

    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (Applies(player) && application && application->GetBase()->GetId() == WardOfLight)
            player->RemoveAurasDueToSpell(WardOfLightProtection, player->GetGUID());
    }
};
