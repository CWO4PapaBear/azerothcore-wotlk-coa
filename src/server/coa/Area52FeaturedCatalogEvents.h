#ifndef AREA52_FEATURED_CATALOG_EVENTS_H
#define AREA52_FEATURED_CATALOG_EVENTS_H

#include "Area52CreationSummary.h"
#include "Area52FeaturedIncarnations.h"
#include "Area52FeaturedRewards.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace Area52Build
{
struct CatalogSelection
{
    FeaturedRewardItems Items{};
    FeaturedPreviewEffects Preview{}, Incarnations{};
    bool AssignedItems = false;
    bool Valid = false;
    bool operator==(CatalogSelection const&) const = default;
};

inline CatalogSelection ReadCatalogSelection(std::string_view description)
{
    CatalogSelection value;
    value.AssignedItems = description.find("###AREA52_FEATURED_VANITY\n") != std::string_view::npos;
    value.Valid = ReadFeaturedRewards(description, value.Items)
        && ReadFeaturedPreview(description, value.Preview)
        && ReadFeaturedIncarnations(description, value.Incarnations);
    return value;
}

inline bool CatalogSelectionChanged(std::string_view before, std::string_view after)
{
    auto const left = ReadCatalogSelection(before);
    auto const right = ReadCatalogSelection(after);
    return !left.Valid || !right.Valid || left != right || SummaryLine(before) != SummaryLine(after);
}

inline bool WriteCatalogSignal(std::filesystem::path const& path)
{
    static std::uint64_t sequence = 0;
    auto temporary = path;
    temporary += ".pending";
    std::ofstream output(temporary, std::ios::trunc);
    output << std::chrono::system_clock::now().time_since_epoch().count() << '-' << ++sequence << '\n';
    output.flush();
    bool const written = output.good();
    output.close();
    if (!written || output.fail())
        return false;
    std::error_code error;
    std::filesystem::rename(temporary, path, error);
    return !error;
}

class CatalogSignal
{
public:
    void Configure(bool enabled, std::string path)
    {
        Enabled = enabled;
        Path = std::move(path);
        if (Ready && Enabled)
            Request();
    }

    void Start()
    {
        if (!Ready)
        {
            Ready = true;
            Request();
        }
    }

    void Request()
    {
        if (Enabled)
            Pending = true;
    }

    template<class Writer> int Pump(std::uint64_t now, Writer writer)
    {
        if (!Ready || !Enabled || !Pending || now < RetryAt)
            return 0;
        if (!writer(Path))
        {
            RetryAt = now + 60000;
            return -1;
        }
        Pending = false;
        RetryAt = 0;
        return 1;
    }

private:
    bool Ready = false;
    bool Enabled = false;
    bool Pending = false;
    std::uint64_t RetryAt = 0;
    std::string Path;
};
}
#endif
