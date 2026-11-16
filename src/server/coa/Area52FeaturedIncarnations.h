#ifndef AREA52_FEATURED_INCARNATIONS_H
#define AREA52_FEATURED_INCARNATIONS_H
#include "Area52FeaturedPreview.h"
#include <string>
namespace Area52Build
{
constexpr std::array<std::uint32_t, 4> FeaturedIncarnationCategories{17, 18, 23, 31};
inline bool ReadFeaturedIncarnations(std::string_view description, FeaturedPreviewEffects& values)
{
    constexpr std::string_view marker = "###AREA52_FEATURED_INCARNATIONS\n";
    auto const start = description.find(marker);
    values.fill(FeaturedPreviewDefault);
    if (start == std::string_view::npos)
        return true;
    if ((start && description[start - 1] != '\n') || description.find(marker, start + marker.size()) != std::string_view::npos)
        return false;
    auto const end = description.find('\n', start + marker.size());
    std::string encoded = "###AREA52_FEATURED_PREVIEW\n";
    encoded += description.substr(start + marker.size(), end == std::string_view::npos ? end : end - start - marker.size());
    if (!ReadFeaturedPreview(encoded, values))
        return false;
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (!values[i] || values[i] == FeaturedPreviewDefault)
            continue;
        bool valid = false;
        switch (FeaturedIncarnationCategories[i])
        {
            case 17:
                valid = values[i] == 822
                    || values[i] == 45401
                    || values[i] == 45402
                    || values[i] == 45403
                    || values[i] == 47124
                    || values[i] == 47250
                    || values[i] == 47251
                    || values[i] == 47522
                    || values[i] == 47524
                    || values[i] == 47533
                    || values[i] == 47536
                    || values[i] == 47537
                    || values[i] == 47538
                    || values[i] == 47539
                    || values[i] == 47540
                    || values[i] == 47541
                    || values[i] == 47542
                    || values[i] == 47543
                    || values[i] == 47544
                    || values[i] == 47545
                    || values[i] == 47547
                    || values[i] == 47548
                    || values[i] == 47549
                    || values[i] == 47550
                    || values[i] == 47551
                    || values[i] == 47552
                    || values[i] == 47553
                    || values[i] == 47554
                    || values[i] == 47555
                    || values[i] == 47556
                    || values[i] == 47557
                    || values[i] == 47558
                    || values[i] == 47559
                    || values[i] == 47560
                    || values[i] == 47561
                    || values[i] == 47562
                    || values[i] == 61375
                    || values[i] == 61376
                    || values[i] == 61377
                    || values[i] == 61378
                    || values[i] == 261377
                    || values[i] == 261378
                    || values[i] == 261379
                    || values[i] == 261380
                    || values[i] == 261381
                    || values[i] == 261382;
                break;
            case 18:
                valid = values[i] == 645
                    || values[i] == 646
                    || values[i] == 647
                    || values[i] == 648
                    || values[i] == 649
                    || values[i] == 650
                    || values[i] == 651
                    || values[i] == 652
                    || values[i] == 6004
                    || values[i] == 6005
                    || values[i] == 6006
                    || values[i] == 6007
                    || values[i] == 6008
                    || values[i] == 6009
                    || values[i] == 47563
                    || values[i] == 47565
                    || values[i] == 47566
                    || values[i] == 47567
                    || values[i] == 47568
                    || values[i] == 47569
                    || values[i] == 47571
                    || values[i] == 47572
                    || values[i] == 47573
                    || values[i] == 47574
                    || values[i] == 47575
                    || values[i] == 47576
                    || values[i] == 47577
                    || values[i] == 47578
                    || values[i] == 47579
                    || values[i] == 47580
                    || values[i] == 47581
                    || values[i] == 47582
                    || values[i] == 47584
                    || values[i] == 47585
                    || values[i] == 47586
                    || values[i] == 47587
                    || values[i] == 47588
                    || values[i] == 47589
                    || values[i] == 47590
                    || values[i] == 47591
                    || values[i] == 47592
                    || values[i] == 47593
                    || values[i] == 47595
                    || values[i] == 47596
                    || values[i] == 47597
                    || values[i] == 47598
                    || values[i] == 47599
                    || values[i] == 47600
                    || values[i] == 47601
                    || values[i] == 47602
                    || values[i] == 47603
                    || values[i] == 47604
                    || values[i] == 47605
                    || values[i] == 47606
                    || values[i] == 47607
                    || values[i] == 47608
                    || values[i] == 47609
                    || values[i] == 47610
                    || values[i] == 47611
                    || values[i] == 47612
                    || values[i] == 47613
                    || values[i] == 47614
                    || values[i] == 47615
                    || values[i] == 51429;
                break;
            case 23:
                valid = values[i] == 15605
                    || values[i] == 15606
                    || values[i] == 15607
                    || values[i] == 15608
                    || values[i] == 15609
                    || values[i] == 15610
                    || values[i] == 15611
                    || values[i] == 47640
                    || values[i] == 47642
                    || values[i] == 47647
                    || values[i] == 47648
                    || values[i] == 47649
                    || values[i] == 47650
                    || values[i] == 47651
                    || values[i] == 47652
                    || values[i] == 47653
                    || values[i] == 47654
                    || values[i] == 47655
                    || values[i] == 47656
                    || values[i] == 47657
                    || values[i] == 47658
                    || values[i] == 47659
                    || values[i] == 47660
                    || values[i] == 47661
                    || values[i] == 179673
                    || values[i] == 179674
                    || values[i] == 179675
                    || values[i] == 179676
                    || values[i] == 179677
                    || values[i] == 179678;
                break;
            case 31:
                valid = values[i] == 51205
                    || values[i] == 51206
                    || values[i] == 51207
                    || values[i] == 51208
                    || values[i] == 51209
                    || values[i] == 51210
                    || values[i] == 51211
                    || values[i] == 51212
                    || values[i] == 51213
                    || values[i] == 51214
                    || values[i] == 51215
                    || values[i] == 51216
                    || values[i] == 51217;
                break;
        }
        if (!valid)
            return false;
    }
    return true;
}
}
#endif
