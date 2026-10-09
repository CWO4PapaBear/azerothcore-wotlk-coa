# Area 52 free ability and talent resets

Removes currency checks and charges from individual advancement unlearning and both Reset All operations. Existing server combat, battleground, arena and death checks remain authoritative. Reset All retains the active build and disables Auto-Learn as before; essence refunds continue through the existing selection accounting.

Buff cleanup follows removed selections through rank chains, taught/linked abilities, effect-trigger spells, positive cast/hit/aura SQL links, and recorded aura-trigger provenance. It removes matching self-cast auras on the player while excluding effects supported by retained selections or active Mystic Enchant grants. Existing core unlearn/pet-aura/action-bar cleanup remains in place. No global purge of other players' buffs is introduced. Custom scripts that neither define a link nor record trigger provenance cannot be inferred automatically and require a specific report.

The client uses CONFIG_AREA52_FREE_RESETS to show no cost and bypass rune checks, retaining compatibility with the older paid server configuration. The Area52-FreePick-Client repository contains that Lua change and its regression tests. This patch targets the existing private Free Pick implementation, including the pending active-build stat patch; these dependencies are absent from upstream main. It is an integration patch, not a drop-in upstream PR.

Reviewed upstream revision f3f54bbf2a1566343a7ad177d51c47dd94584c5e; no new relevant unlearn changes in the newly fetched commits. The effective installed patch-B reset code was checked against the client source before replacement. Only its reset Lua member changes in the staged archive, and every archive member was read back.

VERIFY ALL: PASSED for server build and area52_free_resets harness, and separately for the Lua 5.1 area52_reset_client harness. Changed C++ codestyle passed. Reports in outputs/Area52_Free_Resets/verification-final/report.json and verification-client-r2/report.json. An initial client-test invocation failed due to an incorrect environment path; corrected invocation passed. In-game acceptance has not been performed. Place regression.py at apps/coa-tests/area52_free_resets/run.py to run it through tools/verify_all.py.

Server/client candidates are staged, not activated or promoted to the tester launcher. Server candidate also preserves the pending Active Build Primary Stat and Hoplite Hidden Chest changes.
