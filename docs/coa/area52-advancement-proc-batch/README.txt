Area 52 advancement proc batch

Reviewed all 38 advancement entries initially blocked only by missing proc entries, against the primary Area 52 client, effective server data, current CoA upstream fc359be9bf79ffb532c192d7afe754be21feba40 and PTR e1823bb2db751a7cc0a90a8543e778449ebf7d84. The flags concealed additional handler gaps; review.json records each disposition. Wind Rush is represented by two catalog entries.

Complete repair candidates: Adaptive Defense, Fracture, Malice, Improved Power Word: Shield, Shredding Attacks and Empowering Frostbolt. Shadow Power's periodic vulnerability proc is repaired separately; its baseline critical modifier remains unresolved. The other 31 entries are not promoted. The six candidate entries now have yellow VERIFIED tooltip mappings staged across 22 ranks; the authoritative live status ledger remains unchanged pending paired activation.

The patch targets the independent Area 52 runtime reconstruction, not bare upstream main. Apply it to the runtime containing AscensionFreePick and AscensionMysticEnchant. It adds an Area 52-gated event/target/value handler and explicit positive-ID SQL bindings. Other modes are rejected by the handler. Install SQL only into the Area 52 world database; this is not a shared-mode migration. Existing spell ownership and trainer rank paths are unchanged. No player state is modified by the migration.

Verification: VERIFY ALL: PASSED for build, unit and area52_proc_batch harness. The harness checks allowed/rejected proc predicates and SQL replay idempotence/unrelated-row preservation using an in-memory SQL fixture. C++ and SQL style checks passed. This is not a live MySQL migration test, combat test or reconnect test. Gameplay remains tester certification. Source-wide verification was not run on the unrelated dirty runtime checkout. No activation or client release is implied by this source publication.

Tooltip preservation verification passed through verify_all.py: all existing descriptions, SHIFT markup, other DBC fields and archive members remain unchanged. Only the six entries are promoted in the candidate ledger. No client archive is included in this source branch.

Pending: paired server/client activation and tester release; remaining handler work is itemized in review.json. Planned restarts require an in-game announcement and three-minute countdown.
