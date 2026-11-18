#include "AscensionFreePick.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include <algorithm>
#include <limits>

namespace
{
constexpr uint32 Enchant = 414100;
constexpr uint32 Extension = 414101;
constexpr uint32 Heal = 414103;
constexpr uint32 Dance = 51713;
constexpr uint32 Step = 36554;

bool Eligible(Unit* owner)
{
    return owner && owner->IsAlive() && AscensionFreePick::Applies(owner->ToPlayer()) && owner->HasAura(Enchant);
}

class spell_area52_embracing_dance : public SpellScript
{
    PrepareSpellScript(spell_area52_embracing_dance);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({Enchant, Dance, Step}); }
    void After()
    {
        if (Eligible(GetCaster()) && GetCaster()->HasAura(Dance))
            GetCaster()->ToPlayer()->RemoveSpellCooldown(Step, true);
    }
    void Register() override { AfterCast += SpellCastFn(spell_area52_embracing_dance::After); }
};

class aura_area52_embracing_shadows : public AuraScript
{
    PrepareAuraScript(aura_area52_embracing_shadows);
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({Enchant, Extension, Heal, Dance}); }
    bool Check(ProcEventInfo& event)
    {
        Unit* owner = GetTarget();
        DamageInfo* damage = event.GetDamageInfo();
        return Eligible(owner) && owner->HasAura(Dance) && event.GetActionTarget() == owner &&
            damage && damage->GetVictim() == owner && damage->GetDamage() && damage->GetSpellInfo() &&
            event.GetActor() && event.GetActor() != owner && damage->GetAttacker() == event.GetActor();
    }
    void Proc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        if (!Check(event))
            return;
        Unit* owner = GetTarget();
        owner->CastSpell(owner, Extension, true, nullptr, effect);
        owner->CastSpell(owner, Heal, true, nullptr, effect);
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_embracing_shadows::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_embracing_shadows::Proc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_embracing_extension : public SpellScript
{
    PrepareSpellScript(spell_area52_embracing_extension);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({Enchant, Dance}); }
    void Extend(SpellEffIndex index)
    {
        PreventHitDefaultEffect(index);
        Unit* owner = GetCaster();
        if (!Eligible(owner) || GetHitUnit() != owner)
            return;
        Aura* dance = owner->GetAura(Dance);
        if (!dance || dance->GetDuration() <= 0 || GetEffectValue() <= 0)
            return;
        int32 duration = int32(std::min<int64>(int64(dance->GetDuration()) + GetEffectValue(),
            std::numeric_limits<int32>::max()));
        if (duration > dance->GetMaxDuration())
            dance->SetMaxDuration(duration);
        dance->SetDuration(duration);
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_embracing_extension::Extend, EFFECT_0,
            SPELL_EFFECT_ASCENSION_REFRESH_AURA);
    }
};
}

void AddAscensionEmbracingShadowsScripts()
{
    RegisterSpellScript(spell_area52_embracing_dance);
    RegisterSpellScript(aura_area52_embracing_shadows);
    RegisterSpellScript(spell_area52_embracing_extension);
}
