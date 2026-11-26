from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/coa/Area52Zephyr.inc').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text()
    code=code.replace('@CONFIG@', 'static bool (*Area52ControlledTyphoonReady)() = nullptr;\n'+extract(source,'void RegisterArea52ControlledTyphoonReadiness(')+'\n@CONFIG@')
    for tag,sig in (('@CONFIG@','void ConfigureArea52Zephyr('),('@CONNECTED@','bool HasArea52WindHandlers('),('@HIT@','void OnSpellHitResult('),('@REMOVE@','void OnAuraRemove(')):
        code=code.replace(tag,extract(source,sig).replace(' override',''))
    aura=(ROOT/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text()
    code=code.replace('@STACK@',extract(aura,'bool Aura::ModStackAmount('))
    config=(ROOT/'src/server/coa/AscensionHeroShaman.cpp').read_text().split('class area52_wind_configuration final',1)[1]
    code=code.replace('@CONFIG_LOAD@',extract(config,'void OnAfterConfigLoad(').replace(' override',''))
    shaman=(ROOT/'src/server/scripts/Spells/spell_shaman.cpp').read_text().split('class spell_sha_windfury_weapon :',1)[1]
    start=shaman.index('        uint32 attacks = 2;');end=shaman.index('\n    }',start)
    code=code.replace('@WINDFURY@',shaman[start:end])
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual Zephyr metadata, readiness, hit grants, once-per-hit guard, Windfury four/two attack dispatch, ten-stack consumption, cleanup and other-mode isolation; native aura cap, refresh and removal math')
if __name__=='__main__':main()
