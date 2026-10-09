# Area 52 Battlemage

Battlemage 81258 had a dummy aura but no proc row or execution handler. The repair triggers one of the owner's known Arcane Barrage, Fire Blast or Ice Lance families from direct Warrior ability damage, selecting the highest learned rank. It excludes replacement auto-attacks and periodic damage, enforces the described four-second cooldown, and reduces only Battlemage-triggered damage to player-controlled targets by 20%. Ordinary casts retain their existing damage.

The catalog accepts this scripted enchant only when its root proc handler and every applicable damage rank handler exist. No abilities are newly granted: selections continue to come from Advancement and its approved sources. Existing slot ownership and persistence are reused. Area 52 player gating isolates the handler from other game modes.

## Research

Reviewed independent reconstruction jealous-sound/azerothcore-wotlk-coa main at 09da7d129571fc375780de5ebf70a4220e9f36f1 and cache repository hertigservices/ascension-data at 09c361f8b4d51c8a2d6ce84165ca53988d429335. Upstream explicitly identifies Wildcard as Season 10; no separate Season 10 Free Pick backend was identified. No matching Battlemage implementation was found in reviewed upstream main. The primary Area 52 client description supplies this contract; no claim of official backend equivalence is made.

## Verification and deployment

Build, unit and focused production-rule/SQL harness: VERIFY ALL: PASSED. Tests cover the eligibility truth table, highest-known-rank selection, unknown families, cyclic rank data, 20% reduction including arithmetic overflow protection, and repeated SQL application preserving unrelated bindings. C++ and SQL style checks pass.

All 19 relevant client ranks are instant and non-channelled. Triggered casts retain native spell behavior; no cast-time flags are rewritten. No new spellbook grants or rank ladders are introduced.

This is a focused source patch against the preserved private Area 52 integration, not a standalone upstream-main cherry-pick. It is staged, not live. Native binding audit results are recorded separately; combat and reconnect certification are not claimed. No tester release or restart is included.

Native isolated export: Battlemage reports connected=true, with its root handler, four-second proc cooldown, and all 19 rank damage bindings present. The exploratory export scenario passed (1/1); the overall gameplay runner correctly reports incomplete because no gameplay certification scenarios were requested. This is structural evidence only.
