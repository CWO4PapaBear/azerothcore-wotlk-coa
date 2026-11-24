from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    aura=(ROOT/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text()
    druid=(ROOT/'src/server/coa/AscensionHeroDruid.cpp').read_text();druid=druid[druid.index('class aura_hero_druid_vengeful_soul'):]
    mage=(ROOT/'src/server/scripts/Spells/spell_mage.cpp').read_text();mage=mage[mage.index('class spell_mage_mirror_image'):]
    ai=(ROOT/'src/server/scripts/Pet/pet_mage.cpp').read_text();ai=ai[ai.index('struct npc_pet_mage_mirror_image'):]
    code=Path(__file__).with_name('harness.cpp').read_text()
    for marker,source,signature in [('@CHECK@',aura,'bool Aura::IsProcOnCooldown('),('@ADD@',aura,'void Aura::AddProcCooldown(SpellProcEntry'),('@REMOVE@',druid,'void AfterRemove('),('@APPLY@',mage,'void HandleEffectApply('),('@PERIODIC@',mage,'void CalcPeriodic('),('@AI@',ai,'void UpdateAI(')]:
        code=code.replace(marker,extract(source,signature).replace(' override',''))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
    print('PASS actual six-second proc cooldown including GM bypass; Vengeful Soul expiry and clamping; Mirror Image single initialization and delayed/CC-safe spell AI')
if __name__=='__main__':main()
