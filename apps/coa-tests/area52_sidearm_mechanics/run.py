from pathlib import Path
import subprocess
import tempfile
root=Path(__file__).resolve().parents[3]
def function(source,signature):
    start=source.index(signature)
    end=source.index("{",start)+1
    depth=1
    while depth:
        depth+=(source[end]=="{")-(source[end]=="}")
        end+=1
    return source[start:end]
free=(root/"src/server/coa/AscensionFreePick.cpp").read_text()
stat=(root/"src/server/game/Entities/Unit/StatSystem.cpp").read_text()
mgr=(root/"src/server/game/Spells/SpellMgr.cpp").read_text()
source=Path(__file__).with_name("harness.cpp.in").read_text()
source=source.replace("@SYNC@",function(free,"bool SynchronizeTalentRanks("))
source=source.replace("@HIT@","\n".join(function(stat,"void Player::"+name+"()") for name in ["UpdateMeleeHitChances","UpdateRangedHitChances","UpdateSpellHitChances"]))
source=source.replace("@GROUP@",function(mgr,"bool SpellMgr::AddSameEffectStackRuleSpellGroups("))
unit=(root/"src/server/game/Entities/Unit/Unit.cpp").read_text()
a=unit.index("[spellInfo,attacker](AuraEffect const* aurEff)")
b=unit.index("\n            });",a)
source=source.replace("@ARMOR@",unit[a:b]+"\n}")
with tempfile.TemporaryDirectory() as directory:
    path=Path(directory)
    (path/"check.cpp").write_text(source)
    subprocess.run(["g++","-std=c++20","-Wall","-Wextra",str(path/"check.cpp"),"-o",str(path/"check")],check=True)
    subprocess.run([str(path/"check")],check=True)
print("Production synchronization, all three native hit-stat updates and strongest-effect group selection passed with explicit engine adapters")
