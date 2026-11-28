from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[3]
def extract(s,m):
 a=s.index(m);i=s.index('{',a)+1;n=1
 while n:n+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]
u=(r/'src/server/game/Entities/Unit/Unit.cpp').read_text();a=(r/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text();e=(r/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text()
s=(r/'apps/coa-tests/area52_crusader_modifiers/stats.cpp').read_text();s=s[:s.index('int main()')].replace('@HANDLER@','').replace('@MULTIPLIER@',extract(u,'float Unit::GetTotalAuraMultiplier(AuraType auraType, std::function'))
s=s.replace('struct SpellInfo{','struct SpellInfo{int StackAmount=4;int CalcMaxAuraStacks(Unit*)const{return StackAmount;}')
s=s.replace('void HandleModTotalPercentStat(', 'void CalculatePeriodic(Unit*,bool,bool){}void CalculatePeriodicData(){}\nvoid HandleModTotalPercentStat(')
s+=r"""
using AuraRemoveMode=int;constexpr int SPELL_ATTR1_AURA_UNIQUE=1,MAX_SPELL_EFFECTS=3;
struct Aura{SpellInfo info;SpellInfo const*m_spellInfo=&info;int m_stackAmount=0,m_maxDuration=6000,duration=0;bool removed=false;Unit caster;AuraEffect effect;AuraEffect*m_effects[3]={&effect,nullptr,nullptr};
Unit*GetCaster(){return &caster;}int GetStackAmount(){return m_stackAmount;}void Remove(int){removed=true;m_stackAmount=0;effect.amount=0;}void RefreshSpellMods(){}int CalcMaxDuration(){return 6000;}void RefreshDuration(){duration=m_maxDuration;}int CalcMaxCharges(){return 0;}void SetCharges(int){}void SetNeedClientUpdateForTargets(){}
void SetStackAmount(int value){m_stackAmount=value;int amount=2;@AMOUNT@effect.amount=amount;}
Aura*GetBase(){return this;}void RefreshTimers(bool);bool ModStackAmount(int32,AuraRemoveMode,bool=false);};
"""
s=s.replace('@AMOUNT@','amount *= GetBase()->GetStackAmount();' if 'amount *= GetBase()->GetStackAmount();' in e else '')
assert 'amount *= GetBase()->GetStackAmount();' in s
s+=extract(a,'void Aura::RefreshTimers(bool')+'\n'+extract(a,'bool Aura::ModStackAmount(')
s+=r"""
int main(){for(uint32 mask:{1u,2u,4u,8u,32u,64u}){Aura buff;buff.effect.id=286100;buff.effect.type=79;buff.effect.misc=mask;Unit unit;unit.auras[79]={&buff.effect};
for(int i=1;i<=6;++i){buff.duration=100;assert(!buff.ModStackAmount(1,0));assert(buff.GetStackAmount()==std::min(i,4));assert(buff.duration==6000&&buff.m_maxDuration==6000);assert(buff.effect.amount==2*std::min(i,4));for(uint32 school:{1u,2u,3u,4u,5u,8u,9u,16u,32u,33u,64u,65u})assert(close(unit.GetTotalAuraMultiplier(79,[school](AuraEffect const*e){return e->GetMiscValue()&school;}),school&mask?1+.02*std::min(i,4):1));}
buff.duration=100;assert(!buff.ModStackAmount(-1,0));assert(buff.GetStackAmount()==3&&buff.duration==100&&buff.effect.amount==6);assert(buff.ModStackAmount(-3,0)&&buff.removed);unit.auras[79].clear();assert(unit.GetTotalAuraMultiplier(79,[](AuraEffect const*){return true;})==1);}}
"""
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(s);subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS native Aura::ModStackAmount/RefreshTimers and Unit::GetTotalAuraMultiplier: cap4, refresh6seconds, amount2percent perstack and school matching; no full AuraEffect recalculation claim')
