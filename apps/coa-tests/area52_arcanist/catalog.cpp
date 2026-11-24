#include "AscensionMysticEnchantCatalog.h"
#include <cassert>
int main(int argc,char** argv)
{
    using namespace AscensionMysticEnchant;
    assert(argc==2);
    Catalog catalog;
    assert(LoadCatalog(catalog,argv[1]));
    auto const& entry=catalog.BySpell.at(984117);
    assert(entry.Id==132434 && entry.Item==200015 && entry.Quality==5);
    assert(!catalog.BySpell.contains(271284) && !catalog.BySpell.contains(1568559));
    Slots slots{};
    slots[0]=984117;
    std::set<std::uint32_t> owned{984117};
    std::map<std::uint32_t,std::uint32_t> connected;
    assert(!ValidateSlots(catalog,slots,80,owned,connected));
    connected[984117]=1;
    assert(ValidateSlots(catalog,slots,80,owned,connected));
    assert(!ValidateSlots(catalog,slots,80,{},connected));
    slots[1]=984117;
    assert(!ValidateSlots(catalog,slots,80,owned,connected));
    slots[0]=0;
    assert(!ValidateSlots(catalog,slots,80,owned,connected));
}
