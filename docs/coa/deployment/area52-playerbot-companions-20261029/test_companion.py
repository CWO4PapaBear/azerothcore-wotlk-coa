from pathlib import Path
import runpy

for name in ('test_pool_dispatch.py','test_recruitment.py','test_heirlooms.py','test_fixed_equipment.py'):
    runpy.run_path(str(Path('/results')/name), run_name='__main__')
