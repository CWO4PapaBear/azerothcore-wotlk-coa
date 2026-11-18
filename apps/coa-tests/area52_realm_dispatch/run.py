from pathlib import Path
import re
import subprocess
import tempfile

root=Path(__file__).resolve().parents[3]
coa=root/'src/server/coa'
def body(source, signature):
    pos=source.index(signature)
    start=source.index('{',pos)
    depth=0
    for end in range(start,len(source)):
        depth+=(source[end]=='{')-(source[end]=='}')
        if not depth:
            return source[pos:end+1]
    raise ValueError(signature)

local=(coa/'AscensionMysticEnchant.cpp').read_text()
remote=(coa/'RealmMysticEnchant.cpp').read_text()
local=body(local,'void AddSC_AscensionMysticEnchant()')
remote_registration=body(remote,'void AddAscensionMysticEnchantScripts()')
loader=(coa/'CoAScriptLoader.cpp').read_text()
entry=['AddAscensionMysticEnchantScripts','AddSC_AscensionMysticEnchant']
for name in entry:
    assert loader.count('    '+name+'();')==1
order=sorted(entry,key=lambda name:loader.index('    '+name+'();'))
guard=r'if \(!?Area52MysticRealm::UsesFreePickEngine\(\)\)\s*return;'
def registrations(name,src,claims):
    match=re.search(guard,src)
    assert match and src[src.index('{')+1:match.start()].strip()==''
    return 'void '+name+'(){'+match.group()+'\n'+claims+'\n}\n'
local_claims=re.findall(r'AscensionCompatOpcodes::Claim\(0x[0-9A-F]+, (?:Handle|QueueScrollRequest)\);',local)
assert len(local_claims)==9
remote_claim=re.search(r'for \(uint16 opcode : RealmMysticEnchant::CLIENT_OPCODES\)\s*AscensionCompatOpcodes::Claim\(opcode, &RealmMysticEnchant::QueueRequest\);',remote_registration)
assert remote_claim
registry=(coa/'AscensionCompatOpcodes.cpp').read_text().split('namespace AscensionCompatOpcodes',1)[1]
predicate=body((coa/'Area52MysticRealm.h').read_text(),'inline bool UsesFreePickEngine()')
wildcard=body((coa/'AscensionWildcard.cpp').read_text(),'bool PlaysWildcard(std::string_view realmModes)')
program=r"""
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <sstream>
#include <type_traits>
using uint16=std::uint16_t;
struct WorldSession{unsigned GetAccountId() const{return 1;}};
struct WorldPacket{uint16 opcode;uint16 GetOpcode() const{return opcode;}};
struct Config {
 std::string model="hero",realm="live",modes="";unsigned mask=0;bool enabled=true;
 template<class T>T GetOption(std::string key,T fallback){
  if constexpr(std::is_same_v<T,std::string>){if(key=="CoA.ClassModel")return model;if(key=="CoA.RealmType")return realm;if(key=="CoAChallenges.GameModes.Realm")return modes;}
  if constexpr(std::is_same_v<T,bool>)if(key=="CoA.Enable")return enabled;
  if constexpr(std::is_same_v<T,unsigned>)if(key=="CoA.GameModeMask")return mask;
  return fallback;
 }
} settings;auto* sConfigMgr=&settings;
namespace Acore {
std::vector<std::string> Tokenize(std::string_view v,char delim,bool){std::istringstream in{std::string(v)};std::vector<std::string> out;std::string part;while(std::getline(in,part,delim))out.push_back(part);return out;}
namespace String {std::string Trim(std::string v){auto first=v.find_first_not_of(" ");return first==v.npos?"":v.substr(first,v.find_last_not_of(" ")-first+1);}}
}
namespace AscensionFreePick {constexpr unsigned QualityMode=0x400;}
namespace AscensionCompatOpcodes {using Handler=bool(*)(WorldSession*,WorldPacket const&);}
int localCalls=0,upstreamCalls=0;
bool Handle(WorldSession*,WorldPacket const&){++localCalls;return false;}
bool QueueScrollRequest(WorldSession*,WorldPacket const&){++localCalls;return false;}
"""
program+='namespace AscensionWildcard{'+wildcard+'}\nnamespace Area52MysticRealm{'+predicate+'}\n'
program+='namespace AscensionCompatOpcodes'+registry
program+='namespace RealmMysticEnchant{\n'+'\n'.join(re.findall(r'constexpr uint16 CMSG_.*?;',remote))+'\n'
program+=re.search(r'constexpr std::array<uint16, 13> CLIENT_OPCODES = \{.*?};',remote,re.S).group()
program+='bool QueueRequest(WorldSession*,WorldPacket const&){++upstreamCalls;return false;}}\n'
program+=registrations(entry[0],remote_registration,remote_claim.group())+registrations(entry[1],local,'\n'.join(local_claims))
defs=(root/'src/server/game/Spells/Auras/SpellAuraDefines.h').read_text()
for symbol in ('ASCENSION_CLASSMASK_NPC_DAMAGE','ASCENSION_CLASSMASK_PLAYER_DAMAGE','ASCENSION_CLASSMASK_CREATURE_DAMAGE'):
    value=re.search(symbol+r' = (\d+)',defs).group(1)
    program+='constexpr int '+symbol+'='+value+';\n'
program+=r"""
using uint8=std::uint8_t;
enum {SPELL_EFFECT_APPLY_AURA=1,SPELL_AURA_ADD_PCT_MODIFIER=2,SPELL_AURA_OVERRIDE_CLASS_SCRIPTS=3,
      TARGET_UNIT_CASTER=4,SPELL_AURA_MOD_DAMAGE_DONE_VERSUS=168,SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE=303};
struct Target{int id=0;int GetTarget(){return id;}};
struct SpellEffectInfo {int Effect=1,ApplyAuraName=2,MiscValue=36,MiscValueB=0,SpellClassMask=1;Target TargetA{4},TargetB{};};
struct SpellInfo{int Id=3045,SpellFamilyName=9;SpellEffectInfo Effects[3];};
#define LOG_ERROR(...) ((void)0)
"""
conditional=(coa/'AscensionConditionalCombat.cpp').read_text()
program+=body(conditional,'void ApplyArea52TargetDamageContracts(')
program+=body((coa/'AscensionWitchHunterCompletion.cpp').read_text(),'void ConvertCreatureTypeDamage(')
gate=re.search(r'if \(Area52MysticRealm::UsesFreePickEngine\(\)\)\s+ApplyArea52TargetDamageContracts\(spellInfo\);',conditional).group()
program+='void CheckTargets(bool local){SpellInfo s;auto* spellInfo=&s;'+gate+'''
assert(s.Effects[0].ApplyAuraName==(local?303:2));
assert(s.Effects[0].MiscValueB==(local?20014:0));
SpellInfo w;auto& e=w.Effects[0];e.ApplyAuraName=3;e.MiscValue=20014;e.MiscValueB=4;
ConvertCreatureTypeDamage(&w,0);assert(e.ApplyAuraName==168&&e.MiscValue==4&&e.MiscValueB==20014);}
'''
compat=(coa/'AscensionCompat.cpp').read_text()
table=re.search(r'constexpr std::array<uint16, \d+> QUEUED_EXTENSION_OPCODES = \{.*?};',compat,re.S).group()
for symbol in re.findall(r'\bCMSG_\w+',table):
    program+=re.search(r'constexpr uint16 '+symbol+r' = 0x[\dA-Fa-f]+;',compat).group()+'\n'
program+=table
program+='struct AscensionCollectionService{static AscensionCollectionService& Instance(){static AscensionCollectionService v;return v;}void QueueClientPacket(unsigned,WorldPacket const&){++upstreamCalls;}};'
program+='bool ReceiveEquipment(WorldSession* session,WorldPacket const& packet){auto opcode=packet.GetOpcode();'
program+=body(compat,'if (!(opcode == CMSG_SET_SPECIALIZATION_EQUIPMENT_SET_ID')
program+='return AscensionCompatOpcodes::Dispatch(session,packet);}'
unit=(root/'src/server/game/Entities/Unit/Unit.cpp').read_text()
program+='''
struct Consumer {
struct Spell { bool valid; bool ValidateAttribute6SpellDamageMods(void*,void*,bool){return valid;} };
struct Aura {bool affected;int mask;int selector;int GetMiscValueB(){return selector;}int GetMiscValue(){return mask;}bool IsAffectedOnSpell(Spell*){return affected;}};
struct Target {bool player;bool IsPlayer(){return player;}};
enum { DOT=1 };
'''
npc=re.findall(r'if \(aurEff->GetMiscValueB\(\) == ASCENSION_CLASSMASK_NPC_DAMAGE \|\|.*?ValidateAttribute6SpellDamageMods\(this, aurEff, [^;]+;',unit,re.S)
creature=re.findall(r'if \(aurEff->GetMiscValueB\(\) == ASCENSION_CLASSMASK_CREATURE_DAMAGE.*?return false;\s+return [^;]+;',unit,re.S)
assert len(npc)==len(creature)==2
for index in range(2):
    for name,fragment in [('npc',npc[index]),('creature',creature[index])]:
        program+='bool '+name+str(index)+'(Target* victim,Spell* spellProto,Aura* aurEff,int creatureTypeMask,int damagetype){'+fragment+'return false;}\n'
program+='''void check(){Spell s{true};Aura a{true,4,20014};Target npcTarget{false},playerTarget{true};
for(auto test:{&Consumer::npc0,&Consumer::npc1}){
assert((this->*test)(&playerTarget,&s,&a,4,0));assert(!(this->*test)(&npcTarget,&s,&a,4,0));
a.selector=20013;assert((this->*test)(&npcTarget,&s,&a,4,0));assert(!(this->*test)(&playerTarget,&s,&a,4,0));
a.affected=false;assert(!(this->*test)(&npcTarget,&s,&a,4,0));a.affected=true;
s.valid=false;assert(!(this->*test)(&npcTarget,&s,&a,4,0));s.valid=true;a.selector=20014;}
for(auto test:{&Consumer::creature0,&Consumer::creature1}){
assert((this->*test)(&npcTarget,&s,&a,4,0));assert(!(this->*test)(&npcTarget,&s,&a,8,0));
a.affected=false;assert(!(this->*test)(&npcTarget,&s,&a,4,0));a.affected=true;}
assert(!creature1(&npcTarget,nullptr,&a,4,0));}};'''
program+='int main(int argc,char** argv){Consumer{}.check();std::string mode=argv[1];if(mode=="quality")settings.mask=0x400;if(mode=="wildcard")settings.modes="Other, WildCard";if(mode=="mask")settings.mask=1;if(mode=="reborn")settings.model="wcr";bool local=mode=="freepick"||mode=="quality";'
program+='CheckTargets(local);'+''.join(name+'();' for name in order)
program+='for(uint16 op:{0x60C,0x739,0x60F,0x617,0x611,0x603,0x607,0x733}){localCalls=upstreamCalls=0;AscensionCompatOpcodes::Dispatch(nullptr,{op});assert(localCalls==int(local)&&upstreamCalls==int(!local));}WorldSession session;localCalls=upstreamCalls=0;ReceiveEquipment(&session,{0x73A});assert(localCalls==int(local)&&upstreamCalls==int(!local));}'
with tempfile.TemporaryDirectory() as tmp:
    source=Path(tmp)/'dispatch.cpp'
    exe=Path(tmp)/'dispatch'
    source.write_text(program)
    subprocess.run(['c++','-std=c++20',source,'-o',exe],check=True)
    for mode in ('freepick','quality','wildcard','mask','reborn'):
        subprocess.run([exe,mode],check=True)
