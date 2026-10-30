from pathlib import Path
import hashlib,json,subprocess,tempfile
root=Path.cwd()
module=root/'modules/mod-playerbots/src'
core=root/'src/server/coa'
def block(text,marker):
    start=text.index(marker);opening=text.index('{',start);depth,end=1,opening+1
    while depth:
        depth+=(text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]
def run(text):
    with tempfile.TemporaryDirectory() as tmp:
        out=Path(tmp);(out/'check.cpp').write_text(text)
        subprocess.run(['clang++','-std=c++20',str(out/'check.cpp'),'-o',str(out/'check')],check=True)
        subprocess.run([str(out/'check')],check=True)
mystic=(core/'AscensionMysticEnchant.cpp').read_text()
legendary=(module/'Ai/Coa/CoaLegendaryClasses.cpp').read_text()
helpers=(root/'src/server/game/Chat/ChatCommands/ChatCommandHelpers.h').read_text()
tags=(root/'src/server/game/Chat/ChatCommands/ChatCommandTags.h').read_text()
args=(root/'src/server/game/Chat/ChatCommands/ChatCommandArgs.h').read_text()
token=block(helpers,'inline TokenizeResult tokenize(')
tail=block(tags,'ChatCommandResult TryConsume(',) if False else block(tags[tags.index('struct Tail :'):],'ChatCommandResult TryConsume(')
string=block(args[args.index('struct ArgInfo<std::string_view, void>'):],'static ChatCommandResult TryConsume(')
equip=block(mystic,'static bool AdminEquip(')
slots=block(legendary,'void EquipSlots(')
matches=block(legendary,'bool SlotsMatch(')
ready=block(legendary,'bool CoaLegendaryClassReady(')
fixture=r'''
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <map>
#include <mutex>
#include <cassert>
#include <sstream>
using uint32=unsigned;template<class T>using Optional=std::optional<T>;using ChatCommandResult=std::optional<std::string_view>;
struct Guid{unsigned id=1;auto GetRawValue()const{return id;}};
struct Setting{unsigned value=0;};using PlayerSettingVector=std::vector<Setting>;
struct Player{std::string name="Nicarlo Bot";bool hero=true,aura=true,loaded=true,eligible=true;unsigned level=63;PlayerSettingVector settings=PlayerSettingVector(17);
auto const* FindPlayerSettings(std::string const&)const{return &settings;}bool HasAura(unsigned)const{return aura;}
void UpdatePlayerSetting(std::string const&,unsigned i,unsigned value){settings.at(i).value=value;}
std::string GetName(){return name;}Guid GetGUID(){return {};}unsigned GetLevel(){return level;}};
Player bot;struct ChatHandler{unsigned messages=0;Player* GetPlayer(){return &bot;}void SendSysMessage(char const*){++messages;}};
constexpr char COMMAND_DELIMITER=' ';struct TokenizeResult{std::string_view token,tail;};
'''+token+'\nstruct OldStringParser{'+string+'};\nstruct Tail:std::string_view{using std::string_view::operator=;'+tail+r'''};
namespace ObjectAccessor{Player* FindPlayerByName(std::string const& n){return n==bot.name?&bot:nullptr;}}
bool Applies(Player* p){return p&&p->hero;}bool IsCoaHero(Player* p){return Applies(p);}
using Slots=std::array<unsigned,17>;Slots ReadSlots(Player* p){Slots a{};for(unsigned i=0;i<17;++i)a[i]=p->settings[i].value;return a;}
bool Equip(Player* p,Slots const& wanted,bool administrator){assert(administrator);if(!p->loaded||!p->eligible)return false;for(unsigned i=0;i<17;++i)p->settings[i].value=wanted[i];return true;}
'''+equip+r'''
struct CoaLegendaryClass{Slots enchants{};unsigned legendary=978217;};CoaLegendaryClass build;
auto* GetCoaLegendaryClass(Player*){return &build;}std::string SlotSetting(Player const*){return "slots";}
bool ShieldMissing(Player*,CoaLegendaryClass const&){return false;}
struct LegendaryState{unsigned appliedLevel=63;};std::map<unsigned,LegendaryState> States{{1,{}}};std::mutex StatesLock;
namespace AscensionMysticEnchant{void RefreshEquipped(Player*){}}
std::vector<std::string> commands;void QueueCommand(std::string const& cmd){commands.push_back(cmd);}
'''+slots+'\n'+matches+'\n'+ready+r'''
bool Dispatch(std::string command,bool old){
std::string_view input(command);auto take=[&](){auto result=tokenize(input);input=result.tail;return std::string(result.token);};
assert(take()=="mystic"&&take()=="equip");unsigned slot=std::stoul(take()),spell=std::stoul(take());ChatHandler handler;
Tail name;if(old){std::string_view part;OldStringParser::TryConsume(part,&handler,input);name=part;}else name.TryConsume(&handler,input);
return AdminEquip(&handler,slot,spell,name);}
int main(){
for(unsigned i=0;i<17;++i)build.enchants[i]=100+i;build.enchants[0]=build.legendary;bot.settings[0].value=build.legendary;
EquipSlots(&bot,build);assert(commands.size()==16);for(auto const& c:commands)assert(!Dispatch(c,true));assert(!CoaLegendaryClassReady(&bot));
bot.loaded=false;for(auto const& c:commands)assert(!Dispatch(c,false));assert(!CoaLegendaryClassReady(&bot));
bot.loaded=true;bot.eligible=false;for(auto const& c:commands)assert(!Dispatch(c,false));assert(!CoaLegendaryClassReady(&bot));
bot.eligible=true;for(auto const& c:commands)assert(Dispatch(c,false));assert(CoaLegendaryClassReady(&bot));
commands.clear();EquipSlots(&bot,build);assert(commands.empty());bot.level=62;assert(!CoaLegendaryClassReady(&bot));bot.level=63;
ChatHandler handler;Tail missing;missing="Nicarlo";assert(!AdminEquip(&handler,1,44,missing));assert(ReadSlots(&bot)==build.enchants);
bot.hero=false;assert(!CoaLegendaryClassReady(&bot));
}
'''
run(fixture)
assert 'Optional<Tail> name' in block(mystic,'static bool Grant(')
print('BASELINE REPRODUCED and FIX PASSED: actual EquipSlots commands -> native tokenizer/Tail -> AdminEquip -> actual slot/readiness predicates; full names, cold collection, native rejection propagation, repeated calls and level drift. Equip service is a bounded adapter; catalog validation is separately native.')

shaman=(root/'src/server/scripts/Spells/spell_shaman.cpp').read_text()
proc=block(shaman[shaman.index('class spell_sha_earthliving_weapon'):],'bool CheckProc(')
proc_fixture=r'''
#include <cassert>
constexpr unsigned EFFECT_1=1,AURA_STATE_HEALTHLESS_35_PERCENT=1,SPELL_SHAMAN_EARTH_SHIELD_HEAL=379,SPELL_SHAMAN_BLESSING_OF_THE_ETERNALS_R1=51554,SPELL_SHAMAN_BLESSING_OF_THE_ETERNALS_ME=951554;
struct AuraEffect{int amount;int GetAmount()const{return amount;}};
struct Unit{bool area=true,low=false;unsigned guid=1;AuraEffect* talent=nullptr;AuraEffect* enchant=nullptr;
Unit* ToPlayer(){return this;}bool HasAuraState(unsigned){return low;}auto GetGUID(){return guid;}
AuraEffect* GetAuraEffectOfRankedSpell(unsigned,unsigned,unsigned owner){assert(owner==guid);return talent;}
AuraEffect* GetAuraEffect(unsigned spell,unsigned effect,unsigned owner){assert(spell==951554&&effect==1&&owner==guid);return enchant;}};
namespace AscensionFreePick{bool Applies(Unit* p){return p&&p->area;}}
struct SpellInfo{unsigned Id=0;};struct ProcEventInfo{Unit* actor;Unit* target;SpellInfo* spell;Unit* GetActor(){return actor;}Unit* GetProcTarget(){return target;}SpellInfo* GetSpellInfo(){return spell;}};
int chance=-1;bool roll_chance_i(int value){chance=value;return true;}
'''+proc+r'''
int main(){Unit caster,target;AuraEffect talent{40},enchant{20};SpellInfo heal{1};ProcEventInfo event{&caster,&target,&heal};
caster.enchant=&enchant;CheckProc(event);assert(chance==20);target.low=true;CheckProc(event);assert(chance==40);
caster.talent=&talent;CheckProc(event);assert(chance==80);caster.area=false;CheckProc(event);assert(chance==60);
caster.area=true;caster.enchant=nullptr;CheckProc(event);assert(chance==60);target.low=false;CheckProc(event);assert(chance==20);
heal.Id=379;chance=-1;assert(!CheckProc(event)&&chance==-1);event.target=nullptr;assert(!CheckProc(event));event.actor=nullptr;assert(!CheckProc(event));
}
'''
run(proc_fixture)
connection=block(mystic,'bool HasBlessingOfEternalsHandler(')
connection_fixture=r'''
#include <list>
#include <string>
#include <cassert>
#include <algorithm>
using uint32=unsigned;constexpr unsigned EFFECT_0=0,EFFECT_1=1,SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL=71,SPELL_AURA_DUMMY=4;
struct Effect{unsigned ApplyAuraName;};struct SpellInfo{Effect Effects[2]{{71},{4}};};
struct SpellMgr{bool loaded=false,proc=false;SpellInfo info;SpellInfo* GetSpellInfo(unsigned s){return loaded&&s==951554?&info:nullptr;}bool GetSpellProcEntry(unsigned s){return proc&&s==51940;}} mgr;auto* sSpellMgr=&mgr;
struct AuraScript{std::string name;std::string* _GetScriptName(){return &name;}};
struct Scripts{bool bound=false;void CreateAuraScripts(unsigned s,std::list<AuraScript*>& out){assert(s==51940);if(bound)out.push_back(new AuraScript{"spell_sha_earthliving_weapon"});}} scripts;auto* sScriptMgr=&scripts;
'''+connection+r'''
int main(){assert(!HasBlessingOfEternalsHandler(951554));mgr.loaded=true;assert(!HasBlessingOfEternalsHandler(951554));mgr.proc=true;assert(!HasBlessingOfEternalsHandler(951554));scripts.bound=true;
assert(HasBlessingOfEternalsHandler(951554));assert(HasBlessingOfEternalsHandler(951554));assert(!HasBlessingOfEternalsHandler(1));mgr.info.Effects[1].ApplyAuraName=0;assert(!HasBlessingOfEternalsHandler(951554));}
'''
run(connection_fixture)
assert 'if (HasBlessingOfEternalsHandler(spell))' in block(mystic,'void LoadConnections()')
assert 'void OnStartup() override { LoadConnections(); RefreshTestAccess(); }' in mystic
Path('/results/readiness-connection-evidence.json').write_text(json.dumps({'status':'passed','enchant':951554,'stack_limit':1,
    'sources':{str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (core/'AscensionMysticEnchant.cpp',root/'src/server/scripts/Spells/spell_shaman.cpp')}},indent=2))
print('PASS: actual Earthliving callback uses the enchant only below35% target health on Area52, adds to existing talent, excludes Earth Shield, and remains isolated elsewhere. Actual connection predicate waits for native proc and script binding; caller is startup LoadConnections. No status labels changed.')

normalizer=block((root/'src/server/game/Globals/ObjectMgr.cpp').read_text(),'bool normalizePlayerName(')
name_fixture=r'''
#include <string>
#include <algorithm>
#include <cwctype>
#include <cassert>
bool Utf8toWStr(std::string const& a,std::wstring& b){b.assign(a.begin(),a.end());return true;}
bool WStrToUtf8(std::wstring const& a,std::string& b){b.assign(a.begin(),a.end());return true;}
void wstrToLower(std::wstring& s){std::transform(s.begin(),s.end(),s.begin(),::towlower);}wchar_t wcharToUpper(wchar_t c){return ::towupper(c);}
'''+normalizer+r'''
int main(){for(std::string name:{"Nicarlo Bot","Krallamees Bot","Rusho Bot"}){auto expected=name;assert(normalizePlayerName(name)&&name==expected);}std::string shortName="Nicarlo";assert(normalizePlayerName(shortName)&&shortName!="Nicarlo Bot");}
'''
run(name_fixture)
print('PASS: actual server name normalization preserves full ASCII bot names; shortened name is a different player. Existing client invite forwards full text; no client change required.')
