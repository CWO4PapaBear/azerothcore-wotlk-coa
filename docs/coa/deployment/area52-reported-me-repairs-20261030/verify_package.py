from pathlib import Path
import os,shutil,subprocess
T=Path(__file__).resolve().parent;S=Path('/home/dml/builds/area52-playerbots-20261026/source');V=T/'verification-workspace'
(V/'tools').mkdir(parents=True,exist_ok=True)
if not (V/'.git').exists():subprocess.run(['git','init','--quiet',str(V)],check=True)
for p in (S/'tools').glob('*.py'):shutil.copyfile(p,V/'tools'/p.name)
H=V/'apps/coa-tests/reported-package';H.mkdir(parents=True,exist_ok=True);shutil.copyfile(T/'check_package.py',H/'run.py')
env=dict(os.environ,REPORTED_COMBINED_ROOT=str(T),GIT_CONFIG_COUNT='1',GIT_CONFIG_KEY_0='safe.directory',GIT_CONFIG_VALUE_0=str(V))
raise SystemExit(subprocess.run(['python3','-B','tools/verify_all.py','--stages','harness','--harness','reported-package','--output',str(T/'package-verification-r1')],cwd=V,env=env).returncode)
