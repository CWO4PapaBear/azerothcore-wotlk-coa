#include <initializer_list>
#include <cassert>
using int32=int;
struct Player{bool area=true;Player*ToPlayer(){return this;}bool MatchesSpellInteraction(unsigned id,unsigned base){return id==base||(base==403&&id>=997083&&id<=997097);}};
namespace AscensionFreePick{bool Applies(Player*p){return p&&p->area;}}
struct SpellInfo{unsigned Id=403;};
struct Spell{Player*m_originalCaster;SpellInfo*m_spellInfo;int m_damageBeforeTakenMods=0;float m_damageMultipliers[1]={.7f};void capture(int damage){@CAPTURE@}void bounce(){unsigned i=0;@BOUNCE@}};
int main(){Player p;SpellInfo info;Spell s{&p,&info};for(unsigned id:{403u,421u,997083u,997097u}){info.Id=id;s.m_damageBeforeTakenMods=0;s.capture(1000);assert(s.m_damageBeforeTakenMods==1000);s.bounce();assert(s.m_damageBeforeTakenMods==700);}info.Id=123;s.m_damageBeforeTakenMods=400;s.capture(1000);s.bounce();assert(s.m_damageBeforeTakenMods==400);info.Id=421;p.area=false;s.capture(1000);s.bounce();assert(s.m_damageBeforeTakenMods==400);p.area=true;s.m_originalCaster=nullptr;s.bounce();assert(s.m_damageBeforeTakenMods==400);}
