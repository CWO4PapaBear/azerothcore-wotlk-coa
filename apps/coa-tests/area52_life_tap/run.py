from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[3]
source='''#include "Area52LifeTap.h"
using namespace Area52LifeTap;
static_assert(Cost(10000,20000)==1200);
static_assert(Cost(20000,10000)==1200);
static_assert(Cost(10000,10000)==600);
static_assert(Cost(101,0)==6);
static_assert(Cost(1,0)==1);
static_assert(Cost(4294967295u,0)==257698037u);
static_assert(!CanPay(1200,1200));
static_assert(!CanPay(1199,1200));
static_assert(CanPay(1201,1200));
int main(){}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(source)
 subprocess.run(['c++','-std=c++20','-I'+str(root/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Life Tap uses 6 percent of the higher maximum; rounding, wide arithmetic and nonlethal health boundaries pass.')

