from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b].replace(' override','')
def main():
    s=(ROOT/'src/server/coa/AscensionHeroShaman.cpp').read_text()
    functions='\n'.join(extract(s,x) for x in ('bool Area52DualWieldReady(', 'bool Area52DualWieldBonus(', 'void RefreshArea52DualWield(', 'void OnAfterAuraEffectCalculateAmount(', 'void ConfigureArea52DualWield('))
    check=extract(s.split('class aura_hero_shaman_dual_wield_specialization',1)[1],'bool CheckProc(')
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@FUNCTIONS@',functions).replace('@CHECK@',check).replace('@REMOVE@',extract(s.split('class area52_dual_wield_cleanup',1)[1],'void OnAuraRemove('))
    code=code.replace('@DURABILITY@',extract((ROOT/'src/server/game/Entities/Player/Player.cpp').read_text(),'void Player::DurabilityPointsLoss('))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS production dual-wield amount, refresh, metadata and proc eligibility: ranks, copies, equipment, feral, owner load, cleanup and other modes')
if __name__=='__main__':main()
