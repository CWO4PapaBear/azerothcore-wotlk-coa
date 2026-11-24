from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    s=(ROOT/'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text()
    block=extract(s[s.index('void AuraEffect::HandleAuraDummy('):],'if (GetSpellInfo()->SpellFamilyName == SPELLFAMILY_GENERIC && GetSpellInfo()->SpellIconID == 2851 &&')
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@HANDLER@',block)
    forms=[]
    for helper in (48418,48420,48421,48422):
        end=s.index('target->CastCustomSpell(target, '+str(helper)+', &bp, nullptr, nullptr, true);')
        start=s.rfind('if (AuraEffect const* aurEff = target->GetDummyAuraEffect(',0,end)
        forms.append('case '+str(helper)+':'+extract(s[start:],'if (AuraEffect const* aurEff')+'break;')
    code=code.replace('@FORMS@',''.join(forms))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual Master Shapeshifter lifecycle block: 2/4 percent, current forms, repeated sync, downgrade/removal, independent grants and foreign helper preservation')
if __name__=='__main__':main()
