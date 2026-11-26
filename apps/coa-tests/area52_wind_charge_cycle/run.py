from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text()
    signatures=('bool Aura::IsProcOnCooldown(', 'void Aura::AddProcCooldown(TimePoint', 'void Aura::AddProcCooldown(SpellProcEntry', 'void Aura::PrepareProcToTrigger(', 'void Aura::ConsumeProcCharges(', 'void Aura::SetCharges(')
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@METHODS@','\n'.join(extract(source,x) for x in signatures))
    unit=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text()
    dispatcher=extract(unit,'void Unit::TriggerAurasProcOnEvent(std::list<AuraApplication*>*')
    assert dispatcher.index('GetProcAurasTriggeredOnEvent(')<dispatcher.index('TriggerAurasProcOnEvent(myProcEventInfo')
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS native Desolate single charge consumption and removal; Calm six-second cooldown reserved before handlers, exact boundary and no charges consumed')
if __name__=='__main__':main()
