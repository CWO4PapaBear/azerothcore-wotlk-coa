from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[3]
s=(root/"src/server/game/Spells/SpellInfo.cpp").read_text();a=s.index("void SpellInfo::ApplyAllSpellImmunitiesTo(");b=s.index("\nbool SpellInfo::CanSpellProvideImmunityAgainstAura",a)
code=Path(__file__).with_name("harness.cpp.in").read_text().replace("@NATIVE@",s[a:b])
with tempfile.TemporaryDirectory() as directory:
    p=Path(directory);(p/'test.cpp').write_text(code)
    subprocess.run(['g++','-std=c++20','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
print('Production immunity dispatcher applies/removes root and snare protections without removing independent immunity; target storage is an adapter')
