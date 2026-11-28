from pathlib import Path
import sys,subprocess,tempfile
r=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(r/'apps/coa-tests'))
from coa_talent_catalog import STUBS
with tempfile.TemporaryDirectory() as directory:
 p=Path(directory)
 for name,text in STUBS.items():(p/name).write_text(text)
 subprocess.run(['g++','-std=c++20','-I'+str(p),'-I'+str(r/'src/server/coa'),'-I'+str(r/'src/server/shared/DataStores'),'-I'+str(r/'src/common'),str(Path(__file__).with_name('fixture.cpp')),str(r/'src/server/coa/AscensionFreePickRules.cpp'),str(r/'src/server/shared/DataStores/ClientDBC.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS PROPOSED Crusader ascending-rank plan (not saved/applied) native selection/budget/prerequisite validation at levels1-80')
