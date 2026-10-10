from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[3]
s=(root/'src/server/game/Achievements/AchievementMgr.cpp').read_text()
a=s.index('bool AchievementGlobalMgr::IsStatisticCriteria(');b=s.index('bool AchievementGlobalMgr::IsAverageCriteria(',a)
fixture='''
#include <cassert>
#include <map>
struct AchievementEntry { unsigned categoryId; };
struct AchievementCriteriaEntry { unsigned referredAchievement; };
struct AchievementCategoryEntry { unsigned ID; unsigned parentCategory; };
template<class T> struct Store { std::map<unsigned,T> rows; T const* LookupEntry(unsigned id) const { auto i=rows.find(id); return i==rows.end()?nullptr:&i->second; } };
Store<AchievementEntry> sAchievementStore;
Store<AchievementCategoryEntry> sAchievementCategoryStore;
constexpr unsigned ACHIEVEMENT_CATEGORY_STATISTICS=1, ACHIEVEMENT_CATEOGRY_GENERAL=2;
struct AchievementGlobalMgr { bool IsStatisticCriteria(AchievementCriteriaEntry const*) const; bool IsStatisticAchievement(AchievementEntry const*) const; };
'''+s[a:b]+'''
int main() {
 AchievementGlobalMgr mgr;
 AchievementEntry missing{14921};
 assert(!mgr.IsStatisticAchievement(&missing));
 assert(!mgr.IsStatisticAchievement(nullptr));
 assert(!mgr.IsStatisticCriteria(nullptr));
 AchievementCriteriaEntry orphan{999}; assert(!mgr.IsStatisticCriteria(&orphan));
 sAchievementCategoryStore.rows={{1,{1,0}},{2,{2,0}},{3,{3,1}},{4,{4,2}},{5,{5,999}}};
 AchievementEntry statistics{3},general{4},brokenParent{5};
 assert(mgr.IsStatisticAchievement(&statistics));
 assert(!mgr.IsStatisticAchievement(&general));
 assert(!mgr.IsStatisticAchievement(&brokenParent));
 sAchievementStore.rows[490]=missing;
 AchievementCriteriaEntry nexus{490}; assert(!mgr.IsStatisticCriteria(&nexus));
 sAchievementCategoryStore.rows[14921]={14921,2}; assert(!mgr.IsStatisticCriteria(&nexus));
}
'''
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'test.cpp').write_text(fixture)
 subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Achievement classification: missing category, parent, achievement, criteria and valid ancestry passed')
