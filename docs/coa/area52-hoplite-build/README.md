# Hoplite build dependency repairs

Review of the saved Hoplite build covers 62 advancement entries and 9 Mystic Enchants.
The staged ledger contains 67 VERIFIED entries and 4 NOT VERIFIED entries, adding 26 VERIFIED entries.
These are structural engineering results, not gameplay certification.

Repairs cover Execute rage/AP scaling, Devastate's two-handed weapon component, Deep Wounds damage
and conditional criticals, Relentless Strikes finisher rewards, Enrage and Honor Among Thieves proc
filters, Anger Management's Charge helper, Shattering Throw's fallback, Sweeping Swings and Spartan Kick.
Existing native handlers and durable ownership/book-rank paths are preserved where appropriate.

Remaining exceptions are Hunger for Blood, Critical Block, Draconic Sinister Strike and Wind Slam.
Their specific missing contracts are recorded in audit.json. No unsupported chance, duration or
replacement rule was invented to promote them. Current upstream and the HERO FREE PICK PTR were
fetched and pinned; their alternative handlers do not supply those complete Area 52 contracts.
Client definitions are evidence from the recovered client, not verified live Ascension behavior.

The incremental server build and five production-rule harnesses passed. The SQL migration passed
idempotence checks on connection-local temporary tables. Native isolated startup/export passed its
scenario; the exploratory wrapper reports INCOMPLETE because it is not a combat verification suite.
Subsequent contract assertions passed for bindings, proc entries, movement flags and 18 trainable
rank groups. Three stackable enchant groups are not book-trained ranks. Flurry of Spears is an
explicitly documented mobile channel. In-game behavior and reconnect remain tester certification.

The staged client changes 76 status labels to yellow VERIFIED. Archive comparison confirms all
other tooltip/SHIFT content, DBC fields and archive files are unchanged. The candidate ledger and
client archive remain local pending activation; proprietary client data is not included here.

area52-runtime-repairs.patch is a focused delta against the existing Area 52 reconstruction,
not a drop-in upstream main integration. source-preconditions.json records LF-normalized source
hashes; the patch was applied to those baselines and the resulting hashes matched the tested files.
It depends on existing AscensionFreePick, Hoplite, Mystic ownership and spell-script-value APIs.
Audit harnesses additionally require the local effective-data export and client baseline.
Do not apply realm-specific SQL to another game-mode database. No upstream-ready PR is claimed.

Source publication does not activate a server, install a client, or promote a tester channel.
No live server changes, restart, client installation or tester release occurred in this batch.
The specialized training dummies remain pending.

Investigation and verification took approximately one hour. Initial checks caught an include path
error and a SQL tuple error, both repaired before the final passing checks. The broad rank assertion
was narrowed to trainable ranks rather than stackable ME copies; the channel assertion now records
the documented Flurry exception. A Windows temporary-directory permission failure was resolved by
using the task workspace. No inconsistent outcomes on unchanged tested inputs were observed;
no human corrections were required during this batch.
