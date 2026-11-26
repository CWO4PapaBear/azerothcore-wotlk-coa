#include <cassert>
#include <map>
#include <vector>
#include <utility>
using int32=int;constexpr int SPELL_LINK_AURA=2000000,IMMUNITY_ID=1,AURA_REMOVE_BY_DEATH=5;
struct Aura{int stack=1;int GetStackAmount(){return stack;}void ModStackAmount(int n){stack+=n;}};
struct Unit{int guid=7;std::map<std::pair<int,int>,Aura>auras;void ApplySpellImmune(int,int,int,bool){assert(false);}void AddAura(int id,Unit*t){t->auras[{id,guid}]={};}void RemoveAurasDueToSpell(int){assert(false);}void CastSpell(Unit*,int,bool,void*,void*,int){assert(false);}void RemoveAura(int id,int owner,int,int){auras.erase({id,owner});}Aura*GetAura(int id,int owner){auto i=auras.find({id,owner});return i==auras.end()?nullptr:&i->second;}};
struct Manager{std::map<int,std::vector<int>>links;std::vector<int>const*GetSpellLinked(int key){auto i=links.find(key);return i==links.end()?nullptr:&i->second;}}manager;Manager*sSpellMgr=&manager;
struct Probe{int id;int GetId(){return id;}int GetCasterGUID(){return 7;}int GetStackAmount(){return 1;}void apply(Unit*target,Unit*caster,bool apply,bool onReapply,int removeMode){@LINKS@}};
int main(){for(auto pair:{std::pair<int,int>{19263,67801},{51690,61851}}){manager.links[pair.first+SPELL_LINK_AURA]={pair.second};for(int mode=0;mode<7;++mode){Unit p;p.auras[{pair.second,8}]={};Probe probe{pair.first};probe.apply(&p,&p,true,false,0);assert(p.GetAura(pair.second,7));probe.apply(&p,&p,true,true,0);assert(p.auras.size()==2);probe.apply(&p,&p,false,false,mode);assert(!p.GetAura(pair.second,7)&&p.GetAura(pair.second,8));}}}
