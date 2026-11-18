/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#pragma once

#include "AscensionFreePick.h"
#include "AscensionWildcard.h"
#include "Config.h"
#include <string>

namespace Area52MysticRealm
{
inline bool UsesFreePickEngine()
{
    return sConfigMgr->GetOption<bool>("CoA.Enable", true)
        && sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero"
        && sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live"
        && !AscensionWildcard::PlaysWildcard(sConfigMgr->GetOption<std::string>("CoAChallenges.GameModes.Realm", ""))
        && !(sConfigMgr->GetOption<std::uint32_t>("CoA.GameModeMask", 0) & ~AscensionFreePick::QualityMode);
}
}
