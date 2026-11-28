#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

using uint32 = std::uint32_t;
using SpellEffIndex = int;
using AuraEffectHandleModes = int;
enum AuraRemoveMode { AURA_REMOVE_BY_DEFAULT, AURA_REMOVE_BY_EXPIRE, AURA_REMOVE_BY_ENEMY_SPELL,
    AURA_REMOVE_BY_DEATH, AURA_REMOVE_BY_CANCEL };
struct AuraEffect { };
struct SpellInfo { uint32 Id; };
struct Unit
{
    bool Friendly = false;
    bool Hostile = false;
    bool Alive = true;
    std::vector<uint32> Casts;
    Unit* LastTarget = nullptr;
    bool IsFriendlyTo(Unit*) const { return Friendly; }
    bool IsValidAttackTarget(Unit*) const { return Hostile; }
    bool IsAlive() const { return Alive; }
    void CastSpell(Unit* target, uint32 spell, bool triggered, void* = nullptr, AuraEffect const* = nullptr)
    {
        assert(triggered);
        LastTarget = target;
        Casts.push_back(spell);
    }
};
struct Application
{
    AuraRemoveMode Mode = AURA_REMOVE_BY_DEFAULT;
    AuraRemoveMode GetRemoveMode() const { return Mode; }
};

@RANKS@

struct Active
{
    Unit* Caster = nullptr;
    Unit* Target = nullptr;
    SpellInfo Info{};
    Unit* GetCaster() const { return Caster; }
    Unit* GetHitUnit() const { return Target; }
    SpellInfo const* GetSpellInfo() const { return &Info; }
    @ACTIVE@
};

struct Removal
{
    Unit* Caster = nullptr;
    Unit* Target = nullptr;
    Application App;
    uint32 Id = 0;
    Unit* GetCaster() const { return Caster; }
    Unit* GetTarget() const { return Target; }
    Application* GetTargetApplication() { return &App; }
    uint32 GetId() const { return Id; }
    @REMOVAL@
};

int main()
{
    constexpr std::array<std::array<uint32, 5>, 7> expected{{
        {954817,954818,954827,954819,954828},
        {954972,954979,954992,954985,954998},
        {954973,954980,954993,954986,954999},
        {954974,954981,954994,954987,955000},
        {954975,954982,954995,954988,955001},
        {954976,954983,954996,954989,955002},
        {954977,954984,954997,954991,955003}
    }};
    for (auto const& rank : expected)
    {
        Unit caster, target;
        Active active{&caster, &target, {rank[0]}};
        caster.Friendly = true;
        active.HandleDummy(0);
        assert(caster.Casts == std::vector<uint32>{rank[3]} && caster.LastTarget == &target);
        caster.Casts.clear();
        caster.Friendly = false;
        caster.Hostile = true;
        active.HandleDummy(0);
        assert(caster.Casts == std::vector<uint32>{rank[1]});
        caster.Casts.clear();
        caster.Hostile = false;
        active.HandleDummy(0);
        assert(caster.Casts.empty());
        active.Target = nullptr;
        active.HandleDummy(0);
        assert(caster.Casts.empty());
        active.Target = &target;
        active.Info.Id = 12345;
        active.HandleDummy(0);
        assert(caster.Casts.empty());
        for (bool friendly : {false, true})
        {
            uint32 const periodic = friendly ? rank[3] : rank[1];
            uint32 const burst = friendly ? rank[4] : rank[2];
            for (auto mode : {AURA_REMOVE_BY_DEFAULT, AURA_REMOVE_BY_EXPIRE, AURA_REMOVE_BY_ENEMY_SPELL,
                AURA_REMOVE_BY_DEATH, AURA_REMOVE_BY_CANCEL})
            {
                Removal removal{&caster, &target, {mode}, periodic};
                caster.Casts.clear();
                removal.HandleRemove(nullptr, 0);
                bool const allowed = mode == AURA_REMOVE_BY_EXPIRE || mode == AURA_REMOVE_BY_ENEMY_SPELL;
                assert(caster.Casts.size() == (allowed ? 1u : 0u));
                if (allowed)
                    assert(caster.Casts[0] == burst && caster.LastTarget == &target);
                caster.Casts.clear();
                target.Alive = false;
                removal.HandleRemove(nullptr, 0);
                assert(caster.Casts.empty());
                target.Alive = true;
                removal.Caster = nullptr;
                removal.HandleRemove(nullptr, 0);
                assert(caster.Casts.empty());
                removal.Caster = &caster;
                removal.Id = 12345;
                removal.HandleRemove(nullptr, 0);
                assert(caster.Casts.empty());
            }
        }
    }
    assert(!FindExecutionSentenceRank(0));
    assert(!FindExecutionSentenceRank(954827));
}
