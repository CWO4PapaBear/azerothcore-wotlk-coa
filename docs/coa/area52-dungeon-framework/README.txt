Area 52 shared dungeon framework integration

This package backports the shared vanilla dungeon foundation and three follow-up repairs to the existing Area 52 integration baseline. It is not a replacement server checkout or a claim that upstream main can reproduce the rest of the private Area 52 deployment. source-preconditions.json identifies the exact required file contents. integration.patch contains only this milestone's changes.

Upstream main reviewed: 0a0a21d3830e24dec61f16aeb013a16c748d5615
Foundation: https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/6477
Boss damage and healing: https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/7036
Dire Maul West: https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/7125
Anastari possession: https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/7039

Scope: Normal/Heroic/Mythic support for 19 classic dungeons, authored creature tiers, health targets, loot variants, encounter credit, boss spell kits, difficulty packet support, and the matching SQL tables. Mythic+ is not included.

Area 52 reconciliation:
- Five conflicting Stormwind training-dummy GUIDs are relocated, preserving positions and formation references. Upstream dungeon spawn GUIDs remain unchanged.
- Hearthsinger remains available on all difficulties. Landslide retains its 45-second stun cooldown.
- Existing loot rolls remain intact. Only eligible equipment is replaced by the matching tier. The upstream forced-one-item boss reward policy is not imported.
- New percent-damage execution, trash caps, damage multipliers and power exemptions are confined to the authored classic-dungeon scope. Player and player-owned casts do not acquire the new percent-damage behavior.
- Player-owned summons are excluded from authored dungeon health and level overrides.
- Additional non-Normal reset support is confined to classic dungeons; permanent binds and occupied-instance restrictions remain.
- Existing Area 52 progression, acquisition, achievements, collections, and Part 1/Parts 2-3 repairs are retained.

The upstream health reconstruction includes provisional values and inferred tuning. The integration does not establish historical live Ascension tuning. Native server tests do not certify the client UI or complete multiplayer dungeon clears.

Activation requires deploying the verified matching binary and all package SQL migrations together. Do not apply only the tables, do not replace the running binary in place, and do not treat this source publication as live activation or a launcher release. Use the established three-minute announcement/countdown for an authorized restart.

Verification and activation readiness:
- Final build and unit target: VERIFY ALL: PASSED.
- Five focused harnesses: VERIFY ALL: PASSED (levels, health, loot, difficulty packets, scoped percent damage).
- Six registered Area 52 HERO runtime scenarios, real clock: VERIFY ALL: PASSED. Normal/Heroic/Mythic entry, creature tiers, loot, health targets, and both group queues pass; no batch-sensitive or acceleration-sensitive cases.
- All 32 migration hashes match the isolated updater records. Five dummies retain their exact positions, and both formations retain all six members under the updated GUID references. The upstream dungeon GUIDs are unchanged.
- The four follow-up SmartAI migrations rewrite complete entry/source blocks, preserving the other actions.
- Full-repository source checks remain VERIFY ALL: FAILED: 14 suites pass, four fixture/tooling suites do not. See verification-summary.json. This is not a claim of a fully green repository-wide audit.
- Binaries and SQL are staged only. Source publication does not activate the realm or publish client release assets.
