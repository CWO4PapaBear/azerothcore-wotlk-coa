#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
using uint32=std::uint32_t;using int32=std::int32_t;
constexpr int SPEC_MASK_ALL=3;
bool Enabled=true;
constexpr uint32 Exorcist=986280,GhostlyBlow=983240;
constexpr std::array<uint32,13> HeroicStrike={78,284,285,1608,11564,11565,11566,11567,25286,29707,30324,47449,47450};
constexpr std::array<uint32,10> ExorcistSlash={986281,986347,986348,986349,986350,986351,986352,986353,986354,986355};
struct Session {bool PlayerLogout(){return false;}};
constexpr uint32 Light=926573,Hook=92580,Deconstruction=954081;
constexpr std::array<uint32,8> Consecration={26573,20116,20922,20923,20924,27173,48818,48819};
constexpr std::array<uint32,9> Devastate={20243,302035,302036,302037,302038,30016,30022,47497,47498};
constexpr std::array<uint32,9> Deconstruct={954082,954443,954444,954445,954446,954447,954448,954449,954450};
struct Aura {int Stack=1,Duration=0,Maximum=0;int GetStackAmount(){return Stack;}void SetDuration(int v){Duration=v;}void SetMaxDuration(int v){Maximum=v;}};
struct Player {
 Session session;Session* GetSession(){return &session;}Player* ToPlayer(){return this;}uint32 GetGUID(){return 1;}void RemoveAurasDueToSpell(uint32 id,uint32){Auras.erase(id);}
 int Level=80;float AP=1000;bool IsHuman=false;Aura Sunder,Buff;
 bool IsPlayer(){return IsHuman;}int GetLevel(){return Level;}float GetTotalAttackPowerValue(int){return AP;}
 Aura* GetAura(uint32){return &Sunder;}Aura* AddAura(uint32,Player*){return &Buff;}
 bool Allowed=true;std::set<uint32> Permanent,Temporary,Auras;std::map<uint32,uint32> Replacements;
 bool HasAura(uint32 id){return Auras.contains(id);}
 bool HasSpell(uint32 id){return Permanent.contains(id)||Temporary.contains(id);}
 bool HasActiveSpell(uint32 id){return HasSpell(id);}
 void learnSpell(uint32 id,bool){Temporary.insert(id);}
 void removeSpell(uint32 id,int,bool){Temporary.erase(id);}
 void SetTemporarySpellReplacement(uint32 id,uint32 value){Replacements[id]=value;}
 uint32 GetTemporarySpellReplacement(uint32 id){return Replacements[id];}
};
using Unit=Player;using SpellEffIndex=int;using SpellCastResult=int;constexpr int SPELL_FAILED_BAD_TARGETS=1,SPELL_CAST_OK=0,BASE_ATTACK=0;
bool Applies(Player* player){return Enabled&&player&&player->Allowed;}
struct flag96 {};
struct Effect {int BasePoints=0;float BonusMultiplier=0;int Effect=0,ApplyAuraName=0,DieSides=0,MiscValue=0,TargetA=0;flag96 SpellClassMask;};
struct SpellInfo {uint32 Id=0,SpellLevel=1,SchoolMask=0,CasterAuraSpell=0,ProcFlags=0,ProcCharges=0,AuraInterruptFlags=0;std::array<Effect,3> Effects;};

constexpr int EFFECT_0=0,EFFECT_1=1,EFFECT_2=2,AURA_INTERRUPT_FLAG_LEAVE_COMBAT=1,SPELL_SCHOOL_MASK_NORMAL=1,SPELL_SCHOOL_MASK_HOLY=2,SPELL_AURA_DUMMY=4,SPELL_AURA_ABILITY_IGNORE_AURASTATE=262,SPELL_EFFECT_APPLY_AURA=6,SPELL_AURA_ADD_FLAT_MODIFIER=107,SPELLMOD_CRITICAL_CHANCE=7,TARGET_UNIT_CASTER=1;
constexpr uint32 PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS=16,PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS=256,PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS=1024,PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG=2048;
struct Manager {SpellInfo info;SpellInfo const* GetSpellInfo(uint32 id){info.Id=id;return &info;}} manager;Manager* sSpellMgr=&manager;
namespace AscensionMysticEnchant {bool GrantsActive(Player* p,uint32 id){return p->HasAura(id);}void SetGrantedSpell(Player* p,uint32,uint32 id,bool active,bool=false){if(active)p->Temporary.insert(id);else p->Temporary.erase(id);}}
void SynchronizeCryotherapist(Player*){}
void ConfigureNimbleMind(SpellInfo*){}
void ConfigureStayFrosty(SpellInfo*){}
void ConfigureWardOfLight(SpellInfo*){}
void ConfigureCosmicReprisal(SpellInfo*){}
void ConfigureWorldforged(SpellInfo*){}
namespace AscensionFreePick {constexpr uint32 AgilityEffectDamageKey=1;float AgilityAttackPower(Player*,bool){return 0;}}
struct Cast {void SetScriptValue(uint32,uint32){}};
DURATION
REPLACEMENT
SYNCHRONIZE
struct Contracts {CONTRACT};
struct HookTest {
 Cast cast;Cast* GetSpell(){return &cast;}
 Player* Caster;Unit* Target;int Value=3;
 Player* GetCaster(){return Caster;}Unit* GetExplTargetUnit(){return Target;}
 int GetEffectValue(){return Value;}void SetEffectValue(int value){Value=value;}
 HOOK_CHECK
 HOOK_DAMAGE
};
struct DeconstructTest {
 Cast cast;Cast* GetSpell(){return &cast;}
 Player* Caster;Unit* Target;int Value=15,DamageDone=1;
 Player* GetCaster(){return Caster;}Unit* GetHitUnit(){return Target;}int GetHitDamage(){return DamageDone;}
 int GetEffectValue(){return Value;}void SetEffectValue(int value){Value=value;}
 DECON_DAMAGE
 DECON_HIT
};
int main(){
 Player caster,target;HookTest hook;hook.Caster=&caster;hook.Target=&target;assert(hook.Check()==SPELL_CAST_OK);
 target.IsHuman=true;assert(hook.Check()==SPELL_FAILED_BAD_TARGETS);target.IsHuman=false;
 hook.Damage(0);assert(hook.Value==683);
 DeconstructTest decon;decon.Caster=&caster;decon.Target=&target;decon.Damage(1);assert(decon.Value==210);
 target.Sunder.Stack=1;decon.Hit();assert(caster.Buff.Duration==3000);
 target.Sunder.Stack=5;decon.Hit();assert(caster.Buff.Duration==15000);
 caster.Buff.Duration=0;decon.DamageDone=0;decon.Hit();assert(caster.Buff.Duration==0);

 assert(DeconstructionDuration(0)==0&&DeconstructionDuration(1)==3000&&DeconstructionDuration(5)==15000&&DeconstructionDuration(6)==15000);
 Player p;p.Auras={Light,Hook,Deconstruction};p.Permanent.insert(Consecration.begin(),Consecration.end());
 p.Permanent.insert(Devastate.begin(),Devastate.end());Synchronize(&p);
 for(unsigned i=0;i<8;++i)assert(p.Replacements[Consecration[i]]==926574+i*2);
 for(unsigned i=0;i<9;++i)assert(p.Replacements[Devastate[i]]==Deconstruct[i]);
 assert(p.HasSpell(92583));p.Permanent.insert(92583);Synchronize(&p);assert(p.HasSpell(92583));
 p.Permanent.erase(Devastate[4]);Synchronize(&p);assert(!p.HasSpell(Deconstruct[4]));
 p.Auras.insert(81315);Synchronize(&p);assert(!p.Replacements[26573]);assert(p.Replacements[20243]);
 p.Auras.clear();Synchronize(&p);assert(p.Temporary.empty());assert(p.HasSpell(20243));assert(p.HasSpell(92583));
 p.Auras={Light,Hook,Deconstruction};p.Allowed=false;Synchronize(&p);assert(p.Temporary.empty());
 SpellInfo spell;spell.Id=954082;spell.Effects[2].BasePoints=14;Contracts c;c.OnLoadSpellCustomAttr(&spell);
 c.OnLoadSpellCustomAttr(&spell);assert(spell.Effects[1].BasePoints==14&&spell.Effects[1].BonusMultiplier==0.3f&&spell.Effects[2].BasePoints==-1);
 spell.Id=92583;c.OnLoadSpellCustomAttr(&spell);assert(spell.Effects[0].BonusMultiplier==0.2f);
 spell.Id=926590;c.OnLoadSpellCustomAttr(&spell);assert(spell.Effects[1].BasePoints==-2);
 Enabled=false;spell.Effects[0].BonusMultiplier=0.8f;c.OnLoadSpellCustomAttr(&spell);assert(spell.Effects[0].BonusMultiplier==0.8f);
}
