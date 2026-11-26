static bool (*Area52ControlledTyphoonReady)() = nullptr;
void RegisterArea52ControlledTyphoonReadiness(bool (*ready)())
{
    Area52ControlledTyphoonReady = ready;
}

namespace
{
bool Area52ZephyrRegistered = false;

void ConfigureArea52Zephyr(SpellInfo* info)
{
    if (!Area52WindMetadataEnabled)
        return;
    if (info->Id == 977725)
    {
        info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
        info->Effects[EFFECT_0].TriggerSpell = 0;
    }
    if (info->Id == 414003)
        info->StackAmount = 10;
    if (info->Id == 979769)
    {
        info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
        info->Effects[EFFECT_0].TriggerSpell = 0;
        info->Effects[EFFECT_1].Effect = 0;
        info->Effects[EFFECT_2].Effect = 0;
        info->StackAmount = 1;
    }
}
}

bool HasArea52WindHandlers(uint32 spell)
{
    if (spell == 978232)
        return HasArea52WindRagerHandlers();
    if (spell == 997082)
        return HasArea52DesolateHandlers();
    if (spell == 977725)
    {
        SpellInfo const* info = sSpellMgr->GetSpellInfo(spell);
        return Area52WindMetadataEnabled && info && info->Effects[EFFECT_0].IsAura(SPELL_AURA_DUMMY) &&
            sSpellMgr->GetSpellInfo(977726) && Area52ControlledTyphoonReady && Area52ControlledTyphoonReady();
    }
    if (spell != 979769 || !Area52WindMetadataEnabled || !Area52ZephyrRegistered)
        return false;
    SpellInfo const* enchant = sSpellMgr->GetSpellInfo(979769);
    SpellInfo const* stacks = sSpellMgr->GetSpellInfo(414003);
    return enchant && stacks && enchant->Effects[EFFECT_0].IsAura(SPELL_AURA_DUMMY) &&
        stacks->StackAmount == 10 && sSpellMgr->GetSpellInfo(25504) && sSpellMgr->GetSpellInfo(33750);
}

class area52_zephyr_hits final : public AllSpellScript
{
public:
    area52_zephyr_hits() : AllSpellScript("area52_zephyr_hits", {ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player) || !player->HasAura(979769) ||
            !target || miss != SPELL_MISS_NONE || !damage)
            return;
        uint32 const id = spell->GetSpellInfo()->Id;
        if ((id == 25504 || id == 33750) && !spell->GetScriptValue(979769))
        {
            spell->SetScriptValue(979769, 1);
            player->CastSpell(player, 414003, true);
        }
    }
};

class area52_zephyr_cleanup final : public UnitScript
{
public:
    area52_zephyr_cleanup() : UnitScript("area52_zephyr_cleanup", {UNITHOOK_ON_AURA_REMOVE}) { }

    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (AscensionFreePick::Applies(player) && application && application->GetBase()->GetId() == 979769)
            player->RemoveAurasDueToSpell(414003, player->GetGUID());
    }
};
