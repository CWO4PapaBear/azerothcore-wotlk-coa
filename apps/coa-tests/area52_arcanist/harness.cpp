#include "Area52ArcanistRules.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
using uint32 = std::uint32_t;
using int32 = std::int32_t;
using uint8 = std::uint8_t;
using AuraEffectHandleModes = int;
constexpr int EFFECT_0=0, EFFECT_1=1, EFFECT_2=2, SPELL_AURA_DUMMY=4;
constexpr int SPELL_AURA_MOD_DAMAGE_PERCENT_DONE=79, TARGET_UNIT_CASTER=1, SPELL_SCHOOL_MASK_MAGIC=126;
constexpr unsigned SPELL_ATTR0_CU_FORCE_AURA_SAVING=1;
int SpellImplicitTargetInfo(int v) { return v; }
struct Effect {
    int ApplyAuraName=0, Effect=6, TriggerSpell=0, TargetA=1, MiscValue=0;
    bool IsAura(int a) const { return Effect==6 && ApplyAuraName==a; }
};
struct SpellInfo {
    unsigned Id=0, SchoolMask=4, SpellLevel=1;
    bool positive=false;
    unsigned AttributesCu=0;
    Effect Effects[3];
    bool IsPositive() const { return positive; }
};
struct AuraEffect {
    int amount=0, mask=0;
    unsigned id=0;
    int GetAmount() const { return amount; }
    int GetMiscValue() const { return mask; }
    unsigned GetId() const { return id; }
};
struct Aura {
    unsigned stacks=1;
    void SetStackAmount(uint8 n) { stacks=n; }
};
struct Session { bool logout=false; bool PlayerLogout() const { return logout; } };
struct State { bool Loaded=false, DraftResolved=false; std::set<uint32> Applied,Known; };
struct Custom {
    State state;
    template<typename T> T* Get(char const*) { return &state; }
};
struct Player {
    bool area52=true, alive=true, world=true, meteor=true, hostile=false, pvp=false;
    unsigned guid=1, level=80;
    Session session;
    Custom CustomData;
    std::array<uint32,17> slots{};
    std::map<std::pair<uint32,uint32>,Aura> auras;
    std::map<uint32,AuraEffect> bonuses;
    std::vector<uint32> casts;
    unsigned activeDraft=0,forfeited=0;
    Player* ToPlayer() { return this; }
    Player const* ToPlayer() const { return this; }
    unsigned GetGUID() const { return guid; }
    bool HasAura(unsigned id,unsigned owner) const { return auras.contains({id,owner}); }
    bool HasActiveSpell(unsigned id) const { return id==760040 && meteor; }
    bool HasSpell(unsigned id) const { return HasActiveSpell(id); }
    void RemoveAurasDueToSpell(unsigned id,unsigned owner) { auras.erase({id,owner}); }
    Aura* AddAura(unsigned id,Player*) { return &auras[{id,guid}]; }
    Aura* GetAura(unsigned id,unsigned owner) { auto i=auras.find({id,owner}); return i==auras.end()?nullptr:&i->second; }
    void CastSpell(Player* target,unsigned id,bool triggered) {
        assert(triggered);casts.push_back(id);
        if(target==this) AddAura(id,this);
    }
    bool IsValidAttackTarget(Player* target) const { return target && target!=this && target->hostile && target->alive; }
    Session* GetSession() { return &session; }
    bool IsInWorld() const { return world; }
    bool IsAlive() const { return alive; }
    unsigned GetLevel() const { return level; }
    struct Setting { unsigned value; };
    Setting GetPlayerSetting(char const* key,unsigned) { return {std::string(key).find("active_build")!=std::string::npos?activeDraft:forfeited}; }
    void UpdatePlayerSetting(unsigned,unsigned i,unsigned v) {slots[i]=v;}
    void SaveToDB(bool,bool) {}
    void SetTemporarySpellReplacement(unsigned,unsigned) {}
    bool IsCharmedOwnedByPlayerOrPlayer() const { return pvp; }
    AuraEffect const* GetAuraEffect(unsigned id,int) const { auto i=bonuses.find(id);return i==bonuses.end()?nullptr:&i->second; }
    template<typename Predicate> float GetTotalAuraMultiplier(int,Predicate) const { return 1.0f; }
    float GetAscensionPvpTuningDamageMultiplier(Player const*,uint32) const;
};
using Unit=Player;
using WorldObject=Player;
namespace AscensionFreePick { bool Applies(Player const* p) { return p && p->area52; } }
namespace Area52MysticRealm { bool configured=true; bool UsesFreePickEngine() {return configured;} }
struct Spell {
    struct Targets { Unit* target=nullptr; Unit* GetUnitTarget() {return target;} } m_targets;
    bool triggered=false;
    std::map<uint32,uint32> values;
    bool IsTriggered() const {return triggered;}
    uint32 GetScriptValue(uint32 k) {return values[k];}
    void SetScriptValue(uint32 k,uint32 v) {values[k]=v;}
};
struct AuraScript { std::string name; std::string const* _GetScriptName() const {return &name;} };
using SpellScript=AuraScript;
struct Manager {
    std::map<uint32,SpellInfo> records;
    SpellInfo const* GetSpellInfo(uint32 id) {auto i=records.find(id);return i==records.end()?nullptr:&i->second;}
    uint32 GetNextSpellInChain(uint32) {return 0;}
} manager;
auto* sSpellMgr=&manager;
struct Registry {
    std::map<uint32,std::string> bindings;
    void CreateAuraScripts(uint32 id,std::list<AuraScript*>& list) {if(bindings.contains(id))list.push_back(new AuraScript{bindings[id]});}
    void CreateSpellScripts(uint32 id,std::list<SpellScript*>& list) {CreateAuraScripts(id,list);}
} registry;
auto* sScriptMgr=&registry;
namespace Acore::Containers { template<typename T> void RandomResize(std::list<T>& l,unsigned n) {while(l.size()>n)l.pop_back();} }
namespace Area52Arcanist { @ELIGIBLE@ @SYNC@ @CONNECTED@ }
struct CastHook { @CAST@ };
struct Metadata { @METADATA@ };
struct AuraHook {
    Player* owner=nullptr;unsigned id=0,stacks=1;
    Player* GetTarget() {return owner;}
    unsigned GetId() const {return id;}
    unsigned GetStackAmount() const {return stacks;}
    @POTENTIAL@
    @REMOVE@
    @UNEQUIP@
};
struct TargetHook {
    Unit* target=nullptr;uint32 _targets=1;int32 damage=0;
    Unit* GetExplTargetUnit() {return target;}
    int32 GetHitDamage() const {return damage;}
    void SetHitDamage(int32 n) {damage=n;}
    @TARGETS@ @HIT@
};
constexpr int SPELL_AURA_ASCENSION_MOD_PVP_DAMAGE_DONE_PCT=322;
namespace Area52HopliteRemaining {constexpr unsigned HungerBuff=986114;}
bool IsAscensionPvpTuningAura(unsigned) {return false;}
@PVP@
namespace Restoration {
char const* StateKey="fixture";
struct Entry {};
struct Catalog {std::map<uint32,Entry> BySpell;} Data;
struct Grant {unsigned Enchant,Spell,Requires,Replaces;};
std::vector<Grant> SpellGrants,RetiredSpellGrants;
bool Applies(Player* p) {return AscensionFreePick::Applies(p);}
auto ReadSlots(Player* p) {return p->slots;}
bool ArrangeSlots(Catalog&,std::array<uint32,17>&) {return true;}
bool ValidateOwnedSlots(State const& state,std::array<uint32,17> const& slots,unsigned) {
    return std::all_of(slots.begin(),slots.end(),[&](unsigned id){return !id || state.Known.contains(id);});
}
unsigned Setting(Player*) {return 0;}
void SendSlots(Player*) {}
bool InvestmentMet(Player*,Entry,bool) {return true;}
bool GrantsActive(Player*,unsigned) {return false;}
template<typename P> bool ExactGrantPrerequisiteMet(Grant,P) {return false;}
bool GrantRequirementMet(Player*,unsigned) {return false;}
bool GrantRankDue(bool,bool,unsigned,unsigned) {return false;}
void SetGrantedSpell(Player*,unsigned,unsigned,bool) {}
void RefreshEpicReplacements(Player*) {}
@RESTORE@
}
namespace Persistence {
constexpr unsigned SPELL_ATTR0_CU_FORCE_AURA_SAVING=1, SPELL_ATTR0_CU_AURA_CANNOT_BE_SAVED=2, SPELL_ATTR0_DO_NOT_DISPLAY=4;
struct SpellInfo {
    unsigned flags=0,Stances=0;
    bool HasAttribute(unsigned flag) const {return flags&flag;}
    bool IsChanneled() const {return false;}
    bool IsSingleTarget() const {return false;}
};
struct Owner {unsigned GetGUID() const{return 1;}} owner;
struct Aura {
    SpellInfo info;bool passive=false;
    SpellInfo const* GetSpellInfo() const {return &info;}
    bool IsPassive() const {return passive;}
    unsigned GetCasterGUID() const {return 1;}
    Owner const* GetOwner() const {return &owner;}
    bool IsSingleTarget() const {return false;}
    bool IsUsingCharges() const {return false;}
    unsigned GetCharges() const {return 0;}
    bool CanBeSaved() const;
};
@SAVE@
}
int main() {
    using namespace Area52Arcanist;
    assert(Element(0)==-1 && Element(1|2|32)==-1);
    assert(Element(64)==0 && Element(4)==1 && Element(16)==2 && Element(8)==3);
    assert(Element(64|4|16|8)==1 && Element(64|16|8)==3 && Element(64|16)==2);
    Player p,enemy;enemy.hostile=true;enemy.guid=2;p.AddAura(Enchant,&p);
    CastHook hook;SpellInfo info;Spell cast;cast.m_targets.target=&enemy;
    for(unsigned index=0;index<4;++index) {
        info.SchoolMask=Schools[index];Spell fresh;fresh.m_targets.target=&enemy;
        auto before=p.casts.size();hook.OnSpellBeforeEffects(&fresh,&p,&info);
        assert(p.HasAura(Cooldowns[index],p.guid) && p.casts.back()==Meteors[index]);
        auto after=p.casts.size();assert(after>before);
        hook.OnSpellBeforeEffects(&fresh,&p,&info);assert(p.casts.size()==after);
        Spell next;next.m_targets.target=&enemy;hook.OnSpellBeforeEffects(&next,&p,&info);assert(p.casts.size()==after);
    }
    assert(CurrentMote(&p)==0);
    for(unsigned index=0;index<4;++index) {
        AuraHook cycle{&p,Potential,3};cycle.Apply(nullptr,0);assert(CurrentMote(&p)==int(index));
        p.AddAura(Potential,&p);cycle.stacks=4;cycle.Apply(nullptr,0);
        assert(!p.HasAura(Potential,p.guid) && p.HasAura(Resonances[index],p.guid));
        assert(CurrentMote(&p)==int((index+1)%4));
        AuraHook expiry{&p,Resonances[index]};expiry.Remove(nullptr,0);
        assert(!p.HasAura(Cooldowns[index],p.guid));
    }
    for(unsigned scenario=0;scenario<8;++scenario) {
        Player owner=p;owner.casts.clear();Clear(&owner);owner.AddAura(Enchant,&owner);
        Spell attempt;attempt.m_targets.target=&enemy;SpellInfo spell;spell.SchoolMask=4;
        if(scenario==0)owner.area52=false;
        if(scenario==1)owner.meteor=false;
        if(scenario==2)owner.RemoveAurasDueToSpell(Enchant,owner.guid);
        if(scenario==3)attempt.triggered=true;
        if(scenario==4)spell.positive=true;
        if(scenario==5)attempt.m_targets.target=nullptr;
        if(scenario==6)attempt.m_targets.target=&owner;
        if(scenario==7)spell.SchoolMask=32;
        hook.OnSpellBeforeEffects(&attempt,&owner,&spell);assert(owner.casts.empty());
    }
    Metadata metadata;
    SpellInfo parent;parent.Id=Enchant;parent.Effects[0].ApplyAuraName=42;
    manager.records[Enchant]=parent;assert(!Connected());
    metadata.OnLoadSpellCustomAttr(&parent);assert(parent.Effects[0].ApplyAuraName==4 && !parent.Effects[1].Effect);
    manager.records[Enchant]=parent;assert(!Connected());
    registry.bindings[Enchant]="aura_area52_arcanist_equipment";registry.bindings[Potential]="aura_area52_arcanist_potential";
    for(auto id:Resonances)registry.bindings[id]="aura_area52_arcanist_resonance";
    for(auto id:Meteors)registry.bindings[id]="spell_area52_arcanist_meteor";
    assert(!Connected());
    for(auto const& group:{Motes,Cooldowns})for(auto id:group)manager.records[id]={};
    for(auto const& group:{Meteors,Resonances})for(auto id:group)manager.records[id]={};
    manager.records[Potential]={};
    manager.records[Meteor]={};assert(Connected());assert(Connected());
    registry.bindings.erase(Resonances[2]);assert(!Connected());registry.bindings[Resonances[2]]="aura_area52_arcanist_resonance";
    for(unsigned index=0;index<4;++index) {
        SpellInfo resonance;resonance.Id=Resonances[index];resonance.Effects[2].ApplyAuraName=322;
        metadata.OnLoadSpellCustomAttr(&resonance);metadata.OnLoadSpellCustomAttr(&resonance);
        assert(resonance.Effects[2].ApplyAuraName==79 && resonance.Effects[2].MiscValue==int(126&~Schools[index]));
        Area52MysticRealm::configured=false;SpellInfo other;other.Id=Resonances[index];other.Effects[2].ApplyAuraName=322;
        metadata.OnLoadSpellCustomAttr(&other);assert(other.Effects[2].ApplyAuraName==322);Area52MysticRealm::configured=true;
        Player attacker,victim;victim.pvp=true;float bonus=index==0?40:index==2?50:60;
        attacker.bonuses[Resonances[index]]={int(bonus),int(Schools[index]),Resonances[index]};
        assert(std::abs((1+bonus/100)*attacker.GetAscensionPvpTuningDamageMultiplier(&victim,Schools[index])-(1+bonus/200))<0.00001);
        victim.pvp=false;assert(attacker.GetAscensionPvpTuningDamageMultiplier(&victim,Schools[index])==1);
        victim.pvp=true;attacker.area52=false;assert(attacker.GetAscensionPvpTuningDamageMultiplier(&victim,Schools[index])==1);
    }
    for(unsigned order=0;order<2;++order) {
        Player login;login.slots[0]=Enchant;login.activeDraft=1;
        auto& state=login.CustomData.state;state.Known.insert(Enchant);Restoration::Data.BySpell[Enchant]={};
        login.AddAura(Motes[2],&login);login.AddAura(Potential,&login)->SetStackAmount(3);
        if(order==0)state.Loaded=true;else state.DraftResolved=true;
        Restoration::Synchronize(&login);assert(!login.HasAura(Enchant,login.guid));assert(CurrentMote(&login)==2);
        state.Loaded=true;state.DraftResolved=true;
        Restoration::Synchronize(&login);assert(login.HasAura(Enchant,login.guid));assert(CurrentMote(&login)==2);
        Restoration::Synchronize(&login);assert(login.GetAura(Potential,login.guid)->stacks==3);
        login.auras[{Motes[1],99}]={};
        login.slots[0]=0;Restoration::Synchronize(&login);
        assert(!login.HasAura(Enchant,login.guid) && CurrentMote(&login)==-1 && !login.HasAura(Potential,login.guid));
        assert(login.HasAura(Motes[1],99));
        login.slots[0]=Enchant;Restoration::Synchronize(&login);assert(login.HasAura(Enchant,login.guid));
        login.AddAura(Motes[0],&login);login.meteor=false;Restoration::Synchronize(&login);assert(CurrentMote(&login)==-1);
    }
    std::list<WorldObject*> targets;std::array<Player,12> enemies;
    for(auto& e:enemies)targets.push_back(&e);
    TargetHook selector{&enemies.back()};selector.Targets(targets);assert(targets.size()==5 && targets.front()==&enemies.back());
    selector.damage=1000;selector.Hit();assert(selector.damage==200);
    targets={&enemies.back()};selector.Targets(targets);selector.damage=1000;selector.Hit();assert(selector.damage==1000);
    p.session.logout=true;AuraHook cleanup{&p,Enchant};cleanup.Unequip(nullptr,0);assert(CurrentMote(&p)==0);
    p.session.logout=false;cleanup.Unequip(nullptr,0);assert(CurrentMote(&p)==-1);
    Persistence::Aura saved;assert(saved.CanBeSaved());saved.passive=true;assert(!saved.CanBeSaved());
}
