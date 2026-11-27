from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
source = r"""
#include "Area52DragonReviewRules.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>
using uint32 = std::uint32_t;
using int32 = std::int32_t;
constexpr uint32 PROC_FLAG_DONE_MELEE_AUTO_ATTACK = 4;
constexpr int SPELL_ATTR0_PASSIVE=64;constexpr int GLOBALHOOK_ON_LOAD_SPELL_CUSTOM_ATTR = 1;
constexpr int EFFECT_0 = 0, EFFECT_1 = 1, EFFECT_2 = 2, SPELL_EFFECT_APPLY_AURA = 6, SPELL_EFFECT_NONE=0, SPELL_AURA_DUMMY=4, TARGET_UNIT_CASTER=1;
int SpellImplicitTargetInfo(int x){return x;}
struct Effect { int Effect = 0; int BasePoints = 0; int DieSides = 0; int ApplyAuraName=0, TargetA=0; uint32 SpellClassMask[3]{}; };
struct SpellInfo { uint32 Id; Effect Effects[3]; uint32 ProcFlags = 0, Attributes=0; };
struct GlobalScript {
 GlobalScript(char const*, std::vector<int>) {}
 virtual void OnLoadSpellCustomAttr(SpellInfo*) {}
};
struct Config {
 bool enabled = true;
 std::string model = "hero", realm = "live";
 uint32 mode = 0x400;
 template<class T> T GetOption(char const* key, T) {
  if constexpr (std::is_same_v<T, bool>) return enabled;
  else if constexpr (std::is_same_v<T, uint32>) return mode;
  else return std::string(key) == "CoA.ClassModel" ? model : realm;
 }
} config;
Config* sConfigMgr = &config;
@@METADATA@@
int main() {
 for (uint32 count = 0; count < 12; ++count)
  assert(std::abs(Area52DragonReview::BloodthirstBleedMultiplier(count) -
   (1.0f + 0.08f * std::min(count, 5u))) < 0.00001f);
 for (uint32 school = 0; school < 128; ++school) {
  assert(Area52DragonReview::BurningRushAllowsMovement(true, true, school) == bool(school & 4));
  assert(!Area52DragonReview::BurningRushAllowsMovement(false, true, school));
  assert(!Area52DragonReview::BurningRushAllowsMovement(true, false, school));
 }
 assert(Area52DragonReview::BurningRushDrain(10000) == 500);
 assert(Area52DragonReview::BurningRushDrain(9999) == 499);
 assert(Area52DragonReview::BurningRushDrain(UINT32_MAX) == 214748364);
 area52_dragon_review_metadata hook;
  SpellInfo capstone{820503,{}};capstone.Effects[2].ApplyAuraName=42;hook.OnLoadSpellCustomAttr(&capstone);assert(capstone.Effects[2].ApplyAuraName==SPELL_AURA_DUMMY);
 for(uint32 rank : {46867u,56611u,56612u}) {
  SpellInfo info{rank,{}};info.ProcFlags=0x400014;hook.OnLoadSpellCustomAttr(&info);
  assert(info.ProcFlags==0x400010);
  assert(info.ProcFlags&(0x400000|4));assert(!(info.ProcFlags&(0x800000|4)));
  assert(info.ProcFlags&16);
 }
 SpellInfo recast{760096,{}};hook.OnLoadSpellCustomAttr(&recast);
 assert(recast.Effects[0].BasePoints==760094&&recast.Effects[0].DieSides==0);

 SpellInfo execute{20502, {}}, rush{954806, {}};
 rush.Effects[1].BasePoints = 114;
 hook.OnLoadSpellCustomAttr(&execute); hook.OnLoadSpellCustomAttr(&rush);
 assert(execute.Effects[0].Effect == 6 && rush.Effects[1].BasePoints == 99);
 hook.OnLoadSpellCustomAttr(&execute); hook.OnLoadSpellCustomAttr(&rush);
 assert(execute.Effects[0].Effect == 6 && rush.Effects[1].BasePoints == 99);
 for (unsigned variant = 0; variant < 4; ++variant) {
  config.enabled = variant != 0; config.model = variant == 1 ? "coa" : "hero";
  config.realm = variant == 2 ? "seasonal" : "live"; config.mode = variant == 3 ? 0x401 : 0x400;
  execute.Effects[0].Effect = 0; rush.Effects[1].BasePoints = 114;
  hook.OnLoadSpellCustomAttr(&execute); hook.OnLoadSpellCustomAttr(&rush);
  assert(execute.Effects[0].Effect == 0 && rush.Effects[1].BasePoints == 114);
 }
}
"""
warrior = (root / 'src/server/coa/AscensionHeroWarrior.cpp').read_text()
metadata = warrior[warrior.index('class area52_dragon_review_metadata'):warrior.index('class aura_area52_burning_rush')]
source = source.replace('@@METADATA@@', metadata)
with tempfile.TemporaryDirectory() as directory:
 path = Path(directory)
 (path / 'test.cpp').write_text(source)
 subprocess.run(['c++', '-std=c++20', '-Wall', '-Wextra', '-Werror', '-I', str(root / 'src/server/coa'),
                 str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
 subprocess.run([str(path / 'test')], check=True)
print('PASS: production numeric rules and cold-load metadata, repeat application and other-mode isolation.')
