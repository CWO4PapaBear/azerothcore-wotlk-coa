/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"

class aura_area52_spartan_kick : public AuraScript
{
    PrepareAuraScript(aura_area52_spartan_kick);

    bool Load() override
    {
        Player* player = GetUnitOwner()->ToPlayer();
        if (!player || player->getClass() != CLASS_HERO || !sConfigMgr->GetOption<bool>("CoA.Enable", true) ||
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live")
            return false;
        auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
        return !mask || !(*mask & ~uint32(0x400));
    }

    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({1766, 84446}); }

    bool Check(ProcEventInfo& event)
    {
        Unit* owner = GetTarget();
        Unit* target = event.GetActionTarget();
        SpellInfo const* spell = event.GetSpellInfo();
        return event.GetActor() == owner && target && owner->IsValidAttackTarget(target) &&
            spell && spell->GetFirstRankSpell()->Id == 1766 &&
            (owner->HasAura(2457) || owner->HasAura(71) || owner->HasAura(997743));
    }

    void Handle(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(event.GetActionTarget(), 84446, true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_spartan_kick::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_spartan_kick::Handle,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

void AddAscensionSpartanKickScripts()
{
    RegisterSpellScript(aura_area52_spartan_kick);
}
