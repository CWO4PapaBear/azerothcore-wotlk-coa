#include "Area52CreationSummary.h"
#include <cassert>
int main()
{
    using namespace Area52Build;
    for (auto const& source: SummaryArchetypes)
    {
        if (!source.spells[0]) continue;
        std::string saved="Overview\n###AREA52_CREATION_SUMMARY\n1";
        for (auto spell:source.spells) if (spell) saved+=";"+std::to_string(spell)+":12:74657374";
        assert(ValidCreationSummary(saved,source.id));
        auto pos=saved.find(';')+1, end=saved.find(':',pos);
        saved.replace(pos,end-pos,"123456789");
        assert(!ValidCreationSummary(saved,source.id));
        assert(ValidCreationSummary(saved,source.id,[](unsigned id){return id==123456789;}));
        assert(!ValidCreationSummary(saved,source.id,[](unsigned){return false;}));
        assert(!ValidCreationSummary(saved+";123456789:12:74657374",source.id,[](unsigned){return true;}));
        assert(!ValidCreationSummary(saved,0,[](unsigned){return true;}));
        assert(!ValidCreationSummary(saved+"ff",source.id,[](unsigned){return true;}));
    }
}
