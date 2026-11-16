class spell_area52_legendary_proc : public AuraScript
{
    PrepareAuraScript(spell_area52_legendary_proc);

    bool Load() override
    {
        Player* player = GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr;
        return player && AscensionFreePick::Applies(player);
    }

    bool CheckProc(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        SpellInfo const* spell = event.GetSpellInfo();
        if (GetId() == Area52Legendary::Deathbringer && spell &&
            (spell->Id == 983902 || spell->Id == 983904 || spell->Id == 983905 ||
            spell->Id == 983906 || spell->Id == 983907))
            return false;
        return Area52Legendary::Accept(GetId(), player && AscensionFreePick::Applies(player),
            event.GetActor() == player && event.GetActionTarget() && event.GetActionTarget() != player,
            event.GetDamageInfo() && event.GetDamageInfo()->GetDamage() > 0,
            (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) != 0, spell && spell->IsAffectingArea());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_legendary_proc::CheckProc);
    }
};

class spell_area52_unbounded_deathbringer : public AuraScript
{
    PrepareAuraScript(spell_area52_unbounded_deathbringer);

    std::map<ObjectGuid, TimePoint> _targetCooldowns;

    bool Load() override
    {
        return GetUnitOwner() && AscensionFreePick::Applies(GetUnitOwner()->ToPlayer());
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({983905, 983906, 983907});
    }

    bool CheckProc(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        SpellInfo const* spell = event.GetSpellInfo();
        if (spell && (spell->Id == 983902 || spell->Id == 983904 || spell->Id == 983905 ||
            spell->Id == 983906 || spell->Id == 983907))
            return false;
        return Area52Legendary::Accept(GetId(), player && AscensionFreePick::Applies(player),
            event.GetActor() == player && event.GetActionTarget() && event.GetActionTarget() != player,
            event.GetDamageInfo() && event.GetDamageInfo()->GetDamage() > 0,
            (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) != 0, spell && spell->IsAffectingArea());
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        Unit* center = event.GetActionTarget();
        if (!player || !center || !center->IsInWorld())
            return;

        TimePoint const now = GameTime::SteadyNow();
        for (auto it = _targetCooldowns.begin(); it != _targetCooldowns.end();)
            if (it->second <= now)
                it = _targetCooldowns.erase(it);
            else
                ++it;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(center, player, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(center, targets, check);
        Cell::VisitObjects(center, searcher, 8.0f);
        targets.push_front(center);
        for (Unit* target : targets)
        {
            if (!target || !target->IsAlive() || !player->IsValidAttackTarget(target) ||
                !center->IsWithinDistInMap(target, 8.0f) || !center->IsWithinLOSInMap(target) ||
                _targetCooldowns.count(target->GetGUID()))
                continue;
            _targetCooldowns.emplace(target->GetGUID(), now + Milliseconds(4000));
            player->CastSpell(target, 983905, true, nullptr, GetEffect(EFFECT_0));
            player->CastSpell(target, 983906, true, nullptr, GetEffect(EFFECT_0));
            player->CastSpell(player, 983907, true, nullptr, GetEffect(EFFECT_0));
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_unbounded_deathbringer::CheckProc);
        OnProc += AuraProcFn(spell_area52_unbounded_deathbringer::Proc);
    }
};

bool HasArea52UnboundedDeathbringerHandler()
{
    for (uint32 child : {983905u, 983906u, 983907u})
        if (!sSpellMgr->GetSpellInfo(child))
            return false;
    std::list<AuraScript*> scripts;
    sScriptMgr->CreateAuraScripts(Area52Legendary::UnboundedDeathbringer, scripts);
    bool found = false;
    for (AuraScript* script : scripts)
    {
        found |= *script->_GetScriptName() == "spell_area52_unbounded_deathbringer";
        delete script;
    }
    return found;
}

class area52_legendary_metadata : public GlobalScript
{
public:
    area52_legendary_metadata() : GlobalScript("area52_legendary_metadata") { }

    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (info->Id != Area52Legendary::Exorcism && info->Id != Area52Legendary::Consecration)
            return;
        if (!sConfigMgr->GetOption<bool>("CoA.Enable", true) ||
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live" ||
            (sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) & ~uint32(0x400)))
            return;
        if (info->Id == Area52Legendary::Exorcism)
            info->SpellFamilyFlags[1] |= 2;
        if (info->Id == Area52Legendary::Consecration)
            info->SpellFamilyFlags[0] |= 32;
    }
};

class spell_area52_purification_hit : public SpellScript
{
    PrepareSpellScript(spell_area52_purification_hit);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Legendary::Vulnerability});
    }

    void Hit()
    {
        Player* player = GetCaster()->ToPlayer();
        if (player && AscensionFreePick::Applies(player) && player->HasAura(Area52Legendary::Purification) &&
            GetHitUnit() && GetHitDamage() > 0)
            player->CastSpell(GetHitUnit(), Area52Legendary::Vulnerability, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_area52_purification_hit::Hit);
    }
};

class aura_area52_purification_ground : public AuraScript
{
    PrepareAuraScript(aura_area52_purification_ground);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Legendary::Vulnerability});
    }

    void Apply(AuraEffect const* effect, AuraEffectHandleModes)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (player && AscensionFreePick::Applies(player) && player->HasAura(Area52Legendary::Purification))
            player->CastSpell(GetTarget(), Area52Legendary::Vulnerability, true, nullptr, effect);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_purification_ground::Apply,
            EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_zealot_hit : public SpellScript
{
    PrepareSpellScript(spell_area52_zealot_hit);

    void Hit()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || !AscensionFreePick::Applies(player) || !GetHitUnit())
            return;
        bool const pvp = GetExplTargetUnit() && GetExplTargetUnit()->IsControlledByPlayer();
        SetHitDamage(Area52Legendary::ZealotAmount(GetHitDamage(), pvp));
        SetHitHeal(Area52Legendary::ZealotAmount(GetHitHeal(), pvp));
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_zealot_hit::Hit);
    }
};
