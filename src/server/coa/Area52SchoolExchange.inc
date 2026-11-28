bool SchoolExchangeRegistered = false;
struct SchoolExchangeEntry
{
    uint32 enchant;
    uint32 school;
    uint32 helper;
};
constexpr std::array<SchoolExchangeEntry, 5> SchoolExchanges = {{
    {84579, SPELL_SCHOOL_MASK_HOLY, 286101},
    {84581, SPELL_SCHOOL_MASK_FIRE, 286102},
    {84583, SPELL_SCHOOL_MASK_NATURE, 286103},
    {84587, SPELL_SCHOOL_MASK_SHADOW, 286105},
    {84589, SPELL_SCHOOL_MASK_ARCANE, 286106}
}};

bool SchoolExchangeReady(uint32 enchant)
{
    if (!Enabled || !SchoolExchangeRegistered || !sSpellMgr->GetSpellInfo(286100))
        return false;
    for (auto const& entry : SchoolExchanges)
        if (entry.enchant == enchant)
            return sSpellMgr->GetSpellInfo(entry.helper) != nullptr;
    return false;
}

bool SchoolExchangeDamage(SpellInfo const* info, std::set<uint32>& visited)
{
    if (!info || visited.size() >= 32 || !visited.insert(info->Id).second)
        return false;
    if (Area52Holy::DamagingSpell(info) || info->HasEffect(SPELL_EFFECT_POWER_BURN) ||
        info->HasAura(SPELL_AURA_PERIODIC_DAMAGE_PERCENT) || info->HasAura(SPELL_AURA_POWER_BURN))
        return true;
    for (auto const& effect : info->Effects)
        if ((effect.Effect == SPELL_EFFECT_TRIGGER_SPELL || effect.Effect == SPELL_EFFECT_TRIGGER_MISSILE ||
             effect.Effect == SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE || effect.Effect == SPELL_EFFECT_TRIGGER_MISSILE_SPELL_WITH_VALUE ||
             effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
             effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL_FROM_CLIENT ||
             effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE) &&
            effect.TriggerSpell && SchoolExchangeDamage(sSpellMgr->GetSpellInfo(effect.TriggerSpell), visited))
            return true;
    return false;
}

void CleanupSchoolExchange(Player* player)
{
    if (!Applies(player) || !player->GetSession() || player->GetSession()->PlayerLogout() ||
        !AscensionMysticEnchant::CollectionLoaded(player) || !AscensionMysticEnchant::DraftEquipmentReady(player))
        return;
    bool physical = false;
    for (auto const& entry : SchoolExchanges)
    {
        if (!SchoolExchangeReady(entry.enchant))
            return;
        bool const active = AscensionMysticEnchant::GrantsActive(player, entry.enchant);
        physical = physical || active;
        if (!active && !player->HasSpell(entry.helper))
            player->RemoveAurasDueToSpell(entry.helper, player->GetGUID());
    }
    if (!physical && !player->HasSpell(286100))
        player->RemoveAurasDueToSpell(286100, player->GetGUID());
}

class SchoolExchangeCasts final : public AllSpellScript
{
public:
    SchoolExchangeCasts() : AllSpellScript("Area52SchoolExchangeCasts", {ALLSPELLHOOK_ON_CAST}) { }
    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* info, bool) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!spell || !info || !Applies(player))
            return;
        Unit* target = spell->m_targets.GetUnitTarget();
        bool const scriptedDamage = target && player->IsValidAttackTarget(target) &&
            (player->MatchesSpellInteraction(info->Id, 20271) || player->MatchesSpellInteraction(info->Id, 53407) ||
             player->MatchesSpellInteraction(info->Id, 53408) || player->MatchesSpellInteraction(info->Id, 20473) ||
             player->MatchesSpellInteraction(info->Id, 954817) || player->MatchesSpellInteraction(info->Id, 47540));
        std::set<uint32> visited;
        if ((!SchoolExchangeDamage(info, visited) && !scriptedDamage) || spell->IsTriggered() ||
            spell->IsAutoRepeat() || info->Id == 6603 || info->Id == 75)
            return;
        for (auto const& entry : SchoolExchanges)
        {
            if (!SchoolExchangeReady(entry.enchant) || !player->HasAura(entry.enchant) ||
                !AscensionMysticEnchant::GrantsActive(player, entry.enchant) ||
                spell->GetScriptValue(entry.enchant))
                continue;
            spell->SetScriptValue(entry.enchant, 1);
            if (spell->GetSpellSchoolMask() & SPELL_SCHOOL_MASK_NORMAL)
                player->CastSpell(player, entry.helper, true);
            if (spell->GetSpellSchoolMask() & entry.school)
                player->CastSpell(player, 286100, true);
        }
    }
};

class SchoolExchangeLifecycle final : public PlayerScript
{
public:
    SchoolExchangeLifecycle() : PlayerScript("Area52SchoolExchangeLifecycle") { }
    void OnPlayerLogin(Player* player) override { CleanupSchoolExchange(player); }
    void OnPlayerUpdate(Player* player, uint32) override
    {
        if (player->HasAura(286100) || std::any_of(SchoolExchanges.begin(), SchoolExchanges.end(),
            [player](SchoolExchangeEntry const& entry) { return player->HasAura(entry.helper); }))
            CleanupSchoolExchange(player);
    }
};
