from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
source = r'''
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
using uint32 = std::uint32_t;
constexpr uint32 SPELL_INTERRUPT_FLAG_MOVEMENT = 1;
struct SpellInfo { uint32 Id; uint32 InterruptFlags; uint32 ChannelInterruptFlags; };
struct GlobalScript {
    explicit GlobalScript(char const*) {}
    virtual void OnLoadSpellCustomAttr(SpellInfo*) {}
};
struct Configuration {
    bool enabled = true;
    std::string model = "hero";
    std::string realm = "live";
    uint32 mode = 0x400;
    template<class T> T GetOption(char const* key, T fallback) {
        std::string name(key);
        if constexpr (std::is_same_v<T, bool>) return enabled;
        else if constexpr (std::is_same_v<T, uint32>) return mode;
        else if constexpr (std::is_same_v<T, std::string>)
            return name == "CoA.ClassModel" ? model : realm;
        return fallback;
    }
};
Configuration config;
Configuration* sConfigMgr = &config;
#include "Area52CastMovement.inc"
int main() {
    area52_cast_movement_metadata hook;
    unsigned count = 0;
    for (uint32 id = 0; id < 1000000; ++id) {
        if (!Area52CastMovement::RequiresInterruption(id)) continue;
        ++count;
        SpellInfo info{id, 14, 31756};
        hook.OnLoadSpellCustomAttr(&info);
        assert(info.InterruptFlags == 15 && info.ChannelInterruptFlags == 31756);
        hook.OnLoadSpellCustomAttr(&info);
        assert(info.InterruptFlags == 15);
        config.enabled = false;
        info.InterruptFlags = 14;
        hook.OnLoadSpellCustomAttr(&info);
        assert(info.InterruptFlags == 14);
        config.enabled = true;
        config.model = "coa";
        hook.OnLoadSpellCustomAttr(&info);
        assert(info.InterruptFlags == 14);
        config.model = "hero";
        config.realm = "seasonal";
        hook.OnLoadSpellCustomAttr(&info);
        assert(info.InterruptFlags == 14);
        config.realm = "live";
        config.mode = 0x401;
        hook.OnLoadSpellCustomAttr(&info);
        assert(info.InterruptFlags == 14);
        config.mode = 0x400;
    }
    assert(count == 107);
    for (uint32 id : {955072u, 834491u, 286356u, 133u, 0u}) {
        SpellInfo info{id, 14, 0};
        hook.OnLoadSpellCustomAttr(&info);
        assert(info.InterruptFlags == 14);
    }
}
'''
with tempfile.TemporaryDirectory() as temp:
    path = Path(temp)
    (path / 'test.cpp').write_text(source)
    subprocess.run(['c++', '-std=c++20', '-Wall', '-Wextra', '-Werror', '-I', str(root / 'src/server/coa'),
                    str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)
print('PASS: production movement metadata, 107 ranks, mode gates, idempotence and exception preservation.')
