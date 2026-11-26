#include <cassert>
#include <cstdint>
#include <initializer_list>
using uint32=uint32_t;using uint8=uint8_t;using DamageEffectType=int;struct SpellInfo{};
struct Unit{uint8 effect=0;float mod=0;uint32 count=0,damage=0;Unit*seen=nullptr;SpellInfo const*info=nullptr;int type=0;uint32 SpellDamageBonusDone(Unit*v,SpellInfo const*s,uint32 d,DamageEffectType t,uint8 e,float m=0,uint32 c=1){seen=v;info=s;damage=d;type=t;effect=e;mod=m;count=c;return 402;}};
uint32 copy(Unit*owner,Unit*victim,SpellInfo const*spellProto,uint32 pdamage,DamageEffectType damagetype,uint8 effIndex,float TotalMod,uint32 stack){@FORWARD@}
int main(){Unit owner,victim;SpellInfo spell;for(uint8 effect:{0,1,2})for(float mod:{0.0f,1.25f})for(uint32 stack:{1u,3u}){assert(copy(&owner,&victim,&spell,300,2,effect,mod,stack)==201);assert(owner.seen==&victim&&owner.info==&spell&&owner.damage==300&&owner.type==2&&owner.effect==effect&&owner.mod==mod&&owner.count==stack);}}
