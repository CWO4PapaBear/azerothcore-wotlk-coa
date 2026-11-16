#ifndef AREA52_MYSTIC_TEST_ACCESS_H
#define AREA52_MYSTIC_TEST_ACCESS_H

#include <boost/property_tree/ptree.hpp>
#include <cstdint>
#include <charconv>
#include <cctype>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

namespace AscensionMysticEnchant
{
inline std::set<std::uint32_t> ParseTestAccessExcludedAccounts(std::string_view text)
{
    std::set<std::uint32_t> accounts;
    while (!text.empty())
    {
        auto const separator = text.find(',');
        auto token = text.substr(0, separator);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front())))
            token.remove_prefix(1);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back())))
            token.remove_suffix(1);
        std::uint32_t account = 0;
        if (token.empty())
            throw std::runtime_error("Empty Mystic Enchant test-access account exclusion");
        auto const parsed = std::from_chars(token.data(), token.data() + token.size(), account);
        if (parsed.ec != std::errc{} || parsed.ptr != token.data() + token.size() || !account)
            throw std::runtime_error("Invalid Mystic Enchant test-access account exclusion");
        accounts.insert(account);
        if (separator == std::string_view::npos)
            break;
        text.remove_prefix(separator + 1);
        if (text.empty())
            throw std::runtime_error("Trailing Mystic Enchant test-access account exclusion separator");
    }
    return accounts;
}

inline void AddVerifiedTestAccess(std::set<std::uint32_t>& known, std::set<std::uint32_t> const& access,
    std::set<std::uint32_t> const& excluded, std::uint32_t account)
{
    if (!excluded.contains(account))
        known.insert(access.begin(), access.end());
}

template <typename Supported>
std::set<std::uint32_t> VerifiedTestAccess(boost::property_tree::ptree const& report, Supported supported)
{
    if (report.get<unsigned>("schema") != 1)
        throw std::runtime_error("Unsupported Mystic Enchant verification schema");
    std::set<std::uint32_t> result;
    std::set<std::uint32_t> seen;
    for (auto const& child : report.get_child("entries"))
    {
        auto const& row = child.second;
        if (row.get<std::string>("kind") != "mystic")
            continue;
        auto const id = row.get<std::uint32_t>("id");
        if (!seen.insert(id).second)
            throw std::runtime_error("Duplicate Mystic Enchant verification entry");
        auto const status = row.get<std::string>("status");
        if ((status == "VERIFIED" || status == "CERTIFIED") && supported(id))
            result.insert(id);
    }
    return result;
}
}

#endif
