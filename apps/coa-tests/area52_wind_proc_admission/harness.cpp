#include <cassert>
#include <set>
using uint32=unsigned;
constexpr unsigned SPELL_PROC_FLAG_MASK=65536,PERIODIC_PROC_FLAG_MASK=262144,REQ_SPELL_PHASE_PROC_FLAG_MASK=65536,TAKEN_HIT_PROC_FLAG_MASK=131072,DONE_HIT_PROC_FLAG_MASK=65536,PROC_SPELL_PHASE_CAST=1,PROC_HIT_NORMAL=1,PROC_HIT_CRITICAL=2,PROC_HIT_ABSORB=1024,PROC_ATTR_REQ_SPELLMOD=8,PROC_ATTR_USE_STACKS_FOR_CHARGES=16;
struct Aura;struct Spell{std::set<Aura*>m_appliedMods;};struct Player{Player*ToPlayer(){return this;}};
namespace AscensionFreePick{bool Applies(Player*){return true;}}
struct ProcEventInfo{unsigned flags=65536,type=1,phase=1,hit=0;Spell*spell=nullptr;unsigned GetTypeMask(){return flags;}unsigned GetSpellTypeMask(){return type;}unsigned GetSpellPhaseMask(){return phase;}unsigned GetHitMask(){return hit;}Spell const*GetProcSpell(){return spell;}};
struct SpellProcEntry{unsigned ProcFlags=65536,SpellTypeMask=1,SpellPhaseMask=1,HitMask=0,AttributesMask=8;};
bool eligible(SpellProcEntry const&procEntry,ProcEventInfo&eventInfo){@FLAGS@ @PHASE@}
struct Aura{unsigned GetId()const{return 997084;}Player*GetCaster()const{return nullptr;}bool IsUsingCharges()const{return true;}bool owns(SpellProcEntry const*procEntry,ProcEventInfo&eventInfo)const{@OWNERSHIP@ return true;}};
int main(){SpellProcEntry entry;ProcEventInfo e;Spell spell;Aura aura;assert(eligible(entry,e));assert(!aura.owns(&entry,e));e.spell=&spell;assert(!aura.owns(&entry,e));spell.m_appliedMods.insert(&aura);assert(aura.owns(&entry,e));e.phase=2;assert(!eligible(entry,e));e.phase=1;e.flags=16;assert(!eligible(entry,e));e.flags=65536;e.type=2;assert(!eligible(entry,e));e.type=1;entry.ProcFlags=16;assert(!eligible(entry,e));entry.ProcFlags=65536;assert(eligible(entry,e));Aura other;spell.m_appliedMods={&other};assert(!aura.owns(&entry,e));}
