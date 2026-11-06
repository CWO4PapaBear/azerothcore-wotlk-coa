"""Qualify the final candidate with existing focused checks through the required runner."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import sys

root = Path(__file__).resolve().parent
workspace = root.parent / 'featured-mount-classification-20261103/verification-workspace'
checks = [
    'amounts', 'candidate', 'linked', 'strength_links', 'strength_rating',
    'agility_ap', 'agility_chance', 'agility_coefficients', 'agility_events',
    'scripted_agility', 'custom_agility', 'white_agility', 'parry_cap', 'retaliation',
    'power_input', 'power_paths', 'rockbiter_tagged', 'rockbiter_scaling',
    'rockbiter_persistence', 'secondary_candidate', 'dispels_candidate', 'packets',
    'arena_dispels', 'spirit_talents', 'tactical', 'talent_lifecycle', 'weapon_scope',
    'agile_strikes', 'package_hook', 'stance_rebase',
]
names = []
for check in checks:
    name = 'primary-stat-final-' + check.replace('_', '-')
    path = workspace / 'apps/coa-tests' / name
    path.mkdir(exist_ok=True)
    (path / 'run.py').write_text('import runpy\nrunpy.run_path(' + repr(str(root / ('check_' + check + '.py'))) + ', run_name="__main__")\n')
    names.append(name)

report = root / sys.argv[1]
if report.exists() or not report.name.startswith('final-offline-'):
    raise RuntimeError('Use a new final-offline-* report directory')
inputs = {}
for path in list((root / 'candidate').rglob('*')) + [root / ('check_' + check + '.py') for check in checks]:
    if path.is_file():
        inputs[path.relative_to(root).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
environment = dict(os.environ, GIT_CONFIG_COUNT='1', GIT_CONFIG_KEY_0='safe.directory', GIT_CONFIG_VALUE_0=str(workspace))
result = subprocess.run([
    'python3', '-B', 'tools/verify_all.py', '--stages', 'harness', '--jobs', '3',
    '--harness', *names, '--output', str(report),
], cwd=workspace, env=environment)
for relative, digest in inputs.items():
    if hashlib.sha256((root / relative).read_bytes()).hexdigest() != digest:
        raise RuntimeError('Input changed during verification: ' + relative)
(report / 'inputs.json').write_text(json.dumps(inputs, indent=2) + '\n')
raise SystemExit(result.returncode)
