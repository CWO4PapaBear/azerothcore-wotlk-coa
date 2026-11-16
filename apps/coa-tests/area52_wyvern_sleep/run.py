from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[3]
source='''#include "Area52WyvernSleep.h"
using Area52WyvernSleep::PreservesSleep;
static_assert(PreservesSleep(true,24131,19386,1978));
static_assert(!PreservesSleep(false,24131,19386,1978));
static_assert(!PreservesSleep(true,24131,619386,1978));
static_assert(!PreservesSleep(true,24131,0,1978));
static_assert(!PreservesSleep(true,24131,19386,2818));
static_assert(!PreservesSleep(true,24131,19386,0));
static_assert(!PreservesSleep(true,24134,19386,1978));
int main(){}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(source)
 subprocess.run(['c++','-std=c++20','-I'+str(root/'src/server/coa'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Area52 Wyvern-origin sleep alone exempts Serpent Sting; other poison, damage, sources and realms unchanged.')
