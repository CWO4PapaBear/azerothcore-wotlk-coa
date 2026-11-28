#include "Area52CrusaderAbilityRules.h"
#include <array>
#include <cassert>
#include <cstdint>

constexpr int EFFECT_0 = 0;
constexpr int EFFECT_1 = 1;
constexpr int SPELL_SCHOOL_MASK_ALL = 127;
constexpr int SPELL_AURA_MOD_HEALING_RECEIVED = 283;
constexpr int TARGET_UNIT_CASTER_AREA_RAID = 56;

struct SpellImplicitTargetInfo
{
    int Target = 30;
    SpellImplicitTargetInfo() = default;
    explicit SpellImplicitTargetInfo(int target) : Target(target) { }
};

struct Effect
{
    int MiscValue = 2;
    int ApplyAuraName = 271;
    int BasePoints = 7;
    int Radius = 30;
    SpellImplicitTargetInfo TargetB;
};

struct SpellInfo
{
    std::uint32_t Id;
    Effect Effects[3]{};
    int School = 2;
    int Speed = 20;
    int GetSchoolMask() const { return School; }
};

struct Caster
{
    bool Area52 = true;
    int Guid = 1;
    Caster* ToPlayer() { return this; }
    int GetGUID() const { return Guid; }
};

namespace AscensionFreePick
{
bool Applies(Caster* caster) { return caster && caster->Area52; }
}

struct AuraEffect
{
    std::uint32_t Id;
    int Guid = 1;
    bool Affected = false;
    std::uint32_t GetId() const { return Id; }
    int GetCasterGUID() const { return Guid; }
    bool IsAffectedOnSpell(SpellInfo const*) const { return Affected; }
};

@METADATA@

bool Healing(Caster* caster, SpellInfo const* spellProto, AuraEffect const* effect)
{
    return (@HEALING@)(effect);
}

int main()
{
    SpellInfo shield{642};
    ApplyArea52CrusaderAbilityContracts(&shield, false);
    assert(shield.Effects[0].MiscValue == 2);
    ApplyArea52CrusaderAbilityContracts(&shield, true);
    ApplyArea52CrusaderAbilityContracts(&shield, true);
    assert(shield.Effects[0].MiscValue == 127);
    assert(shield.Effects[1].MiscValue == 2);
    for (auto id : {954875u, 955004u, 955005u, 955006u, 955007u, 955008u,
        955013u, 955014u, 955015u, 955016u})
    {
        SpellInfo info{id};
        info.Effects[1].BasePoints = id == 955015 || id == 955016 ? 9 : 7;
        int const amount = info.Effects[1].BasePoints;
        ApplyArea52CrusaderAbilityContracts(&info, false);
        assert(info.Effects[1].ApplyAuraName == 271);
        assert(info.Effects[1].BasePoints == amount);
        ApplyArea52CrusaderAbilityContracts(&info, true);
        ApplyArea52CrusaderAbilityContracts(&info, true);
        assert(info.Effects[1].ApplyAuraName == 283);
        assert(info.Effects[1].BasePoints == 7);
        assert(info.Effects[0].ApplyAuraName == 271);
        Caster caster;
        AuraEffect aura{id};
        for (int school = 1; school < 128; ++school)
        {
            info.School = school;
            assert(Healing(&caster, &info, &aura) == bool(school & 2));
            aura.Guid = 2;
            assert(!Healing(&caster, &info, &aura));
            aura.Guid = 1;
            caster.Area52 = false;
            assert(!Healing(&caster, &info, &aura));
            caster.Area52 = true;
        }
    }
    for (auto id : {954874u, 955017u, 955018u, 955019u, 955020u})
    {
        SpellInfo info{id};
        ApplyArea52CrusaderAbilityContracts(&info, false);
        assert(info.Effects[0].TargetB.Target == 30);
        ApplyArea52CrusaderAbilityContracts(&info, true);
        assert(info.Effects[0].TargetB.Target == 56);
        assert(info.Speed == 20 && info.Effects[0].Radius == 30);
    }
    SpellInfo other{12345};
    ApplyArea52CrusaderAbilityContracts(&other, true);
    assert(other.Effects[0].TargetB.Target == 30 && other.Effects[1].ApplyAuraName == 271);
    Caster caster;
    AuraEffect ordinary{111};
    assert(!Healing(&caster, &other, &ordinary));
    ordinary.Affected = true;
    assert(Healing(&caster, &other, &ordinary));
    ordinary.Guid = 2;
    assert(!Healing(&caster, &other, &ordinary));
    AuraEffect existing{271315, 1, true};
    assert(!Healing(&caster, &other, &existing));
    other.Id = 271303;
    assert(Healing(&caster, &other, &existing));
}
