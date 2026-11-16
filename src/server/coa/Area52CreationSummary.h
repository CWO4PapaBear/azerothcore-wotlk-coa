#pragma once
#include "utf8.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>

namespace Area52Build {
inline constexpr std::string_view SummaryMarker = "###AREA52_CREATION_SUMMARY";
inline std::string_view SummaryLine(std::string_view description)
{
    auto pos = description.find(SummaryMarker);
    if (pos == description.npos)
        return {};
    auto line = description.substr(pos + SummaryMarker.size());
    if ((pos && description[pos-1] != '\n') || !line.starts_with("\n") ||
        line.find(SummaryMarker) != line.npos)
        return "invalid";
    line.remove_prefix(1);
    return line.substr(0, line.find('\n'));
}
struct SummaryArchetype { unsigned id; std::array<unsigned,5> spells; };
inline constexpr SummaryArchetype SummaryArchetypes[] = {
    {1,{774,8936,8004,48438}},
    {2,{635,53563,596}},
    {3,{2050,2061,301001,34861}},
    {6,{635,888571,276273,20473}},
    {7,{331,61295,1064,974}},
    {10,{5185,740,1064,974}},
    {12,{2050,139,2061,47540}},
    {13,{116,2050,92754,596}},
    {14,{403,331,86435,86441}},
    {15,{92726,5185,92737,48438}},
    {16,{986163,986164,986165,276361}},
    {17,{86461,2120,14914,34861}},
    {18,{92779,276961,92770,982331,92778}},
    {19,{20243,53385,14278,14251}},
    {20,{922565,6343,23922,46968}},
    {21,{92789,6343,300048,983241}},
    {22,{2098,8647,16511,23922}},
    {24,{983854,6343,34428,14278}},
    {25,{701463,954425,986067,2120}},
    {26,{990224,890,19028,29722,980060}},
    {27,{954081,701463,983702,983703}},
    {29,{53600,954801,17,53595}},
    {30,{6343,403,977929,421}},
    {31,{701463,986067,983740,11426}},
    {32,{84443,33917,270356,22842,760100}},
    {33,{271403,172,33878,686}},
    {34,{92815,92806,92826,421}},
    {73,{978569,982250,86361,978533}},
    {74,{1495,2973,954821,983797}},
    {75,{982401,92366,2842,997746}},
    {77,{772,78,7384,12294}},
    {78,{1464,954250,954256,12294}},
    {79,{1464,984193,984201,984203}},
    {80,{275491,1752,2098,760050}},
    {82,{275317,56641,5171,3044,826679}},
    {84,{3044,272136,284079,19434}},
    {85,{965200,954587,34026,965289}},
    {86,{997048,997050,2973,1495,997052}},
    {88,{116,133,5143,44549,54488}},
    {90,{5176,283815,2912,48516,954800}},
    {95,{686,172,17877,273458}},
    {97,{116,954263,30455,31687,44572}},
    {101,{585,14914,965858,879,20473}},
    {104,{403,8042,965305,421,16166}},
    {129,{947470,1329,5308}},
    {131,{768,984116,984019,5676,984010}},
    {132,{53408,35395,53385,20375,954817}},
    {133,{1752,978302,978846,44572}},
    {134,{417699,2136,8024,760121,11366}},
    {136,{979748,35395,53385,14914}},
    {139,{954052,1978,5176,53209}},
    {140,{997526,997517,686,998204}},
    {141,{978945,1680,8232,983982}},
    {142,{6343,403,978823,978826,421}},
    {143,{986011,986008,779,6807,33745}},
    {144,{5740,29722,902186,6353,50796}},
};
inline bool SummaryNumber(std::string_view text, unsigned& value)
{
    if (text.empty() || text.front()=='0') return false;
    auto [end,error]=std::from_chars(text.data(),text.data()+text.size(),value);
    return error==std::errc{} && end==text.data()+text.size();
}
inline bool ValidCreationSummary(std::string_view description,unsigned archetype)
{
    if (description.find(SummaryMarker)==description.npos) return true;
    auto data=SummaryLine(description);
    if (!data.starts_with("1;")) return false;
    data.remove_prefix(2);
    auto source=std::find_if(std::begin(SummaryArchetypes),std::end(SummaryArchetypes),
        [archetype](auto const& s){return s.id==archetype;});
    if (source==std::end(SummaryArchetypes)) return false;
    auto remaining=source->spells;
    for (;;)
    {
        auto end=data.find(';');
        auto row=data.substr(0,end);
        auto a=row.find(':'), b=row.find(':',a==row.npos?a:a+1);
        unsigned spell=0,level=0;
        if (a==row.npos || b==row.npos || !SummaryNumber(row.substr(0,a),spell) ||
            !SummaryNumber(row.substr(a+1,b-a-1),level) || level>80) return false;
        auto slot=std::find(remaining.begin(),remaining.end(),spell);
        if (slot==remaining.end()) return false;
        *slot=0;
        auto bytes=row.substr(b+1);
        if (bytes.size()>2048 || bytes.size()%2) return false;
        std::string text;
        for (size_t i=0;i<bytes.size();i+=2)
        {
            auto pair=bytes.substr(i,2);
            if (pair.find_first_not_of("0123456789abcdef")!=pair.npos) return false;
            unsigned c=0;
            std::from_chars(pair.data(),pair.data()+2,c,16);
            if ((c<32 && c!=10 && c!=13) || c==127) return false;
            text.push_back(static_cast<char>(c));
        }
        if (!utf8::is_valid(text.begin(),text.end())) return false;
        if (end==data.npos) break;
        data.remove_prefix(end+1);
    }
    return std::all_of(remaining.begin(),remaining.end(),[](unsigned id){return !id;});
}
}
