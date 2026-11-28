#include "Area52CrusaderAbilityRules.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <initializer_list>
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
constexpr int SPELLFAMILY_WARLOCK = 5;
struct SpellInfo;
struct Unit
{
    uint32 Type = 4;
    bool Player = false;
    bool Totem = false;
    bool Guardian = false;
    bool Controllable = false;
    bool Assist = true;
    bool SameRaid = true;
    float Distance = 0;
    uint32 GetCreatureTypeMask() const { return Type; }
    bool IsPlayer() const { return Player; }
    bool IsTotem() const { return Totem; }
    bool IsGuardian() const { return Guardian; }
    bool IsControllableGuardian() const { return Controllable; }
    bool _IsValidAssistTarget(Unit* target,SpellInfo const*) const { return target->Assist; }
    bool IsInRaidWith(Unit* target) const { return target->SameRaid; }
    float GetPositionX() const { return Distance; }
    float GetPositionY() const { return 0; }
    float GetPositionZ() const { return 0; }
    float GetDistance(float x,float,float) const { return x; }
};
struct SpellInfo
{
    uint32 Id = 710;
    int SpellFamilyName = 5;
    int Category = 0;
    uint32 TargetCreatureType = 12;
    float Speed = 20;
    int GetCategory() const { return Category; }
    bool CheckTargetCreatureType(Unit const*) const;
};
@CREATURE@
namespace AscensionFreePick { bool EnabledValue=true; bool Enabled() { return EnabledValue; } }
struct Modifier { uint32 spellId=954879; int mask=0; };
bool Rank(uint32 Id,Modifier const* mod)
{
    @RANK@
    return false;
}
uint64 Delay(Unit* m_caster,Unit* target,SpellInfo const* m_spellInfo)
{
    struct { uint64 timeDelay=0; } targetInfo;
    uint64 m_delayMoment=0;
    @DELAY@
    return targetInfo.timeDelay;
}
bool Group(Unit* _caster,Unit* _referer,Unit* unitTarget,SpellInfo const* _spellInfo)
{
    @GROUP@
    return true;
}
int main()
{
    SpellInfo info;
    Unit caster,target;
    for (uint32 id : {710u,18647u})
    {
        info.Id=id;
        for (uint32 type : {1u,2u,4u,8u,16u,32u,64u,128u})
        { target.Type=type; assert(info.CheckTargetCreatureType(&target)==bool(type&12)); }
        target.Type=0; assert(info.CheckTargetCreatureType(&target));
    }
    Modifier mod;
    for (uint32 id : {954874u,954875u,955013u,955014u,955015u,955016u,955017u,955018u,955019u,955020u})
    {
        assert(Rank(id,&mod));
        mod.mask=1; assert(!Rank(id,&mod)); mod.mask=0;
        AscensionFreePick::EnabledValue=false; assert(!Rank(id,&mod)); AscensionFreePick::EnabledValue=true;
    }
    for (uint32 id : {955004u,955005u,955006u,955007u,955008u,954878u,954879u,1u})
        assert(!Rank(id,&mod));
    mod.spellId=1; assert(!Rank(954875,&mod));
    for (float distance : {0.0f,4.0f,5.0f,6.0f,12.0f,18.0f,24.0f,30.0f})
    {
        target.Distance=distance;
        uint64 expected=distance<5.0f ? 250 : uint64(std::floor(distance*50));
        assert(Delay(&caster,&target,&info)==expected);
    }
    assert(Delay(&caster,&caster,&info)==0);
    assert(Group(&caster,&caster,&target,&info));
    target.SameRaid=false; assert(!Group(&caster,&caster,&target,&info)); target.SameRaid=true;
    target.Assist=false; assert(!Group(&caster,&caster,&target,&info)); target.Assist=true;
    target.Totem=true; assert(!Group(&caster,&caster,&target,&info)); target.Totem=false;
    target.Guardian=true; assert(!Group(&caster,&caster,&target,&info));
    target.Controllable=true; assert(Group(&caster,&caster,&target,&info));
}
