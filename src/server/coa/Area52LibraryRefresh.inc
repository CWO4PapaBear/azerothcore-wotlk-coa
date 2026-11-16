std::atomic<bool> LibraryEnabled{false};

void LibraryReply(WorldSession* session, std::string const& message)
{
    WorldPacket packet;
    Player* player = session->GetPlayer();
    ChatHandler::BuildChatPacket(packet, CHAT_MSG_WHISPER, LANG_ADDON, player, player,
        "A52LIB\t" + message);
    session->SendPacket(&packet);
}

void LibraryStatus(WorldSession* session, std::shared_ptr<SessionState> state, std::string token)
{
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_LIBRARY_REFRESH);
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, state, token](PreparedQueryResult rows)
        {
            if (!Current(session, state))
                return;
            if (!rows)
            {
                LibraryReply(session, "ERROR|" + token + "|unavailable");
                return;
            }
            Field* fields = rows->Fetch();
            LibraryReply(session, "UPDATE|" + token + "|" + std::to_string(fields[0].Get<uint64>()) + "|"
                + fields[1].Get<std::string>() + "|" + std::to_string(fields[2].Get<uint32>())
                + "|" + std::to_string(fields[3].Get<uint32>()) + "|"
                + std::to_string(fields[4].Get<uint32>()));
        }));
}

#include "Area52LibraryPublication.inc"

class LibraryRefreshPlayer final : public PlayerScript
{
public:
    LibraryRefreshPlayer() : PlayerScript("Area52LibraryRefreshPlayer",
        { PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT, PLAYERHOOK_ON_LOGOUT }) { }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& message,
        Player* receiver) override
    {
        if (type != CHAT_MSG_WHISPER || language != LANG_ADDON || !message.starts_with("A52LIB\t"))
            return true;
        if (receiver != player)
            return false;
        WorldSession* session = player->GetSession();
        std::string const payload = message.substr(7);
        auto const separator = payload.find('|');
        if (separator == std::string::npos)
            return false;
        std::string const command = payload.substr(0, separator);
        auto const arguments = payload.find('|', separator + 1);
        std::string const token = payload.substr(separator + 1,
            arguments == std::string::npos ? arguments : arguments - separator - 1);
        if (token.empty() || token.size() > 24 || token.find_first_not_of("0123456789abcdef") != std::string::npos)
            return false;
        if (!Enabled || !LibraryEnabled || !AscensionFreePick::Applies(player) || session->IsBot())
        {
            LibraryReply(session, "ERROR|" + token + "|unavailable");
            return false;
        }
        if (command != "UPDATE" && command != "STATUS" && command != "PUBLISH" && command != "PSTATUS")
        {
            LibraryReply(session, "ERROR|" + token + "|unsupported");
            return false;
        }
        std::shared_ptr<SessionState> state;
        {
            std::lock_guard lock(StateLock);
            auto found = Sessions.find(session->GetAccountId());
            if (found == Sessions.end() || found->second->Owner != session)
                return false;
            auto const now = Clock::now();
            auto& next = NextRequest[session->GetAccountId()];
            if (now < next)
                return false;
            next = now + std::chrono::seconds(2);
            state = found->second;
        }
        if (command == "PSTATUS")
        {
            if (arguments == std::string::npos)
                LibraryPublicationStatus(session, state, token);
            return false;
        }
        if (command == "PUBLISH")
        {
            if (arguments == std::string::npos)
                return false;
            auto const fields = payload.substr(arguments + 1);
            auto const first = fields.find('|');
            auto const second = fields.find('|', first == std::string::npos ? first : first + 1);
            auto const third = fields.find('|', second == std::string::npos ? second : second + 1);
            if (first == std::string::npos || second == std::string::npos || third == std::string::npos)
                return false;
            uint64 id = 0, updated = 0;
            auto const alias = fields.substr(second + 1, third - second - 1);
            if (!Id(fields.substr(0, first), id) || !Id(fields.substr(first + 1, second - first - 1), updated)
                || alias.empty() || alias.size() > 48
                || alias.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 _-.") != std::string::npos
                || fields.substr(third + 1) != "CC0-1.0")
            {
                LibraryReply(session, "PUBLISH|" + token + "|rejected");
                return false;
            }
            LibraryPublish(session, state, token, id, updated, alias);
            return false;
        }
        if (arguments != std::string::npos)
            return false;
        if (command == "STATUS")
        {
            LibraryStatus(session, state, token);
            return false;
        }
        auto transaction = CharacterDatabase.BeginTransaction();
        transaction->Append(CharacterDatabase.GetPreparedStatement(CHAR_UPD_A52_LIBRARY_REFRESH));
        session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(transaction)).AfterComplete(
            [session, state, token](bool success)
            {
                if (!Current(session, state))
                    return;
                if (success)
                    LibraryStatus(session, state, token);
                else
                    LibraryReply(session, "ERROR|" + token + "|unavailable");
            });
        return false;
    }

    void OnPlayerLogout(Player* player) override
    {
        std::lock_guard lock(StateLock);
        NextRequest.erase(player->GetSession()->GetAccountId());
    }

private:
    std::unordered_map<uint32, Clock::time_point> NextRequest;
};
