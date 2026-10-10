# Area 52 certification leaderboard

This extension depends on the tester-certification and certification-rewards packages in the sibling
directories. Apply `leaderboard.patch` to the matching Area 52 runtime source and deploy
`TesterCertification.lua` at `Interface/AddOns/Area52MysticRules/TesterCertification.lua`.
The patch is a deployment-source extension, not a standalone upstream-main implementation.
`source-preconditions.json` records the expected source before and after the patch.

Players open the movable window with `/a52leaderboard`. Ten accounts appear per page, with Previous,
Next and Refresh controls. Each row shows rank, a contributor character name and one total spanning
abilities, talents and Mystic Enchants across all of that account's characters. Account login names
and account IDs are not transmitted. The current account is marked `(you)` on its row.

The server rebuilds scores from persisted, fully confirmed submissions at startup and adds successful
new certifications to its cache. Existing first-certifier credit rules remain unchanged: repeat
confirmations do not award more points, and manually certified labels without attribution do not
invent credit. Accounts with no submissions are not listed. The most recently credited character name
identifies the account; ties share a rank, with account IDs used only for stable internal ordering.
Deleted characters do not remove historical credit. No new tables or recurring polls are required.
Opening or refreshing fetches a page through the existing realm-scoped, rate-limited certification
protocol. The existing two-second request limit also applies to leaderboard requests.

Verification: incremental worldserver build and certification rule/reward regressions passed through
`verify_all.py`. Lua 5.1 harnesses passed for leaderboard paging, self identification, empty responses,
timeouts, server-message sender validation and the existing ME certification flow. This verifies
structure and mocked UI behavior; visual acceptance in the actual client remains pending.
The focused package reconstruction and publication source checks also passed. The C++ style check
reported pre-existing repeated blank lines at lines 67–68 of the shared `CharacterDatabase.cpp`;
the unchanged preimage contains the same whitespace. Other selected style checks passed.

This package is not activated or distributed to testers by its source publication. No restart is
performed. The existing server and launcher release continue to run the prior certification package.
