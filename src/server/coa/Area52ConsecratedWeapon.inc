bool ConsecratedWeaponRegistered = false;
constexpr uint32 ConsecratedWeaponAura = 977848;
constexpr uint32 ConsecratedWeaponDamage = 977827;

bool ConsecratedWeaponReady()
{
    return Enabled && ConsecratedWeaponRegistered && sSpellMgr->GetSpellInfo(ConsecratedWeaponAura) &&
        sSpellMgr->GetSpellInfo(ConsecratedWeaponDamage);
}

bool ConsecratedWeaponMelee(Player const* player, SpellInfo const* info)
{
    if (!info || info->Id == 6603 || info->Id == 75 || info->Id == ConsecratedWeaponDamage ||
        info->DmgClass == SPELL_DAMAGE_CLASS_RANGED)
        return false;
    if (info->DmgClass == SPELL_DAMAGE_CLASS_MELEE)
        return true;
    for (uint32 origin : player->GetSpellInteractionOrigins(info->Id))
        if (origin != 6603 && origin != 75)
            if (SpellInfo const* original = sSpellMgr->GetSpellInfo(origin))
                if (original->DmgClass == SPELL_DAMAGE_CLASS_MELEE)
                    return true;
    return false;
}

class ConsecratedWeaponHits final : public AllSpellScript
{
public:
    ConsecratedWeaponHits() : AllSpellScript("Area52ConsecratedWeaponHits", {ALLSPELLHOOK_ON_HIT_RESULT}) { }
    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!ConsecratedWeaponReady() || !Applies(player) || !target || !target->IsAlive() || !player->IsValidAttackTarget(target) ||
            miss != SPELL_MISS_NONE || !damage || spell->IsTriggered() || spell->IsAutoRepeat() ||
            !ConsecratedWeaponMelee(player, spell->GetSpellInfo()))
            return;
        WeaponAttackType attack = spell->GetScriptMeleeAttackType();
        if (attack == MAX_ATTACK)
            attack = spell->GetSpellInfo()->DmgClass == SPELL_DAMAGE_CLASS_MELEE &&
                spell->GetSpellInfo()->HasAttribute(SPELL_ATTR3_REQUIRES_OFF_HAND_WEAPON) ? OFF_ATTACK : BASE_ATTACK;
        if (attack != BASE_ATTACK && attack != OFF_ATTACK)
            return;
        Item* weapon = player->GetWeaponForAttack(attack, true);
        if (!weapon || weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) != 45)
            return;
        for (AuraEffect const* effect : player->GetAuraEffectsByType(SPELL_AURA_PROC_TRIGGER_SPELL))
        {
            Aura* aura = effect->GetBase();
            if (effect->GetId() != ConsecratedWeaponAura || aura->GetCastItemGUID() != weapon->GetGUID() ||
                aura->GetCasterGUID() != player->GetGUID())
                continue;
            uint64 const now = uint64(GameTime::GetGameTimeMS().count());
            uint64 const ready = aura->GetScriptValue(ConsecratedWeaponDamage);
            if (ready && now < ready)
                return;
            float chance = player->GetPPMProcChance(player->GetAttackTime(attack), 12.0f, aura->GetSpellInfo());
            player->ApplySpellMod(ConsecratedWeaponAura, SPELLMOD_CHANCE_OF_SUCCESS, chance);
            if (!roll_chance_f(chance))
                return;
            aura->SetScriptValue(ConsecratedWeaponDamage, now + 750);
            player->CastSpell(target, ConsecratedWeaponDamage,
                TRIGGERED_FULL_MASK, weapon, effect);
            return;
        }
    }
};
