import hashlib
import importlib.util
import json
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[3]
directory = root / 'apps/coa-gameplay-test'
sys.path.insert(0, str(directory))
import run

legacy = directory / 'blocked-regressions/level-scaling-damage-engagement.json'
assert hashlib.sha256(legacy.read_bytes()).hexdigest() == '19e9df27e10943a3e14305c026bc38f456d833fe91a2801ad185f4a77aa9ff1b'
assert not (directory / 'scenarios' / legacy.name).exists()
try:
    run.validate(json.loads(legacy.read_text()))
except ValueError as error:
    assert "unknown fields ['level_scaling']" in str(error)
else:
    raise AssertionError('Legacy fixture unexpectedly became runnable')

for name in ['witch-hunter-noctis-blade-scaled-heal', 'level-scaling-personal-opening-hit']:
    data = json.loads((directory / 'scenarios' / (name + '.json')).read_text())
    run.validate(data)
    assert all('level_scaling' not in creature for creature in data['creatures'])
    assert any(step['action'] == 'level_scaling_packet' and step['value'] == 1 for step in data['steps'])
    if name.startswith('witch-hunter'):
        assert hashlib.sha256(json.dumps(data['steps'], sort_keys=True).encode()).hexdigest() == '8c6f46be4c36c8dc823472aa6767382f342ab4060aed80050b34d48e698de874'
    else:
        assertions = [step for step in data['steps'] if step['action'] == 'assert']
        assert len(assertions) == 15
        assert sum(step['metric'] == 'health' and step.get('max') == -1 for step in assertions) == 2
        assert sum(step['metric'] == 'level' and step.get('equals') == 1 for step in assertions) == 4
        assert any(step['metric'] == 'view_level' and step.get('equals') == 55 for step in assertions)
        assert any(step['metric'] == 'view_level' and step.get('equals') == 1 for step in assertions)
print('Scenario definitions validate; Noctis assertions and blocked legacy bytes preserved. Gameplay not executed.')
