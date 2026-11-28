from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[3]
a=(r/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text();start=a.index('    SpellProcEntry consecratedSealProc;');end=a.index('    // check spell triggering us',start);guard=a[start:end]
s=Path(__file__).with_name('harness.cpp').read_text().split('@PRODUCTION@')[0].replace('@PPM@','')
s=s.replace('bool area=true,weaponAvailable=true;', 'bool area=true,weaponAvailable=true,auraPresent=true,granted=true;bool HasAura(uint32,uint32){return auraPresent;}')
s=s.replace('struct Spell{Player*caster;', 'struct Spell{Item*m_CastItem=nullptr;SpellInfo const*trigger=nullptr;SpellInfo const*GetTriggeredByAuraSpellInfo()const{return trigger;}Player*caster;')
s+=r"""
constexpr int DOT=2,PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS=16,PROC_ATTR_TRIGGERED_CAN_PROC=1;
struct flag96{uint32 a=0,b=0,c=0;};struct SpellProcEntry{flag96 SpellFamilyMask{0,1474560,0};uint32 ProcFlags=4,AttributesMask=0;};
namespace AscensionFreePick{bool Applies(Player*p){return Enabled&&p&&p->area;}}namespace AscensionMysticEnchant{bool GrantsActive(Player*p,uint32 id){return p->granted&&id==965295;}}
struct DamageInfo{uint32 damage=100;int type=1;uint32 GetDamage()const{return damage;}int GetDamageType()const{return type;}};
struct ProcEventInfo{Unit*actor;SpellInfo const*info;Spell const*cast;DamageInfo const*damage;Unit*GetActor(){return actor;}SpellInfo const*GetSpellInfo(){return info;}Spell const*GetProcSpell(){return cast;}DamageInfo const*GetDamageInfo(){return damage;}};
struct AuraApplication{Unit*unit;Unit*GetTarget(){return unit;}};
struct Seal {uint32 id=20375;uint32 GetId()const{return id;}SpellProcEntry Select(AuraApplication*aurApp,ProcEventInfo&eventInfo,SpellProcEntry const*procEntry)const{
@GUARD@
return *procEntry;}};
int main(){Enabled=true;Player p;AuraApplication app{&p};SpellInfo info{977827,2},trigger{977848,0};DamageInfo damage;Spell cast;cast.caster=&p;cast.m_CastItem=&p.weapon;cast.trigger=&trigger;ProcEventInfo event{&p,&info,&cast,&damage};Seal seal;SpellProcEntry original;auto selected=seal.Select(&app,event,&original);assert(selected.ProcFlags==16&&selected.AttributesMask==1&&selected.SpellFamilyMask.b==0);assert(original.ProcFlags==4&&original.AttributesMask==0&&original.SpellFamilyMask.b==1474560);
for(int x=0;x<14;++x){p.area=true;p.auraPresent=true;p.granted=true;p.weaponAvailable=true;p.weapon.enchant=45;seal.id=20375;info.Id=977827;trigger.Id=977848;damage.damage=100;damage.type=1;event.actor=&p;event.info=&info;event.cast=&cast;event.damage=&damage;cast.m_CastItem=&p.weapon;cast.trigger=&trigger;Item other;
if(x==0)p.area=false;if(x==1)event.actor=nullptr;if(x==2)info.Id=977828;if(x==3)event.cast=nullptr;if(x==4)cast.m_CastItem=nullptr;if(x==5)trigger.Id=42;if(x==6)event.damage=nullptr;if(x==7)damage.damage=0;if(x==8)damage.type=DOT;if(x==9)p.auraPresent=false;if(x==10)p.granted=false;if(x==11)p.weapon.enchant=1;if(x==12)cast.m_CastItem=&other;if(x==13)seal.id=20376;
auto rejected=seal.Select(&app,event,&original);assert(rejected.ProcFlags==4&&rejected.AttributesMask==0&&rejected.SpellFamilyMask.b==1474560);}
}
"""
s=s.replace('@GUARD@',guard)
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(s);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS extracted narrow Seal override: exact actor/payload/source/item/equipment/ownership/direct-damage guards and immutable baseline proc entry. Full native proc dispatch qualified separately.')
