void LibraryPublicationStatus(WorldSession* session, std::shared_ptr<SessionState> state,
    std::string token)
{
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_LIBRARY_PUBLICATION);
    query->SetData(0, session->GetAccountId());
    query->SetData(1, token);
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, state, token](PreparedQueryResult rows)
        {
            if (!Current(session, state) || !LibraryEnabled)
                return;
            LibraryReply(session, "PUBLISH|" + token + "|"
                + (rows ? rows->Fetch()[0].Get<std::string>() : "unconfirmed"));
        }));
}

void LibraryPublish(WorldSession* session, std::shared_ptr<SessionState> state,
    std::string token, uint64 id, uint64 updated, std::string alias)
{
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_BUILD);
    query->SetData(0, id);
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, state, token, id, updated, alias](PreparedQueryResult rows)
        {
            if (!Current(session, state) || !LibraryEnabled)
                return;
            Entry entry;
            if (!rows || rows->Fetch()[0].Get<uint32>() != session->GetAccountId()
                || !ReadRecord(rows->Fetch(), entry) || entry.Updated != updated || Validate(entry))
            {
                LibraryReply(session, "PUBLISH|" + token + "|rejected");
                return;
            }
            auto* insert = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_LIBRARY_PUBLICATION);
            insert->SetData(0, token);
            insert->SetData(1, alias);
            insert->SetData(2, id);
            insert->SetData(3, session->GetAccountId());
            insert->SetData(4, rows->Fetch()[1].Get<uint32>());
            insert->SetData(5, rows->Fetch()[2].Get<std::string>());
            auto transaction = CharacterDatabase.BeginTransaction();
            transaction->Append(insert);
            session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(transaction)).AfterComplete(
                [session, state, token](bool success)
                {
                    if (!Current(session, state))
                        return;
                    if (success)
                        LibraryPublicationStatus(session, state, token);
                    else
                        LibraryReply(session, "PUBLISH|" + token + "|unconfirmed");
                });
        }));
}
