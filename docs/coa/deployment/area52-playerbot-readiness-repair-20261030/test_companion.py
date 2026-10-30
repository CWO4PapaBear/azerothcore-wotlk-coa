from pathlib import Path
import runpy

for name in ('test_pool_dispatch.py','test_recruitment.py','test_heirlooms.py','test_fixed_equipment.py','test_readiness_paths.py'):
    runpy.run_path(str(Path('/results')/name), run_name='__main__')

import os
os.environ['AREA52_EXPECT_READINESS_REPAIR']='1'
runpy.run_path(str(Path('/results')/'test_readiness_catalog.py'), run_name='__main__')
