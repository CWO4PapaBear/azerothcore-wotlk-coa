# Shared Prestige eligibility

Chromie 178081 is bound to npc_coa_prestige_chromie in the effective Area 52 world database. The rejection came from ActivationFacts.customClass in mod-coa-prestige, not a client or realm advertisement flag. Upstream 413e03e986ad5d9a3e1d95f88af7effe39972b4f still admits CoA classes and Wildcard Heroes only. This repository is an independent reconstruction, not Ascension's official backend.

Remove the class eligibility gate. Keep the enable setting, minimum level, current-cycle, specialization, alive, combat, travel and open-world checks. CoA characters still choose a CoA specialization; other characters use their current specialization slot. Existing Wildcard and CoA reset paths remain mode-specific. Free Pick clears its advancement selection through a fee-free reset before level reduction; standard classes reset talents without a fee. Existing Prestige rewards and quest handling remain in place.

Client reviewed: the primary Area 52 client's patch-B PrestigeMode.lua, SHA-256 d2295ae54dbbcee567ada0709f055f8b026ba9afb685a87b637105597a4099bf. Its confirmation already describes returning to level 1 and purging abilities/talents. No client patch is required to remove the server rejection.

Validation: Area 52 runtime build, unit stage and advancement harness passed. The publication checkout has an older Freepick API, so ResetForPrestige is adapted to that implementation; it preserves the newer upstream CoA replacement-rank cleanup. Windows source checks passed nine suites but failed the unchanged gameplay runner's crash/requeue tests (missing fixture summaries and not-run lanes); this is an unresolved verification limitation, not a passing overall source check. No upstream PR or live activation is included.

The first native run passed standard-class activation. The Free Pick fixture exposed a separate runtime pricing regression before reaching Chromie: HasItemCount(item, 0) returns false without that item. The runtime repair checks funds only when a nonzero cost is due, preserving free learning and fee-free Prestige. The obsolete run was stopped after recording both case results; the corrected Free Pick case is rerun independently. This additional guard is specific to the current runtime implementation, not the publication checkout's older CheckApply pricing implementation.

This focused branch continues the existing Free Pick integration history; it has not been prepared as a clean upstream PR.
