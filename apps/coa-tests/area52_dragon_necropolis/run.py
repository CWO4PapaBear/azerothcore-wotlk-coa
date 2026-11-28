from pathlib import Path
import struct,subprocess,tempfile
r=Path(__file__).resolve().parents[3]
def method(s,sig):
 a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
warrior=(r/'src/server/coa/AscensionHeroWarrior.cpp').read_text();native=(r/'src/server/game/Entities/Player/Player.cpp').read_text()
raw=Path('/data/dbc/Spell.dbc').read_bytes();_,count,fields,size,strings=struct.unpack_from('<4s4I',raw)
records={struct.unpack_from('<I',raw,20+i*size)[0]:struct.unpack_from('<'+str(fields)+'I',raw,20+i*size) for i in range(count)}
for parent,helper in [(49189,52284),(50149,52285),(50150,52286)]:
 a=records[parent];assert not (a[4]&64) and a[71:74]==(3,36,0) and helper in a[116:119]
 assert records[helper][71]==6 and records[helper][95]==69
rejection=method(native[native.index('// xinef: do not add spells with effect learn spell'):],'if (spellInfo->HasEffect(SPELL_EFFECT_LEARN_SPELL))').replace('LOG_INFO("entities.player", "TRYING TO LEARN SPELL WITH EFFECT LEARN 2: {}, PLAYER: {}", spellId, GetGUID().ToString());','')
code=r"""
#include <array>
#include <set>
#include <map>
#include <cassert>
#include <string>
#include <vector>
#include <type_traits>
using uint32=unsigned;constexpr int SPELL_ATTR0_PASSIVE=64,GLOBALHOOK_ON_LOAD_SPELL_CUSTOM_ATTR=1,EFFECT_0=0,EFFECT_1=1,EFFECT_2=2,SPELL_EFFECT_APPLY_AURA=6,SPELL_EFFECT_NONE=0,SPELL_AURA_DUMMY=4,TARGET_UNIT_CASTER=1,SPELL_EFFECT_LEARN_SPELL=36;constexpr unsigned PROC_FLAG_DONE_MELEE_AUTO_ATTACK=4;
int SpellImplicitTargetInfo(int n){return n;}
struct Effect{int Effect=0,ApplyAuraName=0,TargetA=0,BasePoints=34,DieSides=1;};
struct SpellInfo{unsigned Id,ProcFlags=0,Attributes=400;::Effect Effects[3];bool HasEffect(int n){for(auto&e:Effects)if(e.Effect==n)return true;return false;}};
struct GlobalScript{GlobalScript(char const*,std::vector<int>){}virtual void OnLoadSpellCustomAttr(SpellInfo*){}};
struct Config{bool enabled=true;std::string model="hero";template<class T>T GetOption(char const*k,T){if constexpr(std::is_same_v<T,bool>)return enabled;else if constexpr(std::is_same_v<T,unsigned>)return 0x400;else return std::string(k)=="CoA.ClassModel"?model:"live";}}cfg;auto*sConfigMgr=&cfg;
@@META@@
struct Aura{};struct Session{bool logout=false;bool PlayerLogout(){return logout;}};
struct Player{bool realm=true,world=true,alive=true;Session session;std::set<unsigned>spells,authorized={49189,50149,50150};std::map<unsigned,Aura>auras;unsigned casts=0;
 bool IsInWorld(){return world;}bool IsAlive(){return alive;}Session*GetSession(){return &session;}unsigned GetGUID(){return 1;}
 bool HasAura(unsigned id){return auras.contains(id);}bool HasSpell(unsigned id){return spells.contains(id);}Aura*GetAura(unsigned id,unsigned){return HasAura(id)?&auras[id]:nullptr;}
 void RemoveAurasDueToSpell(unsigned id,unsigned){auras.erase(id);}void CastSpell(Player*,unsigned id,bool){auras[id]={};++casts;}
};namespace AscensionFreePick{bool Applies(Player*p){return p->realm;}bool CanLearnSpell(Player*p,unsigned id,bool,bool){return p->authorized.contains(id);}}
@@REFRESH@@
bool accepts(SpellInfo*spellInfo){std::set<unsigned>m_spells{spellInfo->Id};@@REJECT@@ return true;}
int main(){area52_dragon_review_metadata metadata;std::array<unsigned,3>parents={49189,50149,50150},helpers={52284,52285,52286};
for(unsigned i=0;i<3;++i){SpellInfo old{parents[i]};old.Effects[0].Effect=3;old.Effects[1].Effect=36;assert(!accepts(&old));auto repaired=old;metadata.OnLoadSpellCustomAttr(&repaired);assert(accepts(&repaired)&&bool(repaired.Attributes&64));assert(repaired.Effects[0].Effect==6&&repaired.Effects[0].ApplyAuraName==4&&repaired.Effects[0].BasePoints+repaired.Effects[0].DieSides==35);cfg.enabled=false;metadata.OnLoadSpellCustomAttr(&old);assert(!accepts(&old));cfg.enabled=true;
 for(unsigned order=0;order<2;++order){Player p;if(order)RefreshArea52Necropolis(&p);assert(p.auras.empty());p.auras[parents[i]]={};RefreshArea52Necropolis(&p);assert(p.HasAura(helpers[i])&&p.casts==1);RefreshArea52Necropolis(&p);assert(p.casts==1);p.auras.erase(helpers[i]);RefreshArea52Necropolis(&p);assert(p.HasAura(helpers[i]));
 for(unsigned rank=0;rank<3;++rank){p.auras.clear();for(auto h:helpers)p.auras[h]={};p.auras[parents[rank]]={};RefreshArea52Necropolis(&p);for(unsigned j=0;j<3;++j)assert(p.HasAura(helpers[j])==(j==rank));}
 p.auras.clear();for(auto h:helpers)p.auras[h]={};RefreshArea52Necropolis(&p);assert(p.auras.empty());p.spells.insert(helpers[i]);p.auras[helpers[i]]={};RefreshArea52Necropolis(&p);assert(p.HasAura(helpers[i]));}
}
Player stale;stale.auras[49189]={};stale.auras[50150]={};stale.authorized={49189};RefreshArea52Necropolis(&stale);assert(stale.HasAura(52284)&&!stale.HasAura(52286));stale.authorized.clear();RefreshArea52Necropolis(&stale);assert(!stale.HasAura(52284));Player other;other.realm=false;other.auras[52286]={};RefreshArea52Necropolis(&other);assert(other.HasAura(52286));Player logout;logout.session.logout=true;logout.auras[52286]={};RefreshArea52Necropolis(&logout);assert(logout.HasAura(52286));Player dead;dead.alive=false;dead.auras[50150]={};RefreshArea52Necropolis(&dead);assert(!dead.HasAura(52286));dead.alive=true;RefreshArea52Necropolis(&dead);assert(dead.HasAura(52286));}
""".replace('@@META@@',warrior[warrior.index('class area52_dragon_review_metadata'):warrior.index('class aura_area52_burning_rush')]).replace('@@REFRESH@@',method(warrior,'void RefreshArea52Necropolis(')).replace('@@REJECT@@',rejection)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'t.cpp').write_text(code);subprocess.run(['c++','-std=c++20',str(p/'t.cpp'),'-o',str(p/'t')],check=True);subprocess.run([str(p/'t')],check=True)
print('PASS original native learn-effect rejection reproduced from real DBC; candidate metadata, all3 helper ranks, missing aura restoration, both parent/helper orders, repeats, reduction, removal, independent ownership, logout/death/other realm. Parent saved-selection authorization covered by dragon_talent_lifecycle; no gameplay.')
