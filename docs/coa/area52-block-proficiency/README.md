# Area 52 Block proficiency

Area 52 Heroes received Shield proficiency (9116), but not the separate passive Block (107). Native block percentage calculation discards both base chance and modifiers while CanBlock is false. Read-only character checks confirmed Shield without Block on the sampled Heroes.

The patch grants permanent Block through an explicit learning scope at login, only when AscensionFreePick::Applies is true. Existing owners are unchanged. Successful grants are saved. Native shield, facing, attack eligibility and block chance calculations remain intact. No client or SQL changes.

## Scope and provenance

Reviewed jealous-sound/azerothcore-wotlk-coa main at 6b853197b41e13a744334a7e3f7e0a432ad23cb8. No matching Area 52 baseline repair was found. Existing upstream scenarios explicitly grant both 107 and 9116 for block tests. This is an independent Free Pick integration, not evidence of official Ascension behavior.

Effective primary client Spell.dbc SHA256: dc176384681d2efb8b1a772d2a17eb5a0090ac6f827c20b18542fce0d62e8b65. Spell 107 is passive with SPELL_EFFECT_BLOCK; 9116 grants equipment proficiency. Server Spell.dbc SHA256: 5e85f10ccb861e8463e838caaa46630050bd8f6b73bc5ddf6378eb879f69c263, with the same Block definition; no SQL override row for these IDs.

The repair patch is against the private Area 52 integration with its pending trainer/learning guard changes. It is not a standalone upstream-main cherry-pick. It excludes unrelated staged work and client data.

## Verification

VERIFY ALL: PASSED (build, unit, focused area52_block_proficiency harness). The harness compiles the production grant function and Player::UpdateBlockPercentage with boundary fixtures: zero before grant; 5% base after; 33% with 25 aura points and 3 rating points; non-Area-52 isolation; null/missing definition safety; idempotent permanent authorized grant and login save wiring. Both changed C++ files pass the C++ style checker.

Gameplay/reconnect certification was not run. No claim is made that every block talent or enchant is certified. No flaky results observed; the first repeat was rejected because its output directory already existed, then ran successfully in a new directory. No live server restart or client release was performed.
