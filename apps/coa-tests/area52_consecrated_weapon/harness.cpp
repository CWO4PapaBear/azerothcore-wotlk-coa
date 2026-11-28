#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <map>
#include <vector>
#include <initializer_list>
using uint32=uint32_t;using uint64=uint64_t;using uint8=uint8_t;using WeaponAttackType=int;using TriggerCastFlags=int;
constexpr int BASE_ATTACK=0,OFF_ATTACK=1,RANGED_ATTACK=2,MAX_ATTACK=3,SPELL_DAMAGE_CLASS_MELEE=2,SPELL_DAMAGE_CLASS_RANGED=3,TEMP_ENCHANTMENT_SLOT=1,SPELL_AURA_PROC_TRIGGER_SPELL=42,ALLSPELLHOOK_ON_HIT_RESULT=1,SPELL_MISS_NONE=0,SPELLMOD_PROC_PER_MINUTE=1,SPELLMOD_CHANCE_OF_SUCCESS=2,TRIGGERED_FULL_MASK=255,TRIGGERED_DISALLOW_PROC_EVENTS=32;
constexpr int SPELL_ATTR3_REQUIRES_OFF_HAND_WEAPON=1;
struct SpellInfo{uint32 Id=1;int DmgClass=2;bool offhand=false;bool HasAttribute(int attribute)const{return attribute==SPELL_ATTR3_REQUIRES_OFF_HAND_WEAPON&&offhand;}};struct Item{uint32 id=10,enchant=45;uint32 GetGUID(){return id;}uint32 GetEnchantmentId(int){return enchant;}};
struct Player;struct Unit{bool alive=true;bool IsAlive(){return alive;}Player*player=nullptr;Player*ToPlayer(){return player;}Player*GetSpellModOwner()const;float GetPPMProcChance(uint32,float,SpellInfo const*)const;};
struct Aura{uint32 item=10,owner=7;SpellInfo info{977848,0};std::map<uint32,uint64>state;uint32 GetCastItemGUID(){return item;}uint32 GetCasterGUID(){return owner;}uint64 GetScriptValue(uint32 id){return state[id];}void SetScriptValue(uint32 id,uint64 v){state[id]=v;}SpellInfo const*GetSpellInfo(){return &info;}};
struct AuraEffect{Aura aura;uint32 id=977848;uint32 GetId()const{return id;}Aura*GetBase()const{return const_cast<Aura*>(&aura);}};
struct Player:Unit{bool area=true,weaponAvailable=true;uint32 speed=2000;Item weapon;std::vector<AuraEffect const*>effects;std::map<uint32,std::vector<uint32>>origins;int casts=0,flags=0,lastAttack=-1;float ppmMod=1,chanceMod=1;Player(){player=this;}bool IsValidAttackTarget(Unit*t){return t&&t!=this;}std::vector<uint32>GetSpellInteractionOrigins(uint32 id)const{auto i=origins.find(id);return i==origins.end()?std::vector<uint32>{}:i->second;}Item*GetWeaponForAttack(int attack,bool){lastAttack=attack;return weaponAvailable?&weapon:nullptr;}auto const&GetAuraEffectsByType(int){return effects;}uint32 GetGUID(){return 7;}uint32 GetAttackTime(int){return speed;}void ApplySpellMod(uint32,int op,float&v){v*=op==SPELLMOD_PROC_PER_MINUTE?ppmMod:chanceMod;}void CastSpell(Unit*,uint32 id,int f,Item*item,AuraEffect const*){assert(id==977827&&item==&weapon);++casts;flags=f;}};
Player*Unit::GetSpellModOwner()const{return player;}
@PPM@
struct Spell{Player*caster;SpellInfo info;bool triggered=false,automatic=false;int attack=MAX_ATTACK;Unit*GetCaster(){return caster;}SpellInfo const*GetSpellInfo(){return &info;}bool IsTriggered(){return triggered;}bool IsAutoRepeat(){return automatic;}int GetScriptMeleeAttackType(){return attack;}};
struct Manager{std::map<uint32,SpellInfo>records;SpellInfo const*GetSpellInfo(uint32 id){auto i=records.find(id);return i==records.end()?nullptr:&i->second;}}manager,*sSpellMgr=&manager;
bool Enabled=false;bool Applies(Player*p){return Enabled&&p&&p->area;}
uint64 now=1000;namespace GameTime{std::chrono::milliseconds GetGameTimeMS(){return std::chrono::milliseconds(now);}}
float lastChance=0;bool roll=true;int rolls=0;bool roll_chance_f(float chance){lastChance=chance;++rolls;return roll;}
struct AllSpellScript{AllSpellScript(char const*,std::initializer_list<int>){}};
@PRODUCTION@
int main(){Player p;Unit target;AuraEffect effect;p.effects={&effect};Spell spell{&p};ConsecratedWeaponHits hits;
assert(!ConsecratedWeaponReady());Enabled=true;assert(!ConsecratedWeaponReady());ConsecratedWeaponRegistered=true;assert(!ConsecratedWeaponReady());manager.records[977848]={977848,0};assert(!ConsecratedWeaponReady());manager.records[977827]={977827,2};assert(ConsecratedWeaponReady());
hits.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==1);assert(p.flags==TRIGGERED_FULL_MASK);assert(std::abs(lastChance-40)<.001);assert(effect.aura.state[977827]==1750);
now=1749;hits.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==1);now=1750;hits.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==2);
for(uint32 speed:{1000u,1500u,2000u,3500u}){p.speed=speed;now+=750;hits.OnSpellHitResult(&spell,&target,0,100,0,false);assert(std::abs(lastChance-speed*12.f/600)<.001);}
p.origins[986281]={78};manager.records[78]={78,2};spell.info={986281,1};now+=750;int before=p.casts;hits.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==before+1);
for(int x=0;x<13;++x){Spell s{&p};p.area=true;p.weaponAvailable=true;p.weapon.enchant=45;effect.aura.owner=7;effect.aura.item=10;effect.id=977848;if(x==0)s.triggered=true;if(x==1)s.automatic=true;if(x==2)s.info.Id=6603;if(x==3)s.info.Id=75;if(x==4)s.info.Id=977827;if(x==5)s.info.DmgClass=3;if(x==6)s.info.DmgClass=1;if(x==7)p.area=false;if(x==8)p.weaponAvailable=false;if(x==9)p.weapon.enchant=1;if(x==10)effect.aura.owner=8;if(x==11)effect.aura.item=11;if(x==12)s.attack=RANGED_ATTACK;now+=750;before=p.casts;hits.OnSpellHitResult(&s,&target,0,100,0,false);assert(p.casts==before);}
p.area=true;p.weaponAvailable=true;p.weapon.enchant=45;effect.aura.owner=7;effect.aura.item=10;spell={&p};now+=750;before=p.casts;hits.OnSpellHitResult(&spell,&target,1,100,0,false);hits.OnSpellHitResult(&spell,&target,0,0,0,false);assert(p.casts==before);roll=false;hits.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==before);roll=true;hits.OnSpellHitResult(&spell,&target,0,0,100,false);assert(p.casts==before);hits.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==before+1);
for(int route=0;route<4;++route){spell={&p};spell.info.offhand=route==1||route==3;if(route==2)spell.attack=OFF_ATTACK;if(route==3)spell.attack=BASE_ATTACK;now+=750;before=p.casts;hits.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==before+1);assert(p.lastAttack==((route==1||route==2)?OFF_ATTACK:BASE_ATTACK));}
target.alive=false;now+=750;before=p.casts;hits.OnSpellHitResult(&spell,&target,0,100,0,false);assert(p.casts==before);target.alive=true;
manager.records.erase(977827);assert(!ConsecratedWeaponReady());manager.records[977827]={977827,2};assert(ConsecratedWeaponReady());Enabled=false;assert(!ConsecratedWeaponReady());Enabled=true;assert(ConsecratedWeaponReady());}
