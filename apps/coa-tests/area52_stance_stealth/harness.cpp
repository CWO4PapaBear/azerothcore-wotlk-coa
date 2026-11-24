#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <string_view>
#include <vector>
#include <iostream>
using uint32=uint32_t; using uint64=uint64_t;
enum ShapeshiftForm {FORM_NONE=0,FORM_CAT=1,FORM_BATTLESTANCE=17,FORM_DEFENSIVESTANCE=18,FORM_BERSERKERSTANCE=19,FORM_STEALTH=30};
enum {SPELL_AURA_MOD_SHAPESHIFT=36,CLASS_DRUID=11,CLASS_CONTEXT_ABILITY=1,POWER_MANA=0};
struct Unit;
struct AuraEffect {
 uint32 id; int form; int owner=1; mutable uint32 m_suspendedWarriorStance=0;
 int GetMiscValue()const{return form;} int GetCasterGUID()const{return owner;} uint32 GetId()const{return id;}
 void SuspendWarriorStanceForStealth(Unit*)const; void RestoreWarriorStanceAfterStealth(Unit*)const;
 void DiscardSuspendedWarriorStanceForForm(Unit*,uint32)const;
 void RemoveForm(Unit* target)const;
};
struct Session {bool logout=false; bool PlayerLogout(){return logout;}};
struct Unit {
 bool area=true,alive=true; uint32 clientForm=0; Session session; ShapeshiftForm form=FORM_NONE;
 std::vector<AuraEffect const*> effects; std::set<uint32> learned={2457,71,2458,998110,276634};
 std::map<int,int> bars={{1,998110},{2,276634}}; AuraEffect restored{0,0}; int casts=0,bonuses=0;
 Unit* ToPlayer(){return this;} int GetGUID(){return 1;} bool IsAlive(){return alive;}
 Session* GetSession(){return &session;} bool HasActiveSpell(uint32 s){return learned.contains(s);}
 auto const& GetAuraEffectsByType(int)const{return effects;}
 bool HasShapeshiftAura(){return !effects.empty();} void SetShapeshiftForm(ShapeshiftForm f); void SetByteValue(int,int,uint32 f){clientForm=f;form=ShapeshiftForm(f);}
 bool IsClass(int,int){return false;} void setPowerType(int){} void RemoveAurasByShapeShift(){}
 void RemoveAurasDueToSpell(uint32 spell,int owner){
  auto it=std::find_if(effects.begin(),effects.end(),[&](auto e){return e->id==spell&&e->owner==owner;});
  assert(it!=effects.end());auto e=*it;effects.erase(it);bonuses=0;e->RemoveForm(this);
 }
 void CastSpell(Unit*,uint32 spell,bool triggered){
  assert(triggered);assert(effects.empty());assert(learned.contains(spell));++casts;
  restored={spell,spell==2457?17:spell==71?18:19};effects.push_back(&restored);SetShapeshiftForm(ShapeshiftForm(restored.form));bonuses=1;
 }
};
constexpr int UNIT_FIELD_BYTES_2=1;
struct ScriptMgr{void OnUnitSetShapeshiftForm(Unit*,ShapeshiftForm){}} scripts;auto sScriptMgr=&scripts;
SET_FORM
using Player=Unit;
namespace AscensionFreePick{bool Applies(Player* p){return p&&p->area;}}
SUSPEND
RESTORE
DISCARD
void AuraEffect::RemoveForm(Unit* target)const{REMOVAL}
enum SpellCastResult {SPELL_CAST_OK,SPELL_FAILED_NOT_SHAPESHIFT,SPELL_FAILED_ONLY_SHAPESHIFT};
enum {SPELL_EFFECT_HEAL=10,SPELL_EFFECT_HEAL_PCT=136,SPELL_EFFECT_HEAL_MAX_HEALTH=67,SPELL_AURA_PERIODIC_HEAL=8,SPELL_EFFECT_LEARN_SPELL=36,LOCALE_enUS=0};
constexpr uint32 SPELL_ATTR0_NOT_SHAPESHIFTED=0x10000,SPELL_ATTR2_ALLOW_WHILE_NOT_SHAPESHIFTED=0x80000;
constexpr uint32 SHAPESHIFT_FLAG_STANCE=0x1, SHAPESHIFT_FLAG_CAN_ONLY_CAST_SHAPESHIFT_SPELLS=0x400;
struct SpellShapeshiftFormEntry{uint32 flags1;};
struct Store {std::map<uint32,SpellShapeshiftFormEntry> forms;auto LookupEntry(uint32 f)const{auto it=forms.find(f);return it==forms.end()?nullptr:&it->second;}} sSpellShapeshiftFormStore;
int GetTalentSpellCost(uint32){return 0;}
#define LOG_ERROR(...) ((void)0)
struct SpellInfo {
 uint32 Id,Attributes,AttributesEx2;uint64 Stances,StancesNot;const char* SpellName[1]={"Shot"};
 struct Effect{int Effect=0;} Effects[3];
 bool HasEffect(int)const{return false;} bool HasAura(int)const{return false;} bool IsPassive()const{return false;}
 bool HasAttribute(uint32 v)const{return Attributes&v;} SpellCastResult CheckShapeshift(uint32 form)const;
};
CHECK_FORM
void checkShot(uint32 id,uint64 allowed,uint64 denied,uint32 flags,uint32 flags2){
 SpellInfo s{id,flags,flags2,allowed,denied};
 for(auto f:{FORM_BATTLESTANCE,FORM_DEFENSIVESTANCE,FORM_BERSERKERSTANCE,FORM_NONE})assert(s.CheckShapeshift(f)==SPELL_CAST_OK);

}
void enter(Unit& u,AuraEffect& stealth){u.effects.push_back(&stealth);stealth.SuspendWarriorStanceForStealth(&u);u.SetShapeshiftForm(FORM_STEALTH);}
void leave(Unit& u,AuraEffect& stealth){std::erase(u.effects,&stealth);stealth.RemoveForm(&u);stealth.RestoreWarriorStanceAfterStealth(&u);}
int main(){
 FORM_CASES
 SHOT_CASES
 for(uint32 id:{2457,71,2458}){
  Unit u;u.CastSpell(&u,id,true);auto learned=u.learned;auto bars=u.bars;auto original=u.form;
  for(int n=0;n<3;++n){AuraEffect stealth{1784,30};enter(u,stealth);assert(u.effects.size()==1&&u.bonuses==0&&u.form==FORM_STEALTH);assert(stealth.m_suspendedWarriorStance==id);
   stealth.SuspendWarriorStanceForStealth(&u);assert(stealth.m_suspendedWarriorStance==id);
   leave(u,stealth);assert(u.form==original&&u.clientForm==uint32(original)&&u.effects.size()==1&&u.bonuses==1);assert(u.learned==learned&&u.bars==bars);assert(!stealth.m_suspendedWarriorStance);
  }
  assert(u.casts==4);
  for(int condition=0;condition<4;++condition){Unit v;v.CastSpell(&v,id,true);AuraEffect st{1784,30},cat{768,1};enter(v,st);
   if(condition==0)v.alive=false; if(condition==1)v.session.logout=true; if(condition==2)v.learned.erase(id);if(condition==3){v.effects.push_back(&cat);v.form=FORM_CAT;}
   leave(v,st);assert(v.casts==1);assert(!st.m_suspendedWarriorStance);
  }
  Unit v;v.CastSpell(&v,id,true);AuraEffect a{1784,30},b{1785,30};enter(v,a);enter(v,b);leave(v,a);assert(v.casts==1&&b.m_suspendedWarriorStance==id);leave(v,b);assert(v.casts==2&&v.restored.id==id);
 }
 Unit none;AuraEffect stealth{1784,30};enter(none,stealth);leave(none,stealth);assert(none.casts==0&&none.form==FORM_NONE);
 for(bool foreign:{false,true}){Unit u;u.CastSpell(&u,2457,true);u.area=foreign;AuraEffect st{1784,30,foreign?2:1};enter(u,st);assert(u.effects.size()==2&&u.bonuses==1&&!st.m_suspendedWarriorStance);}

 for(uint32 old:{2457,71,2458}) for(uint32 selected:{2457,71,2458}){
  Unit u;u.CastSpell(&u,old,true);AuraEffect st{1784,30};enter(u,st);
  st.DiscardSuspendedWarriorStanceForForm(&u,selected==2457?17:selected==71?18:19);
  leave(u,st);assert(u.casts==1&&u.form==FORM_NONE);
  u.CastSpell(&u,selected,true);assert(u.casts==2&&u.restored.id==selected);
 }
 for(uint32 form:{0u,30u}){
  Unit u;u.CastSpell(&u,2458,true);AuraEffect st{1784,30};enter(u,st);
  st.DiscardSuspendedWarriorStanceForForm(&u,form);leave(u,st);assert(u.casts==2&&u.restored.id==2458);
 }
 {Unit u;u.CastSpell(&u,2458,true);AuraEffect st{1784,30};enter(u,st);
 st.DiscardSuspendedWarriorStanceForForm(&u,FORM_CAT);leave(u,st);assert(u.casts==1);}


 std::cout<<"Native stance and cast-gate checks passed\n";
}
