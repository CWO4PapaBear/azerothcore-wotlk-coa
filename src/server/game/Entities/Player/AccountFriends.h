#ifndef ACCOUNT_FRIENDS_H
#define ACCOUNT_FRIENDS_H

#include "SocialMgr.h"

namespace AccountFriends
{
    struct Entry
    {
        ObjectGuid OriginalGuid;
        std::string OriginalName;
        std::string Note;
    };

    using Friends = std::map<uint32, Entry>;
    uint32 Account(ObjectGuid guid);
    Friends const* Get(uint32 account);
    bool Has(ObjectGuid owner, ObjectGuid target);
    bool Add(ObjectGuid owner, ObjectGuid target);
    void Remove(ObjectGuid owner, ObjectGuid target);
    void SetNote(ObjectGuid owner, ObjectGuid target, std::string note);
    void SendMetadata(Player* player, std::map<ObjectGuid, FriendInfo> const& contacts);
}

#endif
