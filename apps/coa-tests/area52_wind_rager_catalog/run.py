from pathlib import Path
import json, subprocess, tempfile
ROOT=Path(__file__).resolve().parents[3]
def extract(s,signature):
    a=s.index(signature);b=s.index('{',a)+1;n=1
    while n:n+=(s[b]=='{')-(s[b]=='}');b+=1
    return s[a:b]
def main():
    header=(ROOT/'src/server/coa/AscensionFreePick.h').read_text()
    rules=(ROOT/'src/server/coa/AscensionFreePickRules.cpp').read_text()
    types='\n'.join(extract(header,'struct '+n+'\n{')+';' for n in ('Entry','Budget','Catalog','Result'))
    funcs='\n'.join(extract(rules,n) for n in ('std::uint32_t PackedUInt32(', 'Result Refuse(', 'Result::operator bool()', 'bool LoadCatalog(', 'Result PlanMasteryRemoval(', 'std::vector<KnownEntry> IncludeMemberMasteries(', 'Result Validate('))
    selections=json.loads(Path(__file__).with_name('selections.json').read_text())
    code=Path(__file__).with_name('harness.cpp').read_text().replace('TYPES',types).replace('FUNCTIONS',funcs).replace('SELECTIONS',','.join('{%d,%d}'%(x['Spell'],x['Level']) for x in selections))
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test'
        subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
if __name__=='__main__':main()
