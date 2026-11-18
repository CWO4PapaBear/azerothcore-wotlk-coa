#include "AscensionFreePick.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"

namespace
{
class aura_area52_savage_combat : public AuraScript
{
    PrepareAuraScript(aura_area52_savage_combat);

    bool Check(ProcEventInfo& event)
    {
        Unit* owner = GetUnitOwner();
        if (!AscensionFreePick::Applies(owner->ToPlayer()))
            return true;
        Unit* target = event.GetActionTarget();
        SpellInfo const* spell = event.GetSpellInfo();
        if (event.GetActor() != owner || !target || !spell || spell->Dispel != DISPEL_POISON)
            return false;
        for (auto const& entry : target->GetAppliedAuras())
            if (entry.second->GetBase()->GetCasterGUID() == owner->GetGUID()
                && entry.second->GetBase()->GetSpellInfo()->Dispel == DISPEL_POISON)
                return true;
        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_savage_combat::Check);
    }
};
}

void AddAscensionSavageCombatScripts()
{
    RegisterSpellScript(aura_area52_savage_combat);
}
