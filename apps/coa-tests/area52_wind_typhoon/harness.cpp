#include <cassert>
#include <cmath>
using SpellEffIndex=int;
constexpr int SPELL_DRUID_GLYPH_OF_TYPHOON=62135,TRIGGERED_FULL_MASK=1;
struct Unit;
struct SpellInfo{};
struct SpellCastTargets{Unit*unit=nullptr;float x=0,y=0,z=0,o=0;void SetUnitTarget(Unit*u){unit=u;}void SetDst(float a,float b,float c,float e){x=a;y=b;z=c;o=e;}};
struct Unit{bool area=true,enchant=true,glyph=false;int casts=0;float orientation=0;SpellCastTargets last;Unit*ToPlayer(){return this;}bool HasAura(int id){assert(id==977725);return enchant;}bool HasAuraOrTwin(int){return glyph;}float GetOrientation(){return orientation;}float GetPositionZ(){return 3;}void GetNearPoint2D(float&x,float&y,float d,float a){x=10+d*std::cos(a);y=20+d*std::sin(a);}void CastSpell(SpellCastTargets const&t,SpellInfo const*,void*,int){last=t;++casts;}};
namespace AscensionFreePick{bool Applies(Unit*u){return u&&u->area;}}
struct Manager{SpellInfo info;bool present=true;SpellInfo const*GetSpellInfo(int id){assert(id==977726);return present?&info:nullptr;}}manager;Manager*sSpellMgr=&manager;
struct Probe{Unit*caster;int suppressed=0;Unit*GetCaster(){return caster;}void PreventHitDefaultEffect(int){++suppressed;}@KNOCK@ @CONTROLLED@ @AFTER@};
int main(){Unit caster;Probe probe{&caster};for(int hit=0;hit<6;++hit)probe.HandleKnockBack(0);assert(probe.suppressed==6&&caster.casts==0);probe.HandleAfterCast();assert(caster.casts==1&&caster.last.unit==&caster&&caster.last.x>10&&caster.last.y==20);caster.orientation=3.14159265f;probe.HandleAfterCast();assert(caster.casts==2&&caster.last.x<10);caster.enchant=false;probe.HandleKnockBack(0);probe.HandleAfterCast();assert(probe.suppressed==6&&caster.casts==2);caster.glyph=true;probe.HandleKnockBack(0);assert(probe.suppressed==7);caster.enchant=true;caster.glyph=false;caster.area=false;probe.HandleKnockBack(0);probe.HandleAfterCast();assert(probe.suppressed==7&&caster.casts==2);caster.area=true;manager.present=false;probe.HandleAfterCast();assert(caster.casts==2);}
