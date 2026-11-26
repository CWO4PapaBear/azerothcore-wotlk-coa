namespace
{
bool Area52DesolateRegistered = false;
bool (*Area52WindfuryReady)() = nullptr;

bool Area52DesolateEnabled(Player const* player)
{
    return AscensionFreePick::Applies(player) && player->HasAura(997082) && !player->HasAura(978231);
}

void ConfigureArea52Desolate(SpellInfo* info)
{
    if (Area52WindMetadataEnabled && info->Id == 997082)
    {
        info->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_DUMMY;
        info->Effects[EFFECT_1].TriggerSpell = 0;
        info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_DUMMY;
        info->Effects[EFFECT_2].TriggerSpell = 0;
    }
}
}

void RegisterArea52WindfuryReadiness(bool (*ready)())
{
    Area52WindfuryReady = ready;
}

void ApplyArea52DesolateWindfury(Player* player, Unit* target)
{
    if (Area52DesolateEnabled(player) && target && target->IsAlive())
    {
        player->CastSpell(player, 997084, true);
        player->CastSpell(target, 300065, true);
    }
}

bool HasArea52DesolateHandlers()
{
    SpellInfo const* enchant = sSpellMgr->GetSpellInfo(997082);
    SpellProcEntry const* charge = sSpellMgr->GetSpellProcEntry(997084);
    return Area52WindMetadataEnabled && Area52DesolateRegistered && Area52WindfuryReady &&
        Area52WindfuryReady() && charge && charge->Charges == 1 &&
        charge->SpellPhaseMask == PROC_SPELL_PHASE_CAST && (charge->AttributesMask & PROC_ATTR_REQ_SPELLMOD) &&
        (charge->ProcFlags & PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG) && enchant &&
        enchant->Effects[EFFECT_1].IsAura(SPELL_AURA_DUMMY) &&
        sSpellMgr->GetSpellInfo(997084) && sSpellMgr->GetSpellInfo(300065);
}

class area52_desolate_hits final : public AllSpellScript
{
public:
    area52_desolate_hits() : AllSpellScript("area52_desolate_hits", {ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!Area52DesolateEnabled(player) || !target || miss != SPELL_MISS_NONE || !damage ||
            sSpellMgr->GetFirstSpellInChain(spell->GetSpellInfo()->Id) != 997083 || spell->GetScriptValue(997083))
            return;
        spell->SetScriptValue(997083, 1);
        if (Aura* dust = target->GetAura(300065, player->GetGUID()); dust && dust->GetDuration() > 0)
        {
            int32 const duration = std::min<int32>(12000, dust->GetDuration() + 2000);
            dust->SetMaxDuration(std::max(dust->GetMaxDuration(), duration));
            dust->SetDuration(duration);
        }
    }
};

class area52_desolate_cleanup final : public UnitScript
{
public:
    area52_desolate_cleanup() : UnitScript("area52_desolate_cleanup", {UNITHOOK_ON_AURA_REMOVE}) { }

    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (AscensionFreePick::Applies(player) && application && application->GetBase()->GetId() == 997082)
            player->RemoveAurasDueToSpell(997084, player->GetGUID());
    }
};
