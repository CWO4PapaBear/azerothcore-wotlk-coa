Combined Area 52 proc activation package

This cumulative patch supersedes the prior six-entry and nine-entry proc candidate patches; do not apply them twice. It targets the independent Area 52 reconstruction, not bare upstream main. It preserves unrelated runtime work and gates handlers to Area 52 Hero players. Reviewed upstream revision: fc359be9bf79ffb532c192d7afe754be21feba40; PTR comparison: e1823bb2db751a7cc0a90a8543e778449ebf7d84.

23 catalog entries (22 distinct talents, including both Wind Rush entries) qualify for yellow VERIFIED labels. A New Dawn's healing-ownership repair and Shadow Power's periodic proc repair are included, but their remaining baseline gaps prevent VERIFIED promotion. Thirteen further follow-up entries remain unresolved and NOT VERIFIED. Existing tooltip descriptions, SHIFT content and spacing are preserved.

VERIFY ALL: PASSED for build, unit and focused proc harness; tooltip preservation harness passed separately. Migration fixture checks cover idempotence and unrelated-row preservation. No simulated combat or reconnect claim; gameplay certification belongs to testers. Selection persistence and talent-rank acquisition remain on their already-reviewed native paths. No new permanent spell grants are introduced.

Activation uses a rollback-capable binary/configuration pair and the normal database updater restricted to the three included world migrations. Prior state and affected rows are retained for rollback. A three-minute in-game announcement/countdown is required. Client manifest promotion and Discord announcement are separate steps and must follow successful activation.

Activated after the owner-triggered three-minute shutdown on October 8, 2026. Normal updater recorded all three migration hashes; live row and running-binary verification passed. Primary client installed with backup. Client channel and Discord publication are tracked separately.
