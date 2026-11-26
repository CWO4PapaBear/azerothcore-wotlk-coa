#include "Area52CreationPreview.h"
#include "Area52CreationPreviewDefaults.h"
#include <cassert>
#include <iostream>
#include <set>
#include <string>

int main()
{
    using namespace Area52Build;
    std::set<unsigned> archetypes;
    for (auto const& baseline : CreationPreviewDefaults)
    {
        archetypes.insert(baseline[1]);
        for (unsigned stance : {0u,25u,26u,27u,28u,29u,48u,51u,52u,UINT32_MAX})
        {
            for (unsigned mode = 0; mode != 4; ++mode)
            {
                auto row = baseline;
                std::string description = "###AREA52_FEATURED_STANCE\n1:" + std::to_string(stance)
                    + "\n###AREA52_FEATURED_WEAPONS\n1:" + std::to_string(mode);
                assert(ProjectCreationPreview(row, description));
                for (unsigned i = 0; i < row.size(); ++i)
                {
                    unsigned expected = baseline[i];
                    if (i == 12 && stance != UINT32_MAX) expected = stance;
                    if ((i == 25 || i == 26) && (mode == 2 || mode == 3)) expected = 0;
                    if (i == 27 && (mode == 1 || mode == 3)) expected = 0;
                    assert(row[i] == expected);
                }
                auto restored = baseline;
                assert(ProjectCreationPreview(restored, "overview"));
                assert(restored == baseline);
            }
        }
    }
    for (std::string bad : {"1:99", "2:26", "1:-1", "1:4294967296", "1:26junk", "1:", "1:26,48",
        "1:26\n###AREA52_FEATURED_STANCE\n1:48"})
    {
        CreationSelection selection;
        assert(!ReadCreationSelection("###AREA52_FEATURED_STANCE\n" + bad, selection));
    }
    CreationSelection selection;
    assert(!ReadCreationSelection("x###AREA52_FEATURED_STANCE\n1:26", selection));
    assert(!ReadCreationSelection("###AREA52_FEATURED_WEAPONS\n1:4", selection));
    auto row = CreationPreviewDefaults[0];
    assert(ProjectCreationPreview(row, "###AREA52_FEATURED_VANITY\n1:1,2,3,4,5,6,7,8,9,10,11,12,13,14\n"
        "###AREA52_FEATURED_PREVIEW\n1:11783,4294967295,4294967295,4294967295"));
    for (unsigned i = 0; i < 12; ++i) assert(row[16+i] == i+1);
    assert(row[13] == 11783);
    for (auto const& original : CreationPreviewDefaults)
    {
        auto const mapping = std::find_if(CreationIncarnations.begin(), CreationIncarnations.end(),
            [&](auto const& entry) { return entry[2] == original[14]; });
        if (mapping == CreationIncarnations.end()) continue;
        auto const category = std::find(FeaturedIncarnationCategories.begin(), FeaturedIncarnationCategories.end(), (*mapping)[1]);
        if (category == FeaturedIncarnationCategories.end()) continue;
        auto const index = category - FeaturedIncarnationCategories.begin();
        std::string cleared = "###AREA52_FEATURED_INCARNATIONS\n1:";
        for (unsigned i=0;i<4;++i){if(i)cleared+=",";cleared+=i==index?"0":"4294967295";}
        row=original;assert(ProjectCreationPreview(row,cleared));assert(row[14]==0);
        row=original;assert(ProjectCreationPreview(row,""));assert(row==original);
    }
    static_assert(sizeof(CreationPreviewRow) == 112);
    std::cout << "PASS " << CreationPreviewDefaults.size() << " rows, " << archetypes.size()
        << " archetypes; ten stances x four weapon modes, defaults, equipment, pet and invalid metadata\n";
}
