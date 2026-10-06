# Area 52 Hoplite

The Area 52 implementation reconstructs the Hoplite chain from the effective client/server definitions. Upstream `jealous-sound/azerothcore-wotlk-coa` was fetched and reviewed at `041748106d507314b12671b5597a7eaa447ac046`; this is an independent reconstruction, not verification against Ascension's original backend. The client reference is the owner's effective Area 52 patch-D overlay. No client archive or DLL is included.

## Combat contract

- Shield Strike (978533 and 300158) uses the owner-selected hybrid: `18 + 221 * CP + (0.82 + 0.04 * CP) * AP`, with CP capped at five and integer truncation before ordinary damage modifiers. One successful damaging hit reduces Bloodrage's cooldown by exactly two seconds per point and rolls once at 15% for Spear Mastery.
- Thrust grants both Phalanx Strike helpers while Bloodrage's periodic aura and Phalanx Stance are active. The existing five-stack limit remains. Each stack grants 20% Thrust damage, five less energy cost, and 10% Shield Strike damage. Refreshing Bloodrage clears both helpers. Unsupported private damage modifier 38 is translated to the native damage operation on those exact helper effects; the ordinary energy modifier is retained.
- Javelin keeps its native base damage and bleed values. Its direct hit ignores armor. A successful damaging hit applies the caster's Impaled; Colossus Smash consumes only that caster's Impaled and resets Javelin. Each elapsed whole second adds 20% damage against unowned creatures or 5% against players/player-controlled units, capped at the bleed's twelve seconds. The malformed native Skewered modifier path is suppressed so it cannot double the bonus or affect another caster.
- Flurry retains its native seven ticks, cone, movement slow, level/combo-point base values and Siegebreaker-family damage modifiers. The total added scaling is `(AP + SP) * (0.135 + 0.027 * CP * CP)`, divided over seven ticks. Physical-school spell power is used. The forwarded child receives its parent's amount without adding a second level contribution or random roll. Spear Mastery is consumed when the channel applies and grants 50% more damage to unowned creatures for that channel, not to players or their pets. Interruption clears the snapshot.

## Equipment, learning and scope

The scripts require the Hero/live configuration, Free Pick-only/zero game-mode mask, and Hoplite aura 978217. Battle Stance grants Phalanx Stance. Known ranks of Shield Slam, Sinister Strike, Heroic Strike and Siegebreaker receive temporary replacements; the original spells remain learned. The reviewed replacement IDs are level-scaling spells rather than newly invented rank chains. Active casts require Battle Stance and a polearm/shield pair.

Removing Hoplite clears its temporary replacements and stance/stack/channel helpers and invokes native offhand inventory cleanup. Logout skips this removal. Area 52's independent collection-service integration is preserved separately in `integration/area52-hoplite-runtime.patch`: it validates account ownership, saved slots, level and investment before restoring Hoplite ahead of inventory/action-bar loading. Other classes retain the original character-settings load timing. The patch requires the existing Area 52 collection schema/service and Spiritual Attunement service patch; it is not a generic upstream database migration.

The exact Tactical Mastery, Iron Will, Two-Handed Weapon Mastery, Weapon Command and Talonshrike bonus helpers are suppressed while Hoplite is active and can resume through native aura target reevaluation when it is removed. Fury of the Eagle is refused while Hoplite is active. The underlying learned talents and Deflection's ordinary parry/stamina effects are preserved. Unsupported unrelated dummy capstones are not implemented by this change. Other modes do not load these compatibility scripts.

The ME service admits Hoplite only when its required combat/aura bindings are available. Source publication, server activation, client release and in-game acceptance are separate operations.

## Validation

The focused harness extracts production methods for Shield Strike, Flurry scaling, Impaled timing, Thrust/Bloodrage stacks, successful Javelin hits, caster-owned Colossus consumption, and prerequisite replacement cleanup. These are method-level regressions, not a complete engine simulation.

Source checks and C++/SQL lint pass. The final candidate passed `verify_all` build, unit and six focused harness stages (`VERIFY ALL: PASSED`). The worldserver SHA-256 is `0c81bbc2092062874470d8cf812844f10b9da374149fc2f02ee586b020a58cb0`.

The isolated real-clock native scenario passed 34 assertions: collection admission, polearm/shield equipment, four replacements, stance, three Thrust stacks, Bloodrage reset, Javelin/Impaled consumption, cooldown reset, seven positive Flurry hits, Spear Mastery consumption, and incompatible-aura suppression/restoration. Its definition is preserved in `integration/area52-hoplite-connections.json`. This is an exploratory scenario, not a registered combined-verification case: the gameplay aggregate correctly reports `VERIFY ALL: INCOMPLETE` / unavailable combined verification, with one native scenario passed and zero failed. It has no acceleration-sensitive or batch-sensitive results. Do not describe that aggregate as a full-suite pass.

Local verification reports are under `outputs/Area52_MysticEnchant_Review`: `eldritch/verification-hoplite-complete-r2/report.json`, `hoplite/verification-source/report.json`, and `hoplite/gameplay-20261006-113508/report.json`. These reports refer to the assembled Area 52 runtime, including the separate service integration patch. Reconnect, client equipment/action-bar persistence, rendered effects, exact combat totals and interrupted channels remain in-game acceptance checks. No running realm has been changed by this milestone.
