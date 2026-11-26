#include <cassert>
#include <initializer_list>
using uint32=unsigned;
constexpr unsigned SPELL_ATTR3_SUPPRESS_CASTER_PROCS=0x10000,SPELL_ATTR4_SUPPRESS_WEAPON_PROCS=0x800000,SPELL_ATTR0_CANCELS_AUTO_ATTACK_COMBAT=1,SPELL_DAMAGE_CLASS_MELEE=2,SPELL_DAMAGE_CLASS_RANGED=3,SPELL_MISS_IMMUNE2=99;
bool Area52WindMetadataEnabled=false;
struct SpellInfo{unsigned Id,AttributesEx3=0,AttributesEx4=0;int DmgClass=2;bool HasAttribute(unsigned a){return a==0x10000?(AttributesEx3&a):a==0x800000?(AttributesEx4&a):false;}};
@CONFIG@
struct Unit{bool CanProc(){return true;}bool IsPlayer(){return true;}};
bool CanExecuteTriggersOnHit(int){return true;}
int main(){Unit unit;auto*unitTarget=&unit;auto*caster=&unit;int mask=1,missInfo=0;
for(bool area:{false,true}){Area52WindMetadataEnabled=area;for(unsigned id:{57841u,57842u}){SpellInfo info{id};ConfigureArea52WindTalents(&info);auto*m_spellInfo=&info;bool proc=@PROC@;bool weapon=@WEAPON@;assert(proc==(!area||id==57841));assert(weapon==proc);}}}
