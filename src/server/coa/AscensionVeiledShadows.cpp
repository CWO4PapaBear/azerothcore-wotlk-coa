#include "Area52VeiledShadows.h"
#include "AscensionFreePick.h"
#include "Player.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellScript.h"

namespace
{
class spell_area52_veiled_shadows : public SpellScript
{
    PrepareSpellScript(spell_area52_veiled_shadows);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({15274, 15311, 954696}); }
    void After()
    {
        Player* player = GetCaster()->ToPlayer();
        float chance = AscensionFreePick::VeiledShadowsProcChance(player);
        if (chance <= 0 || !roll_chance_f(chance))
            return;
        player->CastSpell(player, 954696, true);
        if (Aura* aura = player->GetAura(954696, player->GetGUID()))
        {
            aura->SetMaxDuration(2000);
            aura->SetDuration(2000);
        }
    }
    void Register() override { AfterCast += SpellCastFn(spell_area52_veiled_shadows::After); }
};
}

void AddAscensionVeiledShadowsScripts()
{
    RegisterSpellScript(spell_area52_veiled_shadows);
}
