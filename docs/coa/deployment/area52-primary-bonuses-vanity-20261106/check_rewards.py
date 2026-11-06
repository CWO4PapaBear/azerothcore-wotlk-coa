from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parent
before=(root/'baseline/Area52FeaturedRewardMail.inc').read_text()
after=(root/'candidate/src/server/coa/Area52FeaturedRewardMail.inc').read_text()
assert before.replace('Get<std::uint32_t>()','Get<std::uint8_t>()')==after
field=(root/'baseline/Field.cpp').read_text()
assert 'result = *reinterpret_cast<T const*>(data.value);' in field
def loader(source):
    a=source.index('                if (rows)');b=source.index('\n            }));',a)
    return source[a:b]
a=after.index('    std::uint32_t level = 0;');b=after.index('    if (!level)',a)
select=after[a:b]
code=r'''
#include <array>
#include <set>
#include <vector>
#include <cstdint>
#include <cassert>
struct Field{alignas(4) std::array<std::uint8_t,4> raw;template<class T>T Get(){return *reinterpret_cast<T const*>(raw.data());}};
struct Rows{std::vector<Field> fields;unsigned i=0;Field* Fetch(){return &fields.at(i);}bool NextRow(){return ++i<fields.size();}};
struct State{std::set<std::uint32_t> RewardLevels;};
void oldLoad(Rows* rows,State* current){
'''+loader(before)+r'''
}
void fixedLoad(Rows* rows,State* current){
'''+loader(after)+r'''
}
constexpr std::array<std::uint32_t,14> FeaturedRewardLevels{25,10,13,16,28,36,40,45,50,22,22,22,20,60};
struct Player{unsigned level;unsigned GetLevel(){return level;}};
unsigned next(Player* player,State& state,std::array<unsigned,14> const& items){
'''+select+r'''
return level;}
int main(){Rows rows;State old,fixed;std::set<unsigned> delivered={10,13,16,20,22,25,28,36,40};
for(auto level:delivered)rows.fields.push_back({{std::uint8_t(level),0x7f,0x21,0x42}});
oldLoad(&rows,&old);rows.i=0;fixedLoad(&rows,&fixed);assert(fixed.RewardLevels==delivered);assert(!old.RewardLevels.contains(10));
std::array<unsigned,14> items;items.fill(1);Player player{62};assert(next(&player,old,items)==10);
for(unsigned expected:{45,50,60}){assert(next(&player,fixed,items)==expected);fixed.RewardLevels.insert(expected);}
assert(!next(&player,fixed,items));auto saved=fixed.RewardLevels;rows.fields.clear();rows.i=0;
for(auto level:saved)rows.fields.push_back({{std::uint8_t(level),0xaa,0xbb,0xcc}});
State reload;fixedLoad(&rows,&reload);assert(reload.RewardLevels==saved);assert(!next(&player,reload,items));
State fresh;fixedLoad(nullptr,&fresh);assert(fresh.RewardLevels.empty());player.level=9;assert(!next(&player,fresh,items));player.level=10;assert(next(&player,fresh,items)==10);
}
'''
with tempfile.TemporaryDirectory() as d:
    p=Path(d)/'rewards.cpp';p.write_text(code);exe=p.with_suffix('')
    subprocess.run(['g++','-std=c++20',str(p),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
assert 'CharacterDatabase.AsyncCommitTransaction(transaction)' in after
assert 'if (!success)' in after and 'current->RewardsLoaded = false;' in after
print('PASS failing old raw tinyint-width receipt read, corrected native loader and selector, 45/50/60 catch-up at level62, saved replay dedup, empty and below-level controls. No database writes or gameplay.')
