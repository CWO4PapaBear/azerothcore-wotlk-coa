#include "Area52BuildProtocol.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <iterator>
int main(int argc,char**argv)
{
    assert(argc==2);
    std::ifstream input(argv[1]); std::string hex; input>>hex;
    std::vector<std::uint8_t> before;
    for(std::size_t i=0;i<hex.size();i+=2) before.push_back(std::stoul(hex.substr(i,2),nullptr,16));
    Area52Build::Entry entry; assert(Area52Build::Decode(before,entry));
    assert(Area52Build::Encode(entry)==before);
    std::map<unsigned,unsigned> levels{{64127,45},{64129,46},{31879,47},{31880,48},{31881,49}};
    std::map<unsigned,unsigned> old{{64127,46},{64129,45},{31879,48},{31880,49},{31881,47}};
    unsigned changed=0;
    for(auto& spell:entry.Spells) if (levels.contains(spell.Id))
    { assert(spell.Level==old.at(spell.Id)); spell.Level=levels.at(spell.Id); ++changed; }
    assert(changed==5);
    auto after=Area52Build::Encode(entry);
    Area52Build::Entry restored; assert(Area52Build::Decode(after,restored));
    for(auto& spell:restored.Spells) if (old.contains(spell.Id)) spell.Level=old.at(spell.Id);
    assert(Area52Build::Encode(restored)==before);
    constexpr char digits[]="0123456789abcdef";
    for(auto byte:after) std::cout<<digits[byte>>4]<<digits[byte&15];
    std::cout<<"\n";
}
