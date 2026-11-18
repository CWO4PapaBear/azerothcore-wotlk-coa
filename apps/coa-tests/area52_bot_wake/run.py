import subprocess,tempfile,os
from pathlib import Path
HERE=Path(__file__).parent
MODULE=Path(os.environ.get('AREA52_BOT_MODULE',str(HERE.resolve().parents[2] / 'modules/mod-playerbots')))
with tempfile.TemporaryDirectory() as d:
 exe=Path(d)/'wake'
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-I'+str(MODULE/'src'),str(HERE/'harness.cpp'),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
print('Wake scheduler: initial delay, faction/role waves, no catch-up, failed-login retries, refill, capacity and presence reset passed')

text=(MODULE/'src/Bot/RandomPlayerbotMgr.cpp').read_text()
start=text.index('auto loginArea52Party = [&](uint32 room)')
start=text.index('{',start)
end=text.index('\n            };',start)
method=('void RandomPlayerbotMgr::LoginArea52Party(uint32 room) '+text[start:end]+'\n}').replace('std::chrono::steady_clock::now()', '::now')
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'integration.cpp').write_text((HERE/'integration.cpp.in').read_text().replace('@@METHOD@@',method))
 subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-I'+str(MODULE/'src'),str(p/'integration.cpp'),'-o',str(p/'integration')],check=True)
 subprocess.run([str(p/'integration')],check=True)
print('Production roster/login method with DB/session adapters: bounded reads, saved roles, foreign-account exclusion, partial retries and logout reset passed')
