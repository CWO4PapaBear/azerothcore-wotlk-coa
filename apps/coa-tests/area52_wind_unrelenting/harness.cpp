#include <cassert>
#include <map>
#include <set>
#include <algorithm>
#include <cstdint>
using uint32=unsigned;using uint8=unsigned char;using int32=int;using AuraRemoveMode=int;
constexpr int EFFECT_0=0,EFFECT_2=2,SPELL_AURA_DUMMY=4,SPELL_MISS_NONE=0;
constexpr unsigned SPELL_ATTR3_IGNORE_CASTER_MODIFIERS=0x20000000,SPELL_ATTR3_SUPPRESS_CASTER_PROCS=0x10000,SPELL_ATTR1_NO_THREAT=0x400;
bool Area52WindMetadataEnabled=false;
struct Aura {unsigned id=0;unsigned stack=1;unsigned GetId(){return id;}unsigned GetStackAmount(){return stack;}};
struct AuraApplication {Aura aura;Aura*GetBase(){return &aura;}};
struct Player;
struct Unit {Player*player=nullptr;std::map<std::pair<unsigned,unsigned>,Aura> auras;Player*ToPlayer(){return player;}Aura*GetAura(unsigned id,unsigned owner){auto i=auras.find({id,owner});return i==auras.end()?nullptr:&i->second;}void RemoveAurasDueToSpell(unsigned id,unsigned owner){auras.erase({id,owner});}};
struct Player:Unit {bool area=true;unsigned guid=7;std::set<unsigned> talents;unsigned casts=0,last=0;Player(){player=this;}bool HasAura(unsigned id)const{return talents.contains(id)||auras.contains({id,guid});}unsigned GetGUID(){return guid;}bool MatchesSpellInteraction(unsigned id,unsigned first)const{return id==first||(id==997097&&first==403)||(id==777001&&first==421);}void CastSpell(Unit*target,unsigned id,bool){++casts;last=id;target->auras[{id,guid}]={id,1};}};
namespace AscensionFreePick {bool Applies(Player const*p){return p&&p->area;}}
struct SpellInfo {unsigned Id=0,AttributesEx=0,AttributesEx3=0;struct Effect{int ApplyAuraName=42;float RealPointsPerLevel=1;unsigned Amplitude=1000;}Effects[3];};
struct Spell {Player*caster=nullptr;SpellInfo info;bool triggered=false;std::map<unsigned,unsigned>values;bool IsTriggered(){return triggered;}Unit*GetCaster(){return caster;}SpellInfo const*GetSpellInfo(){return &info;}unsigned GetScriptValue(unsigned id){return values[id];}void SetScriptValue(unsigned id,unsigned v){values[id]=v;}};
struct TargetInfo {int damage=1000,damageBeforeTakenMods=1200;};
template<class T>int CalculatePct(T n,int p){return n*p/100;}
@RANK@ @ELIGIBLE@ @CONFIG@
struct Probe {@CAST@ @DAMAGE@ @HIT@ @REMOVE@};
int main(){Probe probe;Unit enemy;Player p;
for(unsigned rank=1;rank<=3;++rank){p.talents={30663+rank};for(unsigned id:{403u,324u,51505u,954807u,997097u}){Spell cast;cast.caster=&p;cast.info.Id=id;auto old=p.casts;probe.OnSpellBeforeEffects(&cast,&p,&cast.info);probe.OnSpellBeforeEffects(&cast,&p,&cast.info);assert(p.casts==old+1&&p.last==411999+rank);}
for(bool triggered:{false,true}){Spell cast;cast.caster=&p;cast.info.Id=triggered?403:133;cast.triggered=triggered;auto old=p.casts;probe.OnSpellBeforeEffects(&cast,&p,&cast.info);assert(p.casts==old);}
for(unsigned stacks=0;stacks<=3;++stacks){p.auras[{412002,7}]={412002,stacks};Spell cast;cast.caster=&p;cast.info.Id=421;probe.OnSpellBeforeEffects(&cast,&p,&cast.info);TargetInfo target;probe.OnSpellCalculatedTarget(&cast,&enemy,target);bool empowered=rank==3&&stacks==3;assert(target.damage==(empowered?1150:1000));assert(target.damageBeforeTakenMods==(empowered?1380:1200));p.auras.erase({412002,7});TargetInfo second;probe.OnSpellCalculatedTarget(&cast,&enemy,second);assert(second.damage==target.damage);enemy.auras.clear();probe.OnSpellHitResult(&cast,&enemy,0,100,0,false);assert(bool(enemy.GetAura(412004,7))==empowered);}
p.auras[{411999+rank,7}]={411999+rank,3};p.auras[{411999+rank,8}]={411999+rank,3};AuraApplication app{{30663+rank,1}};probe.OnAuraRemove(&p,&app,0);assert(!p.GetAura(411999+rank,7)&&p.GetAura(411999+rank,8));}
for(unsigned id:{8042u,8050u,8056u})for(unsigned rank:{30675u,30678u,30679u}){p.talents={rank};enemy.auras.clear();enemy.auras[{412004,7}]={412004,1};enemy.auras[{412004,8}]={412004,1};Spell cast;cast.caster=&p;cast.info.Id=id;probe.OnSpellHitResult(&cast,&enemy,0,1,0,false);assert(bool(enemy.GetAura(412004,7))==(rank!=30679));assert(bool(enemy.GetAura(412005,7))==(rank==30679));assert(enemy.GetAura(412004,8));}
for(unsigned id:{30664u,30665u,30666u,30675u,30678u,30679u}){SpellInfo info;info.Id=id;Area52WindMetadataEnabled=false;ConfigureArea52Unrelenting(&info);assert(info.Effects[id<=30666?2:0].ApplyAuraName==42);Area52WindMetadataEnabled=true;ConfigureArea52Unrelenting(&info);ConfigureArea52Unrelenting(&info);assert(info.Effects[id<=30666?2:0].ApplyAuraName==4);}
for(unsigned id:{45284u,45297u}){SpellInfo info;info.Id=id;ConfigureArea52Unrelenting(&info);assert(info.AttributesEx3&SPELL_ATTR3_IGNORE_CASTER_MODIFIERS);assert(info.AttributesEx3&SPELL_ATTR3_SUPPRESS_CASTER_PROCS);assert(info.AttributesEx&SPELL_ATTR1_NO_THREAT);}
p.area=false;p.talents={30666};Spell cast;cast.caster=&p;cast.info.Id=403;auto old=p.casts;probe.OnSpellBeforeEffects(&cast,&p,&cast.info);assert(p.casts==old);probe.OnSpellBeforeEffects(nullptr,nullptr,nullptr);
}
