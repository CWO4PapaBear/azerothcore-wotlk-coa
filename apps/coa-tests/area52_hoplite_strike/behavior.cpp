#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
using uint32 = uint32_t;
using int32 = int32_t;
IMPALED_FUNCTION
int CalculatePct(int amount, uint32 pct) { return amount * pct / 100; }
struct Aura
{
    int remaining = 9000;
    int GetDuration() { return remaining; }
    int GetMaxDuration() { return 12000; }
};
struct Unit
{
    int guid = 1;
    bool player = false;
    int bleedOwner = 0;
    bool cooldownReset = false;
    Aura bleed;
    std::map<int, int> auras;
    bool HasAura(int id) { return auras[id] != 0; }
    void CastSpell(Unit* target, int id, bool)
    {
        if (id == 982251) target->bleedOwner = guid;
        else target->auras[id] = std::min(5, target->auras[id] + 1);
    }
    void RemoveAurasDueToSpell(int id) { auras.erase(id); }
    void RemoveAurasDueToSpell(int id, int owner)
    {
        assert(id == 982251);
        if (bleedOwner == owner) bleedOwner = 0;
    }
    Aura* GetAura(int id, int owner)
    {
        assert(id == 982251);
        return bleedOwner == owner ? &bleed : nullptr;
    }
    int GetGUID() { return guid; }
    Unit* GetCharmerOrOwnerPlayerOrPlayerItself() { return player ? this : nullptr; }
    Unit* ToPlayer() { return this; }
    void RemoveSpellCooldown(int id, bool) { assert(id == 982250); cooldownReset = true; }
};
struct Context
{
    Unit caster, victim;
    bool hasTarget = true;
    int damage = 100;
    Unit* GetCaster() { return &caster; }
    Unit* GetHitUnit() { return hasTarget ? &victim : nullptr; }
    int GetHitDamage() { return damage; }
    void SetHitDamage(int value) { damage = value; }
};
struct Thrust : Context { STACK_METHOD };
struct Bloodrage : Context { RESET_METHOD };
struct Javelin : Context { JAVELIN_METHOD };
struct Colossus : Context
{
    bool Impaled = false;
    COLOSSUS_DAMAGE
    COLOSSUS_FINISH
};
int main()
{
    Thrust thrust;
    thrust.Stack();
    assert(thrust.caster.auras[978539] == 0);
    thrust.caster.auras[29131] = 1;
    thrust.Stack();
    assert(thrust.caster.auras[978539] == 0);
    thrust.caster.auras[997743] = 1;
    thrust.Stack();
    assert(thrust.caster.auras[978539] == 1 && thrust.caster.auras[978537] == 1);
    Bloodrage blood;
    blood.caster.auras[978539] = 5;
    blood.caster.auras[978537] = 5;
    blood.caster.auras[123] = 1;
    blood.Reset();
    assert(!blood.caster.HasAura(978539) && !blood.caster.HasAura(978537));
    assert(blood.caster.HasAura(123));
    Javelin javelin;
    javelin.damage = 0;
    javelin.Hit();
    assert(!javelin.victim.bleedOwner);
    javelin.damage = 100;
    javelin.Hit();
    assert(javelin.victim.bleedOwner == javelin.caster.guid);
    for (bool pvp : {false, true})
    {
        Colossus smash;
        smash.victim.bleedOwner = 1;
        smash.victim.player = pvp;
        smash.Damage();
        assert(smash.damage == (pvp ? 115 : 160));
        smash.Finish();
        assert(smash.caster.cooldownReset && !smash.victim.bleedOwner);
    }
    Colossus foreign;
    foreign.victim.bleedOwner = 2;
    foreign.Damage(); foreign.Finish();
    assert(foreign.damage == 100 && !foreign.caster.cooldownReset && foreign.victim.bleedOwner == 2);
    Colossus failed;
    failed.victim.bleedOwner = 1;
    failed.damage = 0;
    failed.Damage(); failed.Finish();
    assert(!failed.caster.cooldownReset && failed.victim.bleedOwner == 1);
}
