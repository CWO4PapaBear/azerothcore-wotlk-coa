from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
compiler = shutil.which(os.environ.get('CXX', 'c++'))
if not compiler:
    raise SystemExit('C++20 compiler unavailable')
source = r'''
#include "Area52HopliteSupportRules.h"
#include "Area52HopliteRemainingRules.h"
#include <cassert>
int main()
{
    using namespace Area52HopliteSupport;
    using namespace Area52HopliteRemaining;
    assert(HungerDuration(0) == 0);
    assert(HungerDuration(3) == 15000);
    assert(HungerSpeed(3) == 45);
    assert(AdditionalBlock(1000, 200, 25) == 200);
    assert(AdditionalBlock(1000, 200, 28) == 224);
    assert(AdditionalBlock(1000, 1200, 28) == 0);
    assert(AdditionalBlock(1000, 200, -5) == 0);
    for (auto root : {1464u, 23922u, 53600u, 978533u, 54158u, 20187u, 20271u, 300158u})
        assert(WindConsumer(root));
    assert(!WindConsumer(1752));
    assert(!WindConsumer(979189));
    assert(!ShieldConsumer(1464));
    assert(ShieldConsumer(978533));
    assert(DraconicConsumer(1752));
    assert(DraconicConsumer(978569));
    assert(!DraconicConsumer(978533));
    assert(ExecuteExtraRage(true, 150, 850) == 450);
    assert(ExecuteExtraRage(false, 150, 850) == 150);
    assert(ExecuteExtraRage(true, 150, 80) == 80);
    assert(ExecuteExtraRage(true, 700, 800) == 0);
    assert(int(ExecuteAttackPower(true, 1000)) == 270);
    assert(int(ExecuteAttackPower(false, 1000)) == 200);
    assert(RelentlessChance(7, 1) == 7);
    assert(RelentlessChance(7, 5) == 35);
    assert(RelentlessChance(14, 5) == 70);
    assert(RelentlessChance(20, 5) == 100);
    assert(RelentlessChance(20, 0) == 0);
    assert(RelentlessChance(20, 10) == 100);
    assert(DeepWoundsTickPercent(true, 3) == 5.5f);
    assert(DeepWoundsTickPercent(false, 3) == 8.0f);
    for (auto aura : {12317, 13045, 13046, 13047, 13048})
        for (auto root : {2687, 18499, 1719})
            assert(Enrage(aura, root, true, false, false, false));
    assert(!Enrage(12317, 78, true, true, false, false));
    assert(!Enrage(12317, 0, false, true, true, false));
    assert(Enrage(13048, 0, false, true, true, false));
    assert(!Enrage(13048, 0, false, true, false, false));
    assert(!Enrage(13048, 0, false, true, true, true));
    assert(Honor(51701, true, true, false, true));
    assert(!Honor(51700, true, true, false, true));
    assert(!Honor(51701, false, true, false, true));
    assert(!Honor(51701, true, false, false, true));
    assert(!Honor(51701, true, true, true, true));
    assert(!Honor(51701, true, true, false, false));
    assert(Spartan(1766, true, true, true));
    assert(!Spartan(1752, true, true, true));
    assert(!Spartan(1766, false, true, true));
    assert(!Spartan(1766, true, false, true));
    assert(!Spartan(1766, true, true, false));
}
'''
source = '#include <initializer_list>\n' + source
with tempfile.TemporaryDirectory() as tmp:
    p = Path(tmp)
    (p / 'test.cpp').write_text(source)
    subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                    '-I' + str(root / 'src/server/coa'), str(p / 'test.cpp'), '-o', str(p / 'test')], check=True)
    subprocess.run([str(p / 'test')], check=True)
print('Enrage sources/capstone, Honor direct-damage boundary and Spartan player/stance filters passed.')
