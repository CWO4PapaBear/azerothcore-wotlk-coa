import os
import resource
from pathlib import Path
import shutil
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[3]
def extract(path, signature):
    s = (ROOT / path).read_text()
    start = s.index(signature)
    end = s.index('{', start) + 1
    depth = 1
    while depth:
        depth += (s[end] == '{') - (s[end] == '}')
        end += 1
    return s[start:end]
def main():
    code = Path(__file__).with_name('harness.cpp').read_text()
    code = code.replace('GRANT_FUNCTION', extract('src/server/coa/AscensionMysticEnchant.cpp', 'void SetGrantedSpell('))
    code = code.replace('RANK_FUNCTIONS', '\n'.join(extract('src/server/coa/AscensionFreePick.cpp', s) for s in ['std::vector<RankUpgrade> RankUpgrades(', 'bool LearnRankUpgrade(', 'std::uint32_t LearnAvailableRanks(']))
    code = code.replace('ADD_ACTION_FUNCTION', extract('src/server/game/Entities/Player/Player.cpp', 'ActionButton* Player::addActionButton('))
    code = code.replace('ACTION_FUNCTION', extract('src/server/game/Entities/Player/Player.cpp', 'bool Player::IsActionButtonDataValid('))
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    mystic = (ROOT/'src/server/coa/AscensionMysticEnchant.cpp').read_text()
    assert mystic.count('ExactGrantPrerequisiteMet(grant, [player]') == 2
    storage = (ROOT/'src/server/game/Entities/Player/PlayerStorage.cpp').read_text()
    assert storage.index('RestoreArea52MysticSpellsForLogin(this,') < storage.index('_LoadActions(holder.')
    with tempfile.TemporaryDirectory() as temp:
        p = Path(temp)
        (p/'test.cpp').write_text(code)
        subprocess.run([shutil.which(os.environ.get('CXX','c++')), '-std=c++20', '-I', str(ROOT/'src/server/coa'), str(p/'test.cpp'), '-o', str(p/'test')], check=True)
        subprocess.run([str(p/'test')],check=True)
        old = subprocess.check_output(['git','show','d7ebd91a9a866cb9f3a090ea1673524a259d7665:src/server/coa/AscensionFreePick.cpp'],cwd=ROOT,text=True)
        begin=old.index('std::vector<RankUpgrade> RankUpgrades(')
        end=old.index('bool LearnRankUpgrade(',begin)
        current=extract('src/server/coa/AscensionFreePick.cpp','std::vector<RankUpgrade> RankUpgrades(')
        (p/'test.cpp').write_text(code.replace(current,old[begin:end]))
        subprocess.run([shutil.which(os.environ.get('CXX','c++')), '-std=c++20', '-I', str(ROOT/'src/server/coa'), str(p/'test.cpp'), '-o', str(p/'baseline')],check=True)
        baseline=subprocess.run([str(p/'baseline')],capture_output=True,text=True)
        assert baseline.returncode != 0
        assert 'LearnRankUpgrade' in baseline.stderr or 'p.HasSpell' in baseline.stderr
        print('PASS: original rank-up implementation fails the same regression')
    print('PASS: 12 ranks, both adapter restoration orders, repeated sync, native action validation, Lil Red 20/22, ownership and cleanup')
if __name__ == '__main__':
    main()
