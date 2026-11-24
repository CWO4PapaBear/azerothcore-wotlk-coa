#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <vector>
using uint32=uint32_t;using int32=int32_t;
constexpr int SPELLFAMILY_GENERIC=0,FORM_BEAR=5,FORM_DIREBEAR=8,FORM_CAT=1,FORM_MOONKIN=31,FORM_TREE=2,SPELL_AURA_DUMMY=4,EFFECT_0=0;
struct Info{int SpellFamilyName=0,SpellIconID=2851;};struct Unit;
struct AuraEffect{int amount=0;Info info;int GetAmount()const{return amount;}Info const* GetSpellInfo()const{return &info;}void Run(Unit* target,bool apply)const;};
struct Unit{bool a52=true;int form=0,casts=0;std::vector<AuraEffect const*> effects;std::map<std::pair<uint32,uint32>,AuraEffect> helpers;Unit* ToPlayer(){return this;}int GetShapeshiftForm(){return form;}uint32 GetGUID(){return 7;}AuraEffect const* GetDummyAuraEffect(int,int,int){return effects.empty()?nullptr:effects.front();}auto const& GetAuraEffectsByType(int){return effects;}void RemoveAurasDueToSpell(uint32 id,uint32 owner){helpers.erase({id,owner});}AuraEffect const* GetAuraEffect(uint32 id,int,uint32 owner){auto it=helpers.find({id,owner});return it==helpers.end()?nullptr:&it->second;}void CastCustomSpell(Unit*,uint32 id,int* amount,void*,void*,bool){helpers[{id,7}].amount=*amount;++casts;}};
namespace AscensionFreePick{bool Applies(Unit* p){return p&&p->a52;}}
void AuraEffect::Run(Unit* target,bool apply)const{@HANDLER@}
void Enter(Unit* target,uint32 helper){switch(helper){@FORMS@}}
int main(){for(auto [form,helper]:std::vector<std::pair<int,uint32>>{{FORM_BEAR,48418},{FORM_DIREBEAR,48418},{FORM_CAT,48420},{FORM_MOONKIN,48421},{FORM_TREE,48422}}){for(int rank:{2,4}){Unit p;p.form=form;AuraEffect talent{rank};p.effects={&talent};talent.Run(&p,true);assert(p.GetAuraEffect(helper,0,7)->amount==rank);int before=p.casts;talent.Run(&p,true);assert(p.casts==before);p.helpers[{helper,8}]={9};p.effects.clear();talent.Run(&p,false);assert(!p.GetAuraEffect(helper,0,7));assert(p.GetAuraEffect(helper,0,8)->amount==9);p.effects={&talent};talent.Run(&p,true);talent.amount=2;talent.Run(&p,true);assert(p.GetAuraEffect(helper,0,7)->amount==2);AuraEffect independent{3};p.effects={&talent,&independent};talent.Run(&p,false);assert(p.GetAuraEffect(helper,0,7)->amount==3);p.form=0;independent.Run(&p,true);assert(!p.GetAuraEffect(helper,0,7));}}
for(auto [form,helper]:std::vector<std::pair<int,uint32>>{{FORM_BEAR,48418},{FORM_CAT,48420},{FORM_MOONKIN,48421},{FORM_TREE,48422}}){
for(int rank:{2,4}){AuraEffect talent{rank},independent{3};Unit before;before.effects={&independent,&talent};talent.Run(&before,true);assert(before.helpers.empty());before.form=form;Enter(&before,helper);assert(before.GetAuraEffect(helper,0,7)->amount==std::max(3,rank));Unit after;after.form=form;Enter(&after,helper);assert(after.helpers.empty());after.effects={&independent,&talent};talent.Run(&after,true);assert(after.GetAuraEffect(helper,0,7)->amount==std::max(3,rank));talent.amount=2;talent.Run(&after,true);assert(after.GetAuraEffect(helper,0,7)->amount==3);after.effects={&talent};independent.Run(&after,false);assert(after.GetAuraEffect(helper,0,7)->amount==2);}}
Unit other;other.form=31;other.a52=false;AuraEffect talent{4};talent.Run(&other,true);assert(other.casts==0);Unit empty;talent.Run(&empty,true);assert(empty.casts==0);
}
