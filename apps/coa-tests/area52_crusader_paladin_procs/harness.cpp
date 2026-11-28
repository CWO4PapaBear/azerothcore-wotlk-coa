#include <cassert>
#include <cstdint>
#include <vector>

using uint32 = std::uint32_t;
using int32 = std::int32_t;
constexpr uint32 SPELL_PALADIN_JUDGEMENTS_OF_THE_WISE_MANA = 31930;
constexpr uint32 SPELL_PALADIN_REPLENISHMENT = 57669;
constexpr uint32 SPELL_PALADIN_RIGHTEOUS_VENGEANCE_DOT = 61840;
constexpr int SPELLVALUE_BASE_POINT0 = 0;
constexpr int EFFECT_0 = 0;
constexpr int SPELL_AURA_PERIODIC_DAMAGE = 3;
int CalculatePct(int value, int percent) { return value * percent / 100; }
struct Unit;
struct Effect { int CalcValue(Unit*) const { return 25; } };
struct SpellInfo { uint32 Id = 31930; Effect Effects[3]; };
struct AuraEffect { int Amount; int GetAmount() const { return Amount; } };
struct Unit
{
    bool Area52 = true;
    bool Strength = false;
    int BaseMana = 1000;
    int MaximumMana = 9000;
    std::vector<uint32> Casts;
    int CustomAmount = 0;
    Unit* Owner = nullptr;
    Unit* ToPlayer() { return this; }
    bool HasAura(uint32 id) const { return id == 84864 && Strength; }
    int GetCreateMana() const { return BaseMana; }
    int GetMaxPower(int) const { return MaximumMana; }
    int GetLevel() const { return 80; }
    void CastSpell(Unit* target, uint32 id, bool triggered, void*, AuraEffect const*)
    { assert(target == this && triggered); Casts.push_back(id); }
    void CastCustomSpell(uint32 id, int, int amount, Unit* target, bool triggered, void*, AuraEffect const*)
    { CastSpell(target, id, triggered, nullptr, nullptr); CustomAmount = amount; }
    void CastDelayedSpellWithPeriodicAmount(Unit* owner, uint32 id, int aura, int amount)
    { assert(aura == 3); Owner = owner; Casts.push_back(id); CustomAmount = amount; }
};
namespace AscensionFreePick { bool Applies(Unit* u) { return u && u->Area52; } }
struct DamageInfo { uint32 Amount; uint32 GetDamage() const { return Amount; } };
struct ProcEventInfo
{
    Unit* Target;
    DamageInfo* Damage;
    Unit* GetActionTarget() const { return Target; }
    DamageInfo* GetDamageInfo() const { return Damage; }
};
struct Manager
{
    uint32 Twin = 31930;
    SpellInfo Info;
    uint32 GetSpellTwinFor(uint32, uint32) const { return Twin; }
    SpellInfo const* AssertSpellInfo(uint32) const { return &Info; }
} manager;
Manager* sSpellMgr = &manager;
struct Base
{
    Unit* Target;
    uint32 Id;
    bool Prevented = false;
    Unit* GetTarget() const { return Target; }
    uint32 GetId() const { return Id; }
    void PreventDefaultAction() { Prevented = true; }
};
struct Wise : Base { @WISE@ };
struct Vengeance : Base { @VENGEANCE@ };
int Mana(Unit* unitTarget, int damage)
{
    Unit* m_caster = unitTarget;
    SpellInfo info;
    SpellInfo const* m_spellInfo = &info;
    int power = 0;
    @MANA@
    (void)level_multiplier;
    (void)level_diff;
    return damage;
}
int main()
{
    for (auto rank : {31876u,31877u,31878u})
    {
        Unit caster;
        Wise wise{{&caster,rank,false}};
        AuraEffect effect{0};
        ProcEventInfo event{nullptr,nullptr};
        wise.HandleProc(&effect,event);
        assert(wise.Prevented && caster.Casts == std::vector<uint32>({31930,57669}));
        assert(caster.CustomAmount == 0);
        for (int mana : {0,1,1000,4000})
        {
            caster.BaseMana = mana;
            assert(Mana(&caster,15) == mana*15/100);
            caster.MaximumMana = 30000;
            assert(Mana(&caster,15) == mana*15/100);
        }
    }
    Unit reborn;
    manager.Twin = 1131930;
    Wise twin{{&reborn,1131876,false}};
    AuraEffect effect{0}; ProcEventInfo unused{nullptr,nullptr};
    twin.HandleProc(&effect,unused);
    assert(reborn.CustomAmount == 25 && reborn.Casts == std::vector<uint32>({31930,57669}));
    manager.Twin = 31930;
    for (int percent : {7,14,20})
        for (bool area52 : {false,true})
            for (bool strength : {false,true})
            {
                Unit caster, target;
                caster.Area52 = area52; caster.Strength = strength;
                DamageInfo damage{400}; ProcEventInfo event{&target,&damage}; AuraEffect aura{percent};
                Vengeance vengeance{{&caster,53380,false}};
                vengeance.HandleProc(&aura,event);
                assert(vengeance.Prevented && target.Owner == &caster);
                assert(target.Casts == std::vector<uint32>{61840});
                assert(target.CustomAmount == percent * ((area52 && strength) ? 2 : 1));
                target.Casts.clear(); damage.Amount = 0;
                vengeance.HandleProc(&aura,event); assert(target.Casts.empty());
                event.Damage = nullptr;
                vengeance.HandleProc(&aura,event); assert(target.Casts.empty());
                event.Target = nullptr;
                vengeance.HandleProc(&aura,event); assert(target.Casts.empty());
            }
}
