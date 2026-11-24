#include "Area52ArcanistRules.h"

namespace Area52Arcanist
{
bool Eligible(Player* player)
{
    return AscensionFreePick::Applies(player) && player->HasAura(Enchant, player->GetGUID()) &&
        player->HasActiveSpell(Meteor);
}

void Synchronize(Player* player)
{
    if (AscensionFreePick::Applies(player) && !Eligible(player))
        Clear(player);
}

bool Connected()
{
    auto const* info = sSpellMgr->GetSpellInfo(Enchant);
    if (!info || !info->Effects[EFFECT_0].IsAura(SPELL_AURA_DUMMY))
        return false;
    std::vector<std::pair<uint32, std::string>> required = {
        {Enchant, "aura_area52_arcanist_equipment"}, {Potential, "aura_area52_arcanist_potential"}};
    for (auto id : Resonances)
        required.emplace_back(id, "aura_area52_arcanist_resonance");
    for (auto const& [id, name] : required)
    {
        if (!sSpellMgr->GetSpellInfo(id))
            return false;
        std::list<AuraScript*> scripts;
        sScriptMgr->CreateAuraScripts(id, scripts);
        bool found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == name;
            delete script;
        }
        if (!found)
            return false;
    }
    for (auto id : Meteors)
    {
        if (!sSpellMgr->GetSpellInfo(id))
            return false;
        std::list<SpellScript*> scripts;
        sScriptMgr->CreateSpellScripts(id, scripts);
        bool found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == "spell_area52_arcanist_meteor";
            delete script;
        }
        if (!found)
            return false;
    }
    for (auto const& group : {Motes, Cooldowns})
        for (auto id : group)
            if (!sSpellMgr->GetSpellInfo(id))
                return false;
    return sSpellMgr->GetSpellInfo(Meteor) != nullptr;
}
}

class area52_arcanist_casts : public AllSpellScript
{
public:
    area52_arcanist_casts() : AllSpellScript("area52_arcanist_casts", {ALLSPELLHOOK_ON_BEFORE_EFFECTS}) { }

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        Unit* target = spell ? spell->m_targets.GetUnitTarget() : nullptr;
        if (!spell || !info || !Area52Arcanist::Eligible(player) || spell->IsTriggered() ||
            info->IsPositive() || !target || !player->IsValidAttackTarget(target) ||
            spell->GetScriptValue(Area52Arcanist::Enchant))
            return;
        int const index = Area52Arcanist::Element(info->SchoolMask);
        if (index < 0 || player->HasAura(Area52Arcanist::Cooldowns[index], player->GetGUID()))
            return;
        spell->SetScriptValue(Area52Arcanist::Enchant, 1);
        player->CastSpell(player, Area52Arcanist::Cooldowns[index], true);
        if (Area52Arcanist::CurrentMote(player) < 0)
            player->CastSpell(player, Area52Arcanist::Motes[0], true);
        player->CastSpell(target, Area52Arcanist::Meteors[index], true);
    }
};

class aura_area52_arcanist_equipment : public AuraScript
{
    PrepareAuraScript(aura_area52_arcanist_equipment);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetTarget()->ToPlayer();
        if (player && (!player->GetSession() || !player->GetSession()->PlayerLogout()))
            Area52Arcanist::Clear(player);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_arcanist_equipment::Remove,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_arcanist_potential : public AuraScript
{
    PrepareAuraScript(aura_area52_arcanist_potential);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetTarget()->ToPlayer();
        if (Area52Arcanist::Eligible(player))
            Area52Arcanist::Advance(player, GetStackAmount());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_arcanist_potential::Apply,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

class aura_area52_arcanist_resonance : public AuraScript
{
    PrepareAuraScript(aura_area52_arcanist_resonance);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* owner = GetTarget();
        for (unsigned index = 0; index < Area52Arcanist::Resonances.size(); ++index)
            if (GetId() == Area52Arcanist::Resonances[index])
                owner->RemoveAurasDueToSpell(Area52Arcanist::Cooldowns[index], owner->GetGUID());
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_arcanist_resonance::Remove,
            EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_arcanist_meteor : public SpellScript
{
    PrepareSpellScript(spell_area52_arcanist_meteor);

    uint32 _targets = 1;

    bool Load() override
    {
        return AscensionFreePick::Applies(GetCaster() ? GetCaster()->ToPlayer() : nullptr);
    }

    void Targets(std::list<WorldObject*>& targets)
    {
        WorldObject* primary = GetExplTargetUnit();
        bool const included = primary && std::find(targets.begin(), targets.end(), primary) != targets.end();
        if (included)
            targets.remove(primary);
        Acore::Containers::RandomResize(targets, included ? 4 : 5);
        if (included)
            targets.push_front(primary);
        _targets = std::max<uint32>(1, targets.size());
    }

    void Hit()
    {
        SetHitDamage(GetHitDamage() / int32(_targets));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_area52_arcanist_meteor::Targets,
            EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnHit += SpellHitFn(spell_area52_arcanist_meteor::Hit);
    }
};

class area52_arcanist_metadata : public GlobalScript
{
public:
    area52_arcanist_metadata() : GlobalScript("area52_arcanist_metadata") { }

    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!Area52MysticRealm::UsesFreePickEngine())
            return;
        if (info->Id == Area52Arcanist::Enchant)
        {
            info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
            info->Effects[EFFECT_0].TriggerSpell = 0;
            info->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
            info->Effects[EFFECT_1].Effect = 0;
        }
        for (unsigned index = 0; index < Area52Arcanist::Resonances.size(); ++index)
        {
            if (info->Id == Area52Arcanist::Resonances[index])
            {
                info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
                info->Effects[EFFECT_2].MiscValue = SPELL_SCHOOL_MASK_MAGIC & ~Area52Arcanist::Schools[index];
            }
        }
    }
};
