#include <array>
#include <cassert>
#include <cstdint>
#include <set>
#include <vector>
using uint32 = std::uint32_t;
using int32 = std::int32_t;
using SpellEffIndex = int;
using SpellCastResult = int;
using SpellMissInfo = int;
constexpr int SPELL_CAST_OK = 0;
constexpr int SPELL_FAILED_TARGET_AURASTATE = 1;
constexpr int SPELL_MISS_IMMUNE = 7;
constexpr int SPELLFAMILY_PALADIN = 10;
constexpr int SPELLVALUE_MAX_TARGETS = 0;
constexpr int EFFECT_0 = 0;
constexpr uint32 SPELL_PALADIN_FORBEARANCE = 25771;
constexpr uint32 SPELL_PALADIN_AVENGING_WRATH_MARKER = 61987;
constexpr uint32 SPELL_PALADIN_IMMUNE_SHIELD_MARKER = 61988;
struct Effect { uint32 TriggerSpell = 20424; };
struct SpellInfo
{
    uint32 Id = 710;
    int SpellIconID = 0;
    int SpellFamilyName = 10;
    std::array<uint32,3> SpellFamilyFlags{};
    Effect Effects[3];
};
struct Aura { bool Removed = false; void Remove() { Removed = true; } };
struct Unit
{
    int Guid = 1;
    bool Area52 = true;
    bool ConsecratedOwned = false;
    bool Alive = true;
    uint32 BanishId = 710;
    int BanishOwner = 1;
    Aura Banished;
    std::set<uint32> Auras;
    int Cleave = 0;
    Unit* LastTarget = nullptr;
    std::vector<uint32> Casts;
    Unit* ToPlayer() { return this; }
    int GetGUID() const { return Guid; }
    bool HasAura(uint32 id) const { return Auras.count(id); }
    bool IsAlive() const { return Alive; }
    void AddAura(uint32 id, Unit* target) { target->Auras.insert(id); }
    Aura* GetAura(uint32 id,int owner) { return id == BanishId && owner == BanishOwner ? &Banished : nullptr; }
    Aura* GetAuraOfRankedSpell(uint32 id,int owner) { return id == 710 && (BanishId == 710 || BanishId == 18647) && owner == BanishOwner ? &Banished : nullptr; }
    template<class T> void CastCustomSpell(uint32 id,int,int targets,Unit* target,bool,void*,T const*)
    { Cleave=targets; LastTarget=target; Casts.push_back(id); }
};
using Player = Unit;
namespace AscensionMysticEnchant { bool GrantsActive(Player* p,uint32 id) { return p && id==965295 && p->ConsecratedOwned; } }
namespace AscensionFreePick { bool Applies(Unit* u) { return u && u->Area52; } }
struct AuraEffect { SpellInfo Info; SpellInfo const* GetSpellInfo() const { return &Info; } };
struct ProcEventInfo
{
    Unit* Actor;
    Unit* Target;
    SpellInfo* Spell;
    Unit* GetActor() const { return Actor; }
    Unit* GetActionTarget() const { return Target; }
    SpellInfo const* GetSpellInfo() const { return Spell; }
};
struct Base
{
    Unit* Caster = nullptr;
    Unit* Target = nullptr;
    SpellInfo Info;
    std::set<int> Prevented;
    bool DefaultPrevented = false;
    bool AuraPrevented = false;
    Unit* GetCaster() const { return Caster; }
    Unit* GetExplTargetUnit() const { return Target; }
    Unit* GetHitUnit() const { return Target; }
    SpellInfo const* GetSpellInfo() const { return &Info; }
    void PreventHitDefaultEffect(int effect) { Prevented.insert(effect); }
    void PreventDefaultAction() { DefaultPrevented = true; }
    void PreventHitAura() { AuraPrevented = true; }
};
struct Immunities : Base { @CHECK@ @DEBUFFS@ };
struct Banish : Base { @BANISH@ };
struct Seal : Base { @SEAL_CHECK@ @SEAL_PROC@ };
struct Blinding : Base { @BLINDING@ };
int main()
{
    Unit caster,target;
    Immunities immunity;
    immunity.Caster=&caster; immunity.Target=&target;
    assert(immunity.CheckCast()==SPELL_CAST_OK);
    for (auto id : {25771u,61987u})
    {
        target.Auras={id}; assert(immunity.CheckCast()==SPELL_FAILED_TARGET_AURASTATE);
        target.Auras.clear();
    }
    target.Auras={61988}; assert(immunity.CheckCast()==SPELL_CAST_OK);
    target.Auras.clear(); immunity.TriggerDebuffs();
    assert(target.Auras==std::set<uint32>({25771,61987,61988}) && caster.Auras.empty());
    immunity.Target=nullptr; caster.Auras={25771};
    assert(immunity.CheckCast()==SPELL_FAILED_TARGET_AURASTATE);
    immunity.TriggerDebuffs(); caster.Auras.clear();
    for (auto id : {710u,18647u})
    {
        Banish spell; spell.Caster=&caster; spell.Target=&target; spell.Info.Id=id;
        target.BanishId=id; target.BanishOwner=1; target.Banished.Removed=false;
        spell.HandleBanish(0); assert(!target.Banished.Removed);
        spell.HandleBanish(SPELL_MISS_IMMUNE); assert(target.Banished.Removed);
        target.Banished.Removed=false; target.BanishOwner=2;
        spell.HandleBanish(SPELL_MISS_IMMUNE); assert(!target.Banished.Removed);
        target.BanishOwner=1; target.BanishId=999;
        spell.HandleBanish(SPELL_MISS_IMMUNE); assert(!target.Banished.Removed);
        spell.Target=nullptr; spell.HandleBanish(SPELL_MISS_IMMUNE);
    }
    for (bool area52 : {false,true})
        for (auto castId : {710u,18647u})
            for (auto auraId : {710u,18647u})
                for (int owner : {1,2})
                {
                    caster.Area52=area52;
                    Banish spell; spell.Caster=&caster; spell.Target=&target; spell.Info.Id=castId;
                    target.BanishId=auraId; target.BanishOwner=owner; target.Banished.Removed=false;
                    spell.HandleBanish(SPELL_MISS_IMMUNE);
                    assert(target.Banished.Removed==(owner==1 && (area52 || castId==auraId)));
                    target.Banished.Removed=false; spell.Caster=nullptr;
                    spell.HandleBanish(SPELL_MISS_IMMUNE); assert(!target.Banished.Removed);
                }
    caster.Area52=true;
    Seal seal; SpellInfo attack; AuraEffect aura; ProcEventInfo event{&caster,&target,&attack};
    assert(seal.CheckProc(event)); attack.SpellIconID=3025; assert(!seal.CheckProc(event));
    attack.SpellIconID=0; event.Actor=nullptr; assert(!seal.CheckProc(event));
    event.Actor=&caster; event.Target=nullptr; assert(!seal.CheckProc(event)); event.Target=&target;
    for (uint32 mask : {0u,0x20000u,0x40000u,0x60000u})
    {
        attack.SpellFamilyFlags[1]=mask; caster.Casts.clear();
        seal.HandleProc(&aura,event);
        assert(seal.DefaultPrevented && caster.Casts==std::vector<uint32>{20424});
        assert(caster.Cleave==(mask ? 1 : 3) && caster.LastTarget==&target);
    }
    for (bool realm : {false,true})
        for (bool equipped : {false,true})
            for (bool restored : {false,true})
            {
                caster.Area52=realm; caster.ConsecratedOwned=equipped; caster.Auras.clear();
                if (restored) caster.Auras.insert(965295);
                caster.Casts.clear(); attack.SpellFamilyFlags[1]=0; seal.HandleProc(&aura,event);
                assert(caster.Casts==std::vector<uint32>{realm && equipped && restored ? 278746u : 20424u});
                assert(caster.Cleave==3);
            }
    caster.Area52=true; caster.ConsecratedOwned=false; caster.Auras.clear();
    event.Spell=nullptr; seal.HandleProc(&aura,event); assert(caster.Cleave==3);
    target.Alive=false; caster.Casts.clear(); seal.HandleProc(&aura,event); assert(caster.Casts.empty());
    for (bool area52 : {false,true})
        for (bool enchant : {false,true})
            for (bool spirit : {false,true})
            {
                caster.Area52=area52; caster.Auras.clear();
                if (enchant) caster.Auras.insert(983590);
                if (spirit) caster.Auras.insert(84867);
                Blinding blind; blind.Caster=&caster;
                blind.HandleControl(0); blind.HandleControl(1); blind.HandleHeal(2);
                assert(blind.AuraPrevented==(area52 && enchant && spirit));
                assert(bool(blind.Prevented.count(0))==(area52 && enchant && spirit));
                assert(bool(blind.Prevented.count(1))==(area52 && enchant && spirit));
                assert(bool(blind.Prevented.count(2))==(area52 && !(enchant && spirit)));
            }
    Blinding absent; absent.HandleControl(0); absent.HandleHeal(2); assert(absent.Prevented.empty());
}
