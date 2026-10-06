# Area 52 alpha Mystic Scrolls

This focused integration patch targets the existing Area 52 Mystic Enchant implementation. That implementation is preserved in the runtime checkout but is not yet present in upstream main. Do not apply the SQL alone or present this branch as a merge-ready upstream implementation.

Apply with `git apply --check` followed by `git apply` in the Area 52 integration checkout. The patch includes the handler, pending world migration and focused regression harness. Verify through `tools/verify_all.py` with `--harness area52_mystic_scroll mystic_enchant_catalog`; compile the candidate before deploying. Back up the affected world tables and character database. The reserved fallback loot ID 1978660000 must be unused before first deployment.

Common Unidentified Mystic Scroll 97866 drops once per normal creature loot generation at 25 percent. Existing shared loot IDs receive one row each; lootless templates use a shared fallback. Normal lootability rules still apply. The handler gates drops to active Free Pick Mystic Enchants and uses its own roll, avoiding global quality-rate multipliers. Set `CoA.FreePickMysticEnchants.ScrollDropChance` between 0 and 100 to tune it; alpha default is 25.

Each use uniformly selects one unique spell from the active catalog, including already collected and not-yet-implemented enchants. There is no rarity weighting or duplicate protection. The account-wide unlock and scroll consumption share a character-database transaction; failed commits retain the item. The handler waits for database commit before updating in-memory inventory, so database latency can delay the world thread while consuming a scroll. Gameplay testing and a later asynchronous persistence refinement remain distinct from automated verification.

Validation: full binary build and unit test passed; catalog, focused handler, syntax, combo-point and Victory Rush regressions passed. SQL dry-run on temporary copies covered 32066 creature templates and 7837 distinct loot tables with no missing or duplicate scroll entries. No gameplay result is claimed.
