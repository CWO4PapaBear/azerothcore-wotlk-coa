from pathlib import Path
import subprocess,tempfile,struct
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    source=(ROOT/'src/server/coa/Area52Desolate.inc').read_text()
    shaman=(ROOT/'src/server/scripts/Spells/spell_shaman.cpp').read_text()
    functions=[extract(source,x) for x in ('bool Area52DesolateEnabled(', 'void ConfigureArea52Desolate(', 'void RegisterArea52WindfuryReadiness(', 'void ApplyArea52DesolateWindfury(', 'bool HasArea52DesolateHandlers(')]
    functions.insert(0,extract(shaman,'bool HasArea52WindfuryHandler('))
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@FUNCTIONS@','\n'.join(functions))
    native=(ROOT/'src/server/coa/AscensionMysticEnchant.cpp').read_text()
    connection=extract(native,'        if (spell == 979769 || spell == 977725 || spell == 978232 || spell == 997082)')
    startup=extract(native[native.index('class MysticWorld'):],'void OnStartup(').replace(' override','')
    glue='std::map<unsigned,unsigned> Connected;bool HasArea52WindHandlers(unsigned id){return id==997082&&HasArea52DesolateHandlers();}void RefreshTestAccess(){}void LoadConnections(){Connected.clear();for(unsigned spell:{997082u}){'+connection+'}}'+startup
    code=code.replace('@CONNECTIONS@',glue)
    world=(ROOT/'src/server/game/World/World.cpp').read_text()
    main=(ROOT/'src/server/apps/worldserver/Main.cpp').read_text()
    assert world.index('LoadSpellInfoStore()')<world.index('LoadSpellProcs()')
    assert main.index('sWorld->SetInitialWorldSettings()')<main.index('sScriptMgr->OnStartup()')
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print('PASS actual MysticWorld OnStartup refresh and scoped LoadConnections branch: initial unready, missing bindings/procs, ready refresh, repeated startup and disabled realm; real caller initialization order retained')
if __name__=='__main__':main()
