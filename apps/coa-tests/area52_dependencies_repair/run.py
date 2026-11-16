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
#include "Area52DependencyRules.h"
#include "AscensionFreePickLinkedSpells.h"
#include <cassert>
#include <algorithm>
int main()
{
    using namespace Area52Dependency;
    assert(NightfallSource(172) && NightfallSource(689) && NightfallSource(1120) && NightfallSource(47897));
    assert(!NightfallSource(686) && !NightfallSource(348));
    assert(HotStreakSource(133) && HotStreakSource(2136) && HotStreakSource(2948));
    assert(HotStreakSource(44457) && HotStreakSource(44461) && HotStreakSource(44614));
    assert(!HotStreakSource(11366) && !HotStreakSource(6353));
    assert(FingersSource(116, 3, 16, false));
    assert(FingersSource(8056, 11, 16, false));
    assert(!FingersSource(30455, 3, 16, false));
    assert(!FingersSource(42208, 3, 16, false));
    assert(!FingersSource(116, 3, 16, true));
    assert(!FingersSource(116, 5, 16, false));
    assert(!FingersSource(133, 3, 4, false));
    std::uint8_t streak = 0;
    assert(!CompleteCriticalPair(streak, true));
    assert(!CompleteCriticalPair(streak, false));
    assert(!CompleteCriticalPair(streak, true));
    assert(CompleteCriticalPair(streak, true));
    assert(streak == 0);
    assert(DamagePercent(686, false, true, false, false) == 120);
    assert(DamagePercent(686, true, true, false, false) == 75);
    assert(DamagePercent(29722, false, false, true, false) == 110);
    assert(DamagePercent(29722, true, false, true, false) == 80);
    assert(DamagePercent(11366, false, false, false, true) == 140);
    assert(DamagePercent(11366, true, false, false, true) == 100);
    assert(DamagePercent(133, false, true, true, true) == 100);
    assert(ManaBarrierAbsorb(1000, 1000, 1000, false) == 900);
    assert(ManaBarrierAbsorb(1000, 500, 1000, false) == 450);
    assert(ManaBarrierAbsorb(1000, 500, 1000, true) == 225);
    assert(ManaBarrierAbsorb(1000, 0, 1000, false) == 0);
    assert(ManaBarrierAbsorb(1000, 1000, 0, false) == 0);
    assert(MissingManaSurge(1000, 1000) == 0);
    assert(MissingManaSurge(500, 1000) == 150);
    assert(MissingManaSurge(0, 1000) == 300);
    assert(PlayerDamageTakenPercent(false, true, false) == 100);
    assert(PlayerDamageTakenPercent(true, true, false) == 200);
    assert(PlayerDamageTakenPercent(true, false, true) == 115);
    auto const& groups = AscensionFreePick::LinkedSpellGroups;
    auto found = std::find_if(groups.begin(), groups.end(), [](auto const& g) { return g.Parent == 33917; });
    assert(found != groups.end() && found->Children[0] == 33876 && found->Children[1] == 33878);
}
'''
with tempfile.TemporaryDirectory() as tmp:
    p = Path(tmp)
    (p / 'test.cpp').write_text(source)
    subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror', '-I' + str(root / 'src/server/coa'),
                    str(p / 'test.cpp'), '-o', str(p / 'test')], check=True)
    subprocess.run([str(p / 'test')], check=True)
print('Dependency filters, critical streak reset, target damage rules and Mangle grants passed.')
