from pathlib import Path
import tempfile,subprocess,struct
r=Path(__file__).resolve().parents[3]
def method(path,sig):
 s=(r/path).read_text();a=s.index(sig);b=s.index('{',a)+1;n=1
 while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
code=r"""
#include "Area52DragonReviewRules.h"
#include <cassert>
#include <algorithm>
#include <set>
#include <initializer_list>
using uint32=unsigned;using int32=int;using SpellEffIndex=int;constexpr int EFFECT_0=0,EFFECT_1=1,SPELL_WARRIOR_BLOODTHIRST=23885,SPELL_WARRIOR_BLOODTHIRST_DAMAGE=23881,SPELL_WARRIOR_GLYPH_OF_BLOODTHIRST=58367,NODAMAGE=9,SPELL_SCHOOL_MASK_NORMAL=1;
struct AuraEffect{int amount=1,ticks=5;int GetAmount()const{return amount;}int GetTotalTicks()const{return ticks;}};
struct Player{bool realm=true;unsigned max=10000,now=0,until=0;bool HasSpellCooldown(unsigned){return now<until;}void AddSpellCooldown(unsigned id,unsigned,unsigned ms){assert(id==81101);until=now+ms;}Player*ToPlayer(){return this;}unsigned GetMaxHealth(){return max;}AuraEffect*buff=nullptr,*glyph=nullptr;AuraEffect*GetAuraEffect(unsigned,int){return buff;}AuraEffect*GetAuraEffectOfRankedSpellOrTwin(unsigned,int){return glyph;}int CountPctFromMaxHealth(int pct){return max*pct/100;}};
using Unit=Player;
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->realm;}}
struct Effect{int CalcValue(Player*)const{return 1;}};struct SpellInfo{Effect Effects[3];};struct Manager{SpellInfo info;SpellInfo*GetSpellInfo(unsigned){return &info;}}mgr;Manager*sSpellMgr=&mgr;
int AddPct(int value,int pct){return value+value*pct/100;}
struct Heal{Player*p;int value=0;Player*GetCaster(){return p;}void SetEffectValue(int v){value=v;}int GetEffectValue(){return value;}@@HEAL@@};
struct Skull{Player*p;bool prevented=false;Player*GetCaster(){return p;}void PreventHitDefaultEffect(int index){assert(index==1);prevented=true;}@@SKULL@@};
@@BLOODBATH@@
int main(){Player p;Heal h{&p};h.HandleHeal(0);assert(h.value==100);AuraEffect buff;buff.amount=2;p.buff=&buff;h.HandleHeal(0);assert(h.value==200);AuraEffect glyph;glyph.amount=100;p.glyph=&glyph;h.HandleHeal(0);assert(h.value==400);p.max=20000;h.HandleHeal(0);assert(h.value==800);
Skull s{&p};s.LimitCooldown(1);assert(!s.prevented&&p.until==3000);p.now=2999;s.LimitCooldown(1);assert(s.prevented&&p.until==3000);p.now=3000;s.prevented=false;s.LimitCooldown(1);assert(!s.prevented&&p.until==6000);p.realm=false;p.now=6000;s.LimitCooldown(1);assert(p.until==6000);
for(int ap:{0,1000,5000}){AuraEffect effect;int total=50+int(ap*0.026),amount=total;bool recalc=true;SpreadDamage(&effect,amount,recalc);assert(amount==total/5&&!recalc);}AuraEffect zero;zero.ticks=0;int amount=100;bool recalc=true;SpreadDamage(&zero,amount,recalc);assert(amount==100&&!recalc);
for(unsigned hp:{1,99,100,9999,10000})assert(Area52DragonReview::BurningRushDrain(hp)==hp*5/100);
}
""".replace('@@HEAL@@',method('src/server/scripts/Spells/spell_warrior.cpp','void HandleHeal(SpellEffIndex /*effIndex*/)')).replace('@@SKULL@@',method('src/server/coa/Area52DragonWarrior.inc','void LimitCooldown(')).replace('@@BLOODBATH@@',method('src/server/coa/AscensionWildcardBloodbath.cpp','void SpreadDamage('))
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'t.cpp').write_text(code);subprocess.run(['c++','-std=c++20','-I'+str(r/'src/server/coa'),str(p/'t.cpp'),'-o',str(p/'t')],check=True);subprocess.run([str(p/'t')],check=True)
def rows(name):
 b=(Path('/data/dbc')/(name+'.dbc')).read_bytes();_,n,f,z,_=struct.unpack_from('<4s4I',b);return {struct.unpack_from('<I',b,20+i*z)[0]:struct.unpack_from('<'+str(f)+'I',b,20+i*z) for i in range(n)}
s=rows('Spell');duration=rows('SpellDuration')
assert s[23885][36]==1 and s[23885][116]==23880
assert s[954815][98]==1000 and duration[s[954815][40]][1]==5000
assert s[954814][1]==s[13877][1]==s[12328][1] and s[954814][1]!=0
assert s[954814][30]==60000
assert s[179][49]==3 and s[179][80]+1==1 and s[179][111]==12292
assert duration[s[954806][40]][1]==6000 and s[954806][100]==2000
print('PASS production Bloodthirst heal/glyph/max-health calculation, one-charge data; Bloodbath total-to-tick division and shared cooldown category; Skull Crusher 3-second exact boundary and3-stack data; Burning Rush drain arithmetic and duration/cadence. Generic persistence and dispatcher covered separately.')
