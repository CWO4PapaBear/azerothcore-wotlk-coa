#include "AscensionFreePick.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"

namespace
{
constexpr uint32 Enchant = 954634;
constexpr uint32 Nova = 954635;

class aura_area52_blinding_dispersion : public AuraScript
{
    PrepareAuraScript(aura_area52_blinding_dispersion);
    bool Load() override { return AscensionFreePick::Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({Enchant, Nova}); }
    void Removed(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* owner = GetTarget();
        auto mode = GetTargetApplication()->GetRemoveMode();
        if (!owner || !owner->IsAlive() || !owner->IsInWorld()
            || !AscensionFreePick::Applies(owner->ToPlayer()) || !owner->HasAura(Enchant)
            || (mode != AURA_REMOVE_BY_EXPIRE && mode != AURA_REMOVE_BY_CANCEL
                && mode != AURA_REMOVE_BY_ENEMY_SPELL))
            return;
        owner->CastSpell(owner, Nova, true);
    }
    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_blinding_dispersion::Removed,
            EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
    }
};
}

void AddAscensionBlindingDispersionScripts()
{
    RegisterSpellScript(aura_area52_blinding_dispersion);
}
