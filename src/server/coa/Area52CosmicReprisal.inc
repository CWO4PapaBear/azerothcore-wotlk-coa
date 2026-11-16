constexpr uint32 CosmicReprisal = 978733;
constexpr uint32 CosmicSolarEclipse = 48517;
constexpr uint32 CosmicLunarEclipse = 48518;
constexpr uint32 CosmicBrightSun = 978734;
constexpr uint32 CosmicShimmeringMoon = 978736;

void ConfigureCosmicReprisal(SpellInfo* info)
{
    if (info->Id == CosmicBrightSun)
        info->ProcFlags = PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK | PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS |
            PROC_FLAG_TAKEN_RANGED_AUTO_ATTACK | PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS;
    if (info->Id == CosmicShimmeringMoon)
        info->ProcFlags = PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG;
}

void SynchronizeCosmicReprisal(Player* player)
{
    if (!Applies(player) || !player->IsInWorld())
        return;
    bool const active = player->IsAlive() && player->HasAura(CosmicReprisal, player->GetGUID()) &&
        AscensionMysticEnchant::GrantsActive(player, CosmicReprisal);
    constexpr std::array<std::pair<uint32, uint32>, 2> links =
    {{
        {CosmicSolarEclipse, CosmicBrightSun},
        {CosmicLunarEclipse, CosmicShimmeringMoon}
    }};
    for (auto const& [eclipse, helper] : links)
    {
        Aura const* effect = active ? player->GetAura(eclipse, player->GetGUID()) : nullptr;
        if (!effect)
        {
            player->RemoveAurasDueToSpell(helper, player->GetGUID());
            continue;
        }
        Aura* defense = player->GetAura(helper, player->GetGUID());
        if (!defense)
            defense = player->AddAura(helper, player);
        if (defense)
        {
            defense->SetMaxDuration(effect->GetMaxDuration());
            defense->SetDuration(effect->GetDuration());
        }
    }
}

bool CosmicReprisalLifecycleSpell(uint32 spell)
{
    return spell == CosmicReprisal || spell == CosmicSolarEclipse || spell == CosmicLunarEclipse;
}

class CosmicReprisalAuras final : public UnitScript
{
public:
    CosmicReprisalAuras() : UnitScript("Area52CosmicReprisalAuras", true,
        {UNITHOOK_ON_AURA_APPLY, UNITHOOK_ON_AURA_REMOVE}) { }

    void OnAuraApply(Unit* unit, Aura* aura) override
    {
        if (unit && aura && CosmicReprisalLifecycleSpell(aura->GetId()))
            SynchronizeCosmicReprisal(unit->ToPlayer());
    }

    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        if (unit && application && CosmicReprisalLifecycleSpell(application->GetBase()->GetId()))
            SynchronizeCosmicReprisal(unit->ToPlayer());
    }
};
