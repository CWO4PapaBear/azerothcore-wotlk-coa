#ifndef AREA52_HOLY_CONTRACTS_H
#define AREA52_HOLY_CONTRACTS_H
#include "SpellInfo.h"
namespace Area52Holy
{
inline bool HealingSpell(SpellInfo const* spell)
{
    return spell->HasEffect(SPELL_EFFECT_HEAL) || spell->HasEffect(SPELL_EFFECT_HEAL_PCT) ||
        spell->HasEffect(SPELL_EFFECT_HEAL_MAX_HEALTH) || spell->HasAura(SPELL_AURA_PERIODIC_HEAL);
}
inline bool DamagingSpell(SpellInfo const* spell)
{
    return spell->HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE) || spell->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE) ||
        spell->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL) || spell->HasEffect(SPELL_EFFECT_WEAPON_PERCENT_DAMAGE) ||
        spell->HasEffect(SPELL_EFFECT_NORMALIZED_WEAPON_DMG) || spell->HasEffect(SPELL_EFFECT_HEALTH_LEECH) ||
        spell->HasAura(SPELL_AURA_PERIODIC_DAMAGE) || spell->HasAura(SPELL_AURA_PERIODIC_LEECH);
}
inline constexpr uint64 FocusMechanics = IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK |
    (uint64(1) << MECHANIC_SILENCE) | (uint64(1) << MECHANIC_INTERRUPT);
inline bool FocusControl(SpellInfo const* spell)
{
    if (!spell)
        return false;
    if (spell->GetAllEffectsMechanicMask() & FocusMechanics)
        return true;
    return spell->HasEffect(SPELL_EFFECT_KNOCK_BACK) || spell->HasEffect(SPELL_EFFECT_KNOCK_BACK_DEST);
}
}
#endif
