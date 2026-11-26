from pathlib import Path
import os,subprocess,tempfile

here=Path(__file__).parent
module=Path(os.environ['AREA52_BOT_MODULE'])
source=(module/'src/Ai/Coa/CoaGenericContext.cpp').read_text()
method=source[source.index('bool CoaDefiasAction::Execute(Event)'):]
with tempfile.TemporaryDirectory() as directory:
 p=Path(directory)
 (p/'check.cpp').write_text((here/'defias_fixture.cpp.in').read_text().replace('@@METHOD@@',method))
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra',str(p/'check.cpp'),'-o',str(p/'check')],check=True)
 subprocess.run([str(p/'check')],check=True)
print('PASS production Defias action: learned-spell gates, opener, shot cycle, execute, escape/GCD/safe Blink, dispel, food hysteresis, two weapon poisons, group buffs, refreshment, AoE and no-target refusal')
