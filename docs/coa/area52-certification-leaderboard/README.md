# Area 52 certification leaderboard

This extension depends on the tester-certification and certification-rewards packages in the sibling
directories. Apply `leaderboard.patch` to the matching Area 52 runtime source and deploy
`TesterCertification.lua` at `Interface/AddOns/Area52MysticRules/TesterCertification.lua`.
Install `BearCaveIcon.tga` as `Interface/Glues/BearCave/CertificationIcon.tga`, within the launcher's
existing managed graphics paths. It is a 128px RGBA TGA conversion of the existing
Bear Cave launcher app icon, with source and output hashes in `icon-source.json`.
The panel uses the launcher's navy, gold and pale-blue palette with its paw emblem in the top left.
The patch is a deployment-source extension, not a standalone upstream-main implementation.
`source-preconditions.json` records the expected source before and after the patch.

The movable window opens automatically after login certification sync, in the party-portrait area.
It stays open at BACKGROUND strata, behind party portraits, and does not close on Escape.
`/a52leaderboard` opens its first page. Ten accounts appear per page, with Previous,
Next and Refresh controls. Each row shows rank, the last logged-in character name and one total spanning
abilities, talents and Mystic Enchants across all of that account's characters. Account login names
and account IDs are not transmitted. The current account is marked `(you)` on its row.

The server rebuilds scores from persisted, fully confirmed submissions at startup and adds successful
new certifications to its cache. Existing first-certifier credit rules remain unchanged: repeat
confirmations do not award more points, and manually certified labels without attribution do not
invent credit. Accounts with characters remain listed at zero; accounts without characters are excluded.
Ties share a rank, with account IDs used only for stable internal ordering.
Deleted characters do not remove historical credit. The characters migration creates a small table
recording the last logged-in character per account. Before its first tracked login, character selection
uses existing online/logout activity and then character ID as a deterministic fallback. Current names
are read from the character table, so renames are reflected and deleted characters are not displayed.
The auth query reads only account IDs, never login names. No recurring polls are required.
Opening or refreshing fetches a page through the existing realm-scoped, rate-limited certification
protocol. The existing two-second request limit also applies to leaderboard requests.
Live certification broadcasts schedule a refresh after three seconds; overlapping events are coalesced.

Verification: incremental worldserver build and certification rule/reward regressions passed through
`verify_all.py`. Lua 5.1 harnesses passed for leaderboard paging, self identification, empty responses,
timeouts, server-message sender validation and the existing ME certification flow. This verifies
structure and mocked UI behavior; visual acceptance in the actual client remains pending.
The focused package reconstruction and publication source checks also passed. The C++ style check
reported pre-existing repeated blank lines at lines 67–68 of the shared `CharacterDatabase.cpp`;
the unchanged preimage contains the same whitespace. Other selected style checks passed.

The package was activated on Area 52 after the announced three-minute shutdown. The migration hash,
server readiness and unchanged protected services were verified. Matching client assets were installed
locally with backup and released through the existing launcher channel after validation. Source
publication alone does not activate other deployments. In-game visual acceptance remains pending.
