#include "Area52CacheSelection.h"
#include <cassert>
using namespace Area52CacheSelection;
Candidate Gear(unsigned id, unsigned level, unsigned cap)
{
    Candidate value;
    value.item = id;
    value.itemLevel = level;
    value.requiredLevel = cap;
    value.contentUnlocked = value.usable = value.bonusesQualified = true;
    return value;
}
Result Open(std::vector<Candidate> catalog, float equipped, unsigned cap)
{
    float average = CallboardAverage(catalog, equipped, cap);
    for (auto& item : catalog)
        if (item.itemLevel < average || item.itemLevel > average + 6)
            item.contentUnlocked = false;
    return Select(catalog, unsigned(average), 6, {}, cap, true);
}
int main()
{
    for (unsigned cap : {60u, 70u, 80u})
    {
        auto items = std::vector{Gear(1, 200, cap), Gear(2, 206, cap), Gear(3, 207, cap)};
        assert(CallboardAverage(items, 0, cap) == 200);
        assert(CallboardAverage(items, 190, cap) == 200);
        assert(Open(items, 0, cap).choices.size() == 2);
        assert(Open(items, 190, cap).choices.size() == 2);
        assert(CallboardAverage(items, 205.5, cap) == 205.5);
        auto fractional = Open(items, 205.5, cap);
        assert(fractional.choices.size() == 2 && fractional.choices[0].item == 2);
        items[0].owned = items[1].owned = true;
        assert(CallboardAverage(items, 0, cap) == 200);
        assert(Open(items, 0, cap).choices.empty());
        items[0].contentUnlocked = false;
        items[1].usable = false;
        assert(CallboardAverage(items, 0, cap) == 207);
        assert(Open(items, 0, cap).choices[0].item == 3);
        items[2].bonusesQualified = false;
        assert(Open(items, 0, cap).choices.empty());
    }
    std::vector mixed{Gear(1, 50, 60), Gear(2, 100, 70), Gear(3, 200, 80)};
    assert(Open(mixed, 0, 80).choices[0].item == 3);
    assert(Open(mixed, 0, 70).choices[0].item == 2);
    assert(Open(mixed, 0, 60).choices[0].item == 1);
    assert(Open(mixed, 0, 50).choices.empty());
    assert(Open({}, 0, 80).choices.empty());
}
