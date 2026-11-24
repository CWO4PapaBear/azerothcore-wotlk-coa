from pathlib import Path
import subprocess,tempfile,re
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    startup=(ROOT/'src/server/coa/Area52LegendaryBatch.inc').read_text()
    grid=(ROOT/'src/server/game/Grids/Notifiers/GridNotifiers.h').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text().replace('@HOOK@',extract(startup,'void OnLoadSpellCustomAttr(').replace(' override','')).replace('@GRID@',extract(grid,'class AnyGroupedUnitInObjectRangeCheck')+';')
    mgr=(ROOT/'src/server/game/Spells/SpellMgr.cpp').read_text()
    init=extract(mgr,'bool InitTriggerAuraData()\n')
    generate=extract(mgr[mgr.index('void SpellMgr::LoadSpellProcs()'):],'for (SpellInfo const* spellInfo : mSpellInfoMap)')
    definitions=(ROOT/'src/server/game/Spells/Auras/SpellAuraDefines.h').read_text()+(ROOT/'src/server/game/Spells/SpellMgr.h').read_text()
    constants=dict(re.findall(r'\b([A-Z][A-Z_0-9]+)\s*=\s*(0x[0-9a-fA-F]+|[0-9]+)\b',definitions))
    for name,expr in re.findall(r'\b([A-Z][A-Z_0-9]+)\s*=\s*([A-Z][A-Z_0-9]+(?:[ \t]*\|[ \t]*[A-Z][A-Z_0-9]+)+)',definitions):
        parts=[part.strip() for part in expr.split('|')]
        if all(part in constants for part in parts):
            value=0
            for part in parts:value|=int(constants[part],0)
            constants[name]=str(value)
    names=set(re.findall(r'\b(?:SPELL_AURA|PROC_)[A-Z_0-9]+\b',init+generate))
    existing=set(re.findall(r'\b([A-Z][A-Z_0-9]+)\s*=',code))
    declarations=''.join('constexpr int '+name+'='+constants[name]+';\n' for name in sorted(names-existing))
    code=code.replace('@PROC_CONSTANTS@',declarations).replace('@INIT_PROCS@',init).replace('@GENERATE_PROCS@',generate)
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
    print('PASS actual startup hook with configuration available and catalog uninitialized: Earth/Moon rank3 helper, Totem of Wrath raid effects; repeated application and other modes; native grouped target predicate')
if __name__=='__main__':main()
