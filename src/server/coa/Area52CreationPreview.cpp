#include "Area52CreationPreview.h"
#include "Area52CreationPreviewDefaults.h"
#include "Area52BuildProtocol.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "QueryResult.h"
#include "Field.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <map>

namespace
{
bool DecodePreviewBuild(std::string const& hex, Area52Build::Entry& entry)
{
    if (hex.size() % 2 || hex.size() > Area52Build::MaxPacketBytes * 2)
        return false;
    std::vector<std::uint8_t> bytes;
    for (std::size_t i = 0; i < hex.size(); i += 2)
    {
        unsigned value = 0;
        auto const parsed = std::from_chars(hex.data() + i, hex.data() + i + 2, value, 16);
        if (parsed.ec != std::errc{} || parsed.ptr != hex.data() + i + 2)
            return false;
        bytes.push_back(static_cast<std::uint8_t>(value));
    }
    return Area52Build::Decode(bytes, entry);
}
}

void SendArea52CreationPreview(WorldSession* session)
{
    if (!session || session->IsBot() || !sConfigMgr->GetOption<bool>("CoA.FreePickBuilds.Enable", false)
        || !sConfigMgr->GetOption<bool>("CoA.Enable", true)
        || sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero"
        || sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live"
        || sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) != 0)
        return;
    auto result = CharacterDatabase.Query(
        "SELECT a.archetype_id,JSON_UNQUOTE(JSON_EXTRACT(r.document,'$.wire')) "
        "FROM area52_build_archetype a JOIN area52_build b ON b.id=a.build_id AND b.archived=0 "
        "JOIN area52_build_revision r ON r.build_id=a.build_id AND r.revision=a.revision AND r.category=8 "
        "JOIN area52_build_featured f ON f.build_id=a.build_id AND f.revision=a.revision "
        "ORDER BY a.archetype_id LIMIT 256");
    std::map<uint32, std::string> descriptions;
    if (result)
    {
        do
        {
            Area52Build::Entry entry;
            auto const fields = result->Fetch();
            if (!DecodePreviewBuild(fields[1].Get<std::string>(), entry) || entry.Category != 8)
            {
                LOG_ERROR("coa", "Character Creation preview rejected an invalid Featured revision");
                return;
            }
            descriptions.emplace(fields[0].Get<uint32>(), std::move(entry.Description));
        } while (result->NextRow());
    }
    std::vector<WorldPacket> packets;
    for (auto row : Area52Build::CreationPreviewDefaults)
    {
        auto const selected = descriptions.find(row[1]);
        if (selected != descriptions.end() && !Area52Build::ProjectCreationPreview(row, selected->second))
        {
            LOG_ERROR("coa", "Character Creation preview rejected malformed selection metadata");
            return;
        }
        WorldPacket packet(Area52Build::CreationPreviewOpcode, 112);
        for (auto value : row)
            packet << value;
        packets.push_back(std::move(packet));
    }
    for (auto const& packet : packets)
        session->SendPacket(&packet);
}
