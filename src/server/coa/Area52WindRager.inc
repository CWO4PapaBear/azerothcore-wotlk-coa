namespace
{
bool Area52WindRagerRegistered = false;

uint32 Area52WindStrikeKind(uint32 id)
{
    return Area52Wind::StrikeKind(id);
}

void ConfigureArea52WindRager(SpellInfo* info)
{
    if (!Area52WindMetadataEnabled)
        return;
    if (info->Id == 978232)
    {
        info->ProcFlags = PROC_FLAG_DONE_MELEE_AUTO_ATTACK | PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS |
            PROC_FLAG_DONE_RANGED_AUTO_ATTACK | PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS |
            PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG;
        info->ProcChance = 10;
    }
    if (info->Id == 978938 || info->Id == 978939)
    {
        info->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_DUMMY;
        info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_DUMMY;
        if (info->Id == 978938)
        {
            info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
            info->StackAmount = 10;
        }
    }
    if (Area52WindStrikeKind(info->Id))
    {
        info->Effects[EFFECT_0].BasePoints = -1;
        info->Effects[EFFECT_0].RealPointsPerLevel = 0;
    }

}

void AddArea52RisingWinds(Player* player, uint32 count)
{
    if (!AscensionFreePick::Applies(player) || !player->HasAura(978232))
        return;
    for (uint32 n = 0; n < count; ++n)
        player->CastSpell(player, 978938, true);
    if (Aura* winds = player->GetAura(978938, player->GetGUID()); winds && winds->GetStackAmount() >= 10)
    {
        player->RemoveAurasDueToSpell(978938, player->GetGUID());
        player->CastSpell(player, 978939, true);
    }
}
}

bool HasArea52WindRagerHandlers()
{
    SpellInfo const* winds = sSpellMgr->GetSpellInfo(978938);
    if (!Area52WindMetadataEnabled || !Area52WindRagerRegistered || !winds || winds->StackAmount != 10 ||
        !sSpellMgr->GetSpellInfo(978939) || !sSpellMgr->GetSpellInfo(983986) || !sSpellMgr->GetSpellInfo(983974))
        return false;
    SpellProcEntry const* proc = sSpellMgr->GetSpellProcEntry(978232);
    if (!proc || proc->Chance != 10.0f || proc->Cooldown != Milliseconds(6000) || proc->DisableEffectsMask != 6)
        return false;
    uint32 const script = sObjectMgr->GetScriptId("aura_area52_wind_rager");
    if (!script || !ScriptRegistry<SpellScriptLoader>::GetScriptById(script))
        return false;
    auto const [first, last] = sObjectMgr->GetSpellScriptsBounds(978232);
    return std::any_of(first, last, [script](auto const& entry) { return entry.second == script; });
}

class aura_area52_wind_rager final : public AuraScript
{
    PrepareAuraScript(aura_area52_wind_rager);

    bool CheckProc(ProcEventInfo& event)
    {
        DamageInfo const* damage = event.GetDamageInfo();
        return AscensionFreePick::Applies(GetTarget()->ToPlayer()) && event.GetActor() == GetTarget() &&
            damage && damage->GetDamage() && damage->GetDamageType() != DOT &&
            damage->GetSchoolMask() == SPELL_SCHOOL_MASK_NORMAL;
    }

    void HandleProc(AuraEffect const*, ProcEventInfo&)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), 983986, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_wind_rager::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_area52_wind_rager::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class area52_wind_rager_casts final : public AllSpellScript
{
public:
    area52_wind_rager_casts() : AllSpellScript("area52_wind_rager_casts",
        {ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_CALCULATED_TARGET, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player) || !player->HasAura(978232) || spell->IsTriggered() ||
            spell->GetScriptValue(978232))
            return;
        spell->SetScriptValue(978232, 1);
        if (player->MatchesSpellInteraction(info->Id, 1680))
            AddArea52RisingWinds(player, 2);
        if (sSpellMgr->GetFirstSpellInChain(info->Id) == 983982)
            player->RemoveAurasDueToSpell(983986, player->GetGUID());
    }

    void OnSpellCalculatedTarget(Spell* spell, Unit* target, TargetInfo& info) override
    {
        Player* player = spell->GetCaster()->ToPlayer();
        if (!AscensionFreePick::Applies(player) || !player->HasAura(978232) || !target || info.damage <= 0)
            return;
        uint32 const kind = Area52WindStrikeKind(spell->GetSpellInfo()->Id);
        float multiplier = 1.0f;
        if (kind == 1)
            if (Aura* winds = player->GetAura(978938, player->GetGUID()))
                multiplier *= 1.0f + winds->GetStackAmount() * (target->IsPlayer() ? 0.02f : 0.04f);
        if ((kind || player->MatchesSpellInteraction(spell->GetSpellInfo()->Id, 1680)) && player->HasAura(978939))
            multiplier *= 1.5f;
        info.damage = int32(info.damage * multiplier);
        info.damageBeforeTakenMods = int32(info.damageBeforeTakenMods * multiplier);
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool critical) override
    {
        Player* player = spell->GetCaster()->ToPlayer();
        if (!AscensionFreePick::Applies(player) || !player->HasAura(978232) || !target ||
            miss != SPELL_MISS_NONE || spell->GetScriptValue(978938))
            return;
        uint32 const id = spell->GetSpellInfo()->Id;
        if (Area52Wind::OffhandStrike(id) && !player->GetWeaponForAttack(OFF_ATTACK, true))
            return;
        uint32 const kind = Area52WindStrikeKind(id);
        if (!kind)
            return;
        spell->SetScriptValue(978938, 1);
        if (kind == 1 && critical)
            for (uint32 whirlwind : sSpellMgr->GetSpellAndRelatives(1680))
                player->ModifySpellCooldown(whirlwind, -4000);
        AddArea52RisingWinds(player, 1);
    }
};

class area52_wind_rager_amount final : public UnitScript
{
public:
    area52_wind_rager_amount() : UnitScript("area52_wind_rager_amount", {UNITHOOK_ON_AFTER_AURA_EFFECT_CALCULATE_AMOUNT}) { }

    void OnAfterAuraEffectCalculateAmount(AuraEffect const* effect, Unit* caster, int32& amount) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (AscensionFreePick::Applies(player) && effect->GetId() == 983974 && effect->GetEffIndex() == EFFECT_0 &&
            player->HasAura(46917))
            amount = 10;
    }
};

class area52_wind_rager_cleanup final : public UnitScript
{
public:
    area52_wind_rager_cleanup() : UnitScript("area52_wind_rager_cleanup", {UNITHOOK_ON_AURA_REMOVE}) { }

    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (AscensionFreePick::Applies(player) && application && application->GetBase()->GetId() == 978232)
            for (uint32 helper : {978938u, 978939u, 983986u, 983974u})
                player->RemoveAurasDueToSpell(helper, player->GetGUID());
    }
};
