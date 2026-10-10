# Hoplite build dependency repairs

Review of the saved Hoplite build covers 62 advancement entries and 9 Mystic Enchants.
All 71 entries are VERIFIED, adding 30 newly VERIFIED entries.
These are structural engineering results, not gameplay certification.

Repairs cover Execute rage/AP scaling, Devastate's two-handed weapon component, Deep Wounds damage
and conditional criticals, Relentless Strikes finisher rewards, Enrage and Honor Among Thieves proc
filters, Anger Management's Charge helper, Shattering Throw's fallback, Sweeping Swings and Spartan Kick.
Existing native handlers and durable ownership/book-rank paths are preserved where appropriate.

The final repairs cover Hunger for Blood bleed-dependent duration and movement speed, Critical Block additional remaining-damage reduction, Wind Slam consumption/slow exclusivity and Windfury chance, and Draconic Sinister Strike. Its own recovered SLS helper 782247 supplies a 50% proc chance; 782200/782201 supply 12-second spell/weapon critical buffs with shared 20-second cooldown 780999. The tooltip now explicitly states 50%. Client definitions are recovered evidence, not official backend or certified live behavior.

The incremental server build and five production-rule harnesses passed. The SQL migration passed
idempotence checks on connection-local temporary tables. Native isolated startup/export passed its
scenario; the exploratory wrapper reports INCOMPLETE because it is not a combat verification suite.
Subsequent contract assertions passed for bindings, proc entries, movement flags and 18 trainable
rank groups. Three stackable enchant groups are not book-trained ranks. Flurry of Spears is an
explicitly documented mobile channel. In-game behavior and reconnect remain tester certification.

The client changes 82 tooltip entries, preserving original SHIFT content except the requested explicit 50% chance wording. Client archive and structural contracts passed.

area52-runtime-repairs.patch is a focused delta against the existing Area 52 reconstruction,
not a drop-in upstream main integration. source-preconditions.json records LF-normalized source
hashes; the patch was applied to those baselines and the resulting hashes matched the tested files.
It depends on existing AscensionFreePick, Hoplite, Mystic ownership and spell-script-value APIs.
Audit harnesses additionally require the local effective-data export and client baseline.
Do not apply realm-specific SQL to another game-mode database. No upstream-ready PR is claimed.

Activated on Area 52 and released through the separate tester channel. Server startup and both migration hashes verified. Prior crash logs and pre-migration tables retained privately. Specialized training dummies remain pending. Gameplay remains tester certification.
