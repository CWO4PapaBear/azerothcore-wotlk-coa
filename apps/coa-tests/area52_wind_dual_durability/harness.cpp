#include <cassert>
#include <cstdint>
#include <map>
using AuraRemoveMode=int;using uint32=uint32_t;using int32=int32_t;
constexpr int BASE_ATTACK=0,OFF_ATTACK=1,EFFECT_0=0,SPELL_AURA_MOD_CRIT_PCT=290;
struct Player;struct AuraEffect;
constexpr int ITEM_FIELD_MAXDURABILITY=0,ITEM_FIELD_DURABILITY=1,ITEM_CHANGED=2;
struct Weapon{bool broken=false,equipped=true;int durability=10;int GetUInt32Value(int i){return i==0?10:durability;}void SetUInt32Value(int,int n){durability=n;broken=n==0;}bool IsEquipped(){return equipped;}int GetSlot(){return 16;}void SetState(int,Player*){}};using Item=Weapon;
struct Player{bool HasPreventDurabilityLossAura(){return false;}void _ApplyItemMods(Item*,int,bool);void DurabilityPointsLoss(Item*,int32);uint32 removed=0,removedOwner=0;void RemoveAurasDueToSpell(uint32 id,uint32 owner){removed=id;removedOwner=owner;}bool area=true,feral=false;Weapon*main=nullptr,*off=nullptr;std::map<uint32,AuraEffect*>effects;Player*ToPlayer(){return this;}bool IsInFeralForm(){return feral;}Weapon*GetWeaponForAttack(int type,bool){auto*w=type?off:main;return w&&!w->broken?w:nullptr;}uint32 GetGUID(){return 7;}AuraEffect*GetAuraEffect(uint32 id,int,uint32){auto i=effects.find(id);return i==effects.end()?nullptr:i->second;}};
using Unit=Player;
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct Aura{uint32 id=0;uint32 GetId(){return id;}Player*owner=nullptr;Player*GetUnitOwner(){return owner;}};
struct AuraApplication{Aura aura;Aura*GetBase(){return &aura;}};
struct AuraEffect{uint32 id=913715;int index=0,base=2,amount=0,recalculations=0;Aura aura;uint32 GetId()const{return id;}int GetEffIndex()const{return index;}Aura*GetBase()const{return const_cast<Aura*>(&aura);}void RecalculateAmount(Player*);};
struct SpellInfo{uint32 Id=913715,StackAmount=3;struct Effect{int ApplyAuraName=4,BasePoints=-1;}Effects[3];};
bool Area52WindMetadataEnabled=false;
struct DamageInfo{int damage=10,attack=1;int GetDamage(){return damage;}int GetAttackType(){return attack;}};
struct ProcEventInfo{Player*actor=nullptr;DamageInfo*damage=nullptr;SpellInfo*spell=nullptr;DamageInfo*GetDamageInfo(){return damage;}Player*GetActor(){return actor;}SpellInfo*GetSpellInfo(){return spell;}};
@FUNCTIONS@
void AuraEffect::RecalculateAmount(Player*p){amount=base;OnAfterAuraEffectCalculateAmount(this,p,amount);++recalculations;}
struct Proc{Player*target=nullptr;Player*GetTarget(){return target;}@CHECK@};
struct Cleanup{@REMOVE@};
@DURABILITY@
void Player::_ApplyItemMods(Item*,int,bool){RefreshArea52DualWield(this);}
int main(){Player check;Weapon cmh,coh;check.main=&cmh;check.off=&coh;AuraEffect bonus;bonus.aura.owner=&check;check.effects[913715]=&bonus;RefreshArea52DualWield(&check);assert(bonus.amount==2);check.DurabilityPointsLoss(&coh,10);assert(coh.broken&&bonus.amount==0);check.DurabilityPointsLoss(&coh,-10);assert(!coh.broken&&bonus.amount==2);check.DurabilityPointsLoss(&cmh,100);assert(cmh.broken&&bonus.amount==0);
Player p;Weapon mh,oh;AuraEffect effect;effect.aura.owner=&p;p.effects[effect.id]=&effect;
SpellInfo metadata;ConfigureArea52DualWield(&metadata);assert(metadata.Effects[0].ApplyAuraName==4);Area52WindMetadataEnabled=true;ConfigureArea52DualWield(&metadata);assert(metadata.Effects[0].ApplyAuraName==290&&metadata.Effects[0].BasePoints==1&&metadata.StackAmount==1);
for(auto id:{913715u,30816u,30818u,30819u}){p.effects.clear();effect.id=id;effect.base=id==913715?6:id==30816?1:id==30818?2:3;p.effects[id]=&effect;int expected=id==913715?2:effect.base;p.main=nullptr;p.off=nullptr;RefreshArea52DualWield(&p);assert(effect.amount==0);p.main=&mh;RefreshArea52DualWield(&p);assert(effect.amount==0);p.off=&oh;RefreshArea52DualWield(&p);assert(effect.amount==expected);oh.broken=true;RefreshArea52DualWield(&p);assert(effect.amount==0);oh.broken=false;p.feral=true;RefreshArea52DualWield(&p);assert(effect.amount==0);p.feral=false;RefreshArea52DualWield(&p);assert(effect.amount==expected);p.effects.clear();int count=effect.recalculations;RefreshArea52DualWield(&p);assert(effect.recalculations==count);p.area=false;int amount=19;OnAfterAuraEffectCalculateAmount(&effect,&p,amount);assert(amount==19);p.area=true;}
effect.aura.owner=nullptr;int amount=7;OnAfterAuraEffectCalculateAmount(&effect,nullptr,amount);assert(amount==7);
Cleanup cleanup;for(auto id:{30816u,30818u,30819u}){AuraApplication app;app.aura.id=id;cleanup.OnAuraRemove(&p,&app,0);assert(p.removed==(id==30816?965970u:id==30818?966150u:966151u)&&p.removedOwner==7);}p.removed=0;AuraApplication unrelated;unrelated.aura.id=913715;cleanup.OnAuraRemove(&p,&unrelated,0);assert(!p.removed);unrelated.aura.id=30819;p.area=false;cleanup.OnAuraRemove(&p,&unrelated,0);assert(!p.removed);p.area=true;cleanup.OnAuraRemove(nullptr,nullptr,0);
for(auto [parent,helper]:{std::pair<unsigned,unsigned>{16256,16257},{16281,16277},{16282,16278}}){AuraApplication app;app.aura.id=parent;cleanup.OnAuraRemove(&p,&app,0);assert(p.removed==helper&&p.removedOwner==7);}
Proc proc;proc.target=&p;DamageInfo damage;ProcEventInfo event{&p,&damage,nullptr};assert(proc.CheckProc(event));damage.attack=0;assert(!proc.CheckProc(event));damage.attack=1;damage.damage=0;assert(!proc.CheckProc(event));damage.damage=1;event.spell=&metadata;assert(!proc.CheckProc(event));event.spell=nullptr;p.off=nullptr;assert(!proc.CheckProc(event));
}
