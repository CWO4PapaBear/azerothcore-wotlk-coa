#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <set>
#include <utility>

using uint8 = std::uint8_t;
using uint32 = std::uint32_t;
using int32 = std::int32_t;
using SpellGroup = uint32;
struct Effect { bool Area = false; bool IsAreaAuraEffect() const { return Area; } };
struct SpellInfo
{
    uint32 Id;
    Effect Effects[3];
    SpellInfo const* GetFirstRankSpell() const { return this; }
};
class SpellMgr
{
public:
    std::multimap<uint32, SpellGroup> Membership;
    std::map<SpellGroup, std::set<uint32>> mSpellSameEffectStack;
    auto GetSpellSpellGroupMapBounds(uint32 id) const { return Membership.equal_range(id); }
    bool IsSpellMemberOfSpellGroup(uint32 id, SpellGroup group) const
    {
        auto const range = Membership.equal_range(id);
        return std::any_of(range.first,range.second,[group](auto const& pair) { return pair.second == group; });
    }
    bool IsEffectInSameEffectStackGroup(SpellInfo const*, uint8, SpellGroup) const;
    bool AddSameEffectStackRuleSpellGroups(SpellInfo const*, uint8, uint32, int32,
        std::map<SpellGroup,int32>&) const;
};
@POLICY@
@ADMISSION@
int main()
{
    SpellMgr manager;
    constexpr std::array<uint32,7> members{12042,12292,31884,34471,48391,49016,57933};
    for (auto id : members) manager.Membership.emplace(id,1107);
    manager.mSpellSameEffectStack.emplace(1107,std::set<uint32>{79,136});
    for (auto id : members)
        for (int other : {5,15,20,50})
            for (bool reverse : {false,true})
            {
                std::map<SpellGroup,int32> groups;
                SpellInfo wrath{}, peer{};wrath.Id=31884;peer.Id=id;
                auto apply = [&](SpellInfo const& info,int amount)
                { assert(manager.AddSameEffectStackRuleSpellGroups(&info,0,79,amount,groups)); };
                if (reverse) { apply(peer,other); apply(wrath,15); }
                else { apply(wrath,15); apply(peer,other); }
                assert(groups.size()==1 && groups.at(1107)==std::max(15,other));
                groups.clear();
                assert(manager.AddSameEffectStackRuleSpellGroups(&wrath,1,136,15,groups));
                assert(manager.AddSameEffectStackRuleSpellGroups(&peer,1,136,other,groups));
                assert(groups.at(1107)==std::max(15,other));
                assert(!manager.AddSameEffectStackRuleSpellGroups(&wrath,2,108,100,groups));
                assert(groups.at(1107)==std::max(15,other));
            }
    SpellInfo unrelated{};unrelated.Id=12345;
    std::map<SpellGroup,int32> groups;
    assert(!manager.AddSameEffectStackRuleSpellGroups(&unrelated,0,79,100,groups));
    assert(groups.empty());
    SpellInfo wrath{};wrath.Id=31884;
    assert(manager.AddSameEffectStackRuleSpellGroups(&wrath,0,79,-15,groups));
    assert(manager.AddSameEffectStackRuleSpellGroups(&wrath,0,79,-30,groups));
    assert(groups.at(1107)==-30);
    manager.Membership.emplace(31884,2000180);
    manager.mSpellSameEffectStack.emplace(2000180,std::set<uint32>{79});
    groups.clear();
    assert(manager.AddSameEffectStackRuleSpellGroups(&wrath,0,79,15,groups));
    assert(groups.count(1107)==1 && groups.count(2000180)==0);
    wrath.Effects[0].Area=true;
    groups.clear();
    assert(manager.AddSameEffectStackRuleSpellGroups(&wrath,0,79,15,groups));
    assert(groups.count(2000180)==1 && groups.count(1107)==0);
}
