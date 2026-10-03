#include "Area52BuildProtocol.h"
#include <cassert>
#include <fstream>
#include <iterator>

int main(int argc, char** argv)
{
    assert(argc == 2 || argc == 3);
    std::ifstream file(argv[1], std::ios::binary);
    std::vector<std::uint8_t> bytes(std::istreambuf_iterator<char>(file), {});
    Area52Build::Entry entry;
    assert(Area52Build::Decode(bytes, entry));
    if (argc == 3)
    {
        assert(entry.Name == "A52 Storage Test");
        assert(entry.Spells.size() == 9 && entry.Enchants.size() == 4);
        assert(Area52Build::Encode(entry) == bytes);
        return 0;
    }
    assert(entry.Name == "Fixture" && entry.Class == 10 && entry.Category == 1);
    assert(entry.Spells.size() == 1 && entry.Spells[0].Id == 100 && entry.Spells[0].Roles[0] == 1);
    assert(entry.Enchants.size() == 1 && entry.Enchants[0].Id == 200 && entry.Enchants[0].Stacks == 2);
    assert(entry.Equipment[0].size() == 1 && entry.Equipment[1].size() == 1);
    assert(entry.Created == 0x123456789ULL && entry.Difficulty == 2);
    assert(Area52Build::Encode(entry) == bytes);
    Area52Build::Entry sentinel;
    sentinel.Name = "unchanged";
    for (std::size_t size = 0; size < bytes.size(); ++size)
    {
        assert(!Area52Build::Decode(std::span(bytes).first(size), sentinel));
        assert(sentinel.Name == "unchanged");
    }
    auto extra = bytes;
    extra.push_back(0);
    assert(!Area52Build::Decode(extra, sentinel));
    auto duplicate = entry;
    duplicate.Spells.push_back(duplicate.Spells.front());
    assert(!Area52Build::Decode(Area52Build::Encode(duplicate), sentinel));
    duplicate = entry;
    duplicate.Enchants.push_back(duplicate.Enchants.front());
    assert(!Area52Build::Decode(Area52Build::Encode(duplicate), sentinel));
    duplicate = entry;
    duplicate.Spells[0].Roles[0] = 2;
    assert(!Area52Build::Decode(Area52Build::Encode(duplicate), sentinel));
    duplicate = entry;
    duplicate.Description.assign(8193, 'x');
    assert(!Area52Build::Decode(Area52Build::Encode(duplicate), sentinel));
    duplicate = entry;
    duplicate.Enchants[0].Stacks = 0;
    assert(!Area52Build::Decode(Area52Build::Encode(duplicate), sentinel));
    std::vector<std::uint8_t> oversized(Area52Build::MaxPacketBytes + 1);
    assert(!Area52Build::Decode(oversized, sentinel));
}
