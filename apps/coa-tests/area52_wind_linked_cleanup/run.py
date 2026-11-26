from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def main():
 source=(ROOT/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text();a=source.index('    // handle spell_linked_spell table');b=source.index('    // mods at aura apply',a)
 code=Path(__file__).with_name('harness.cpp').read_text().replace('@LINKS@',source[a:b])
 with tempfile.TemporaryDirectory() as tmp:
  cpp=Path(tmp)/'test.cpp';cpp.write_text(code);exe=Path(tmp)/'test';subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
 print('PASS actual native linked aura apply/reapply/removal: Deterrence and Killing Spree helpers, all remove modes, other caster ownership preserved')
if __name__=='__main__':main()
