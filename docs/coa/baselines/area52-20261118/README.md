# Area 52 merged Development catch-up

This PR brings the existing fork forward from the accepted baseline. The merge preserves upstream ancestry and the qualified Area 52 resolution tree. It is not a deployment or a new server repository.

The baseline includes reconciled migrations, Free-Pick behavior, pilot duplicate cleanup and Sister Alma's quest script. Area 52 starter quests, trainers and routes remain preserved. Follow-ups include all-rank Dark Ranger bindings, authenticated bug-report names, Defias repairs, Callboard reward floors and staggered bot wake-up.

## Reproduction

Use the original `../area52-20261115` instructions and ordered migrations first. Apply `playerbots-followup.patch` to its pinned patched module tree and verify `playerbots.json`. Exact patch replay matched the current module tree, including Bulwark equipment and staggered login. Population and enabled configuration are unchanged.

`followup-migrations.json` pins subsequent world SQL. Reconcile deployed ledgers before selecting unapplied steps. The qualified Development package retains guarded apply/rollback and before-images privately; their hashes are recorded here. Do not blindly apply historical rollback to a newer realm.

`verified-access.json` is the exact Development runtime catalog, including existing certification flags. Include it when staging; omission previously suppressed enchant grants. Preserve newer Cloud certification changes and account exclusions during promotion. This catalog does not replace player certification records.

## Verification

`source-inputs.json` records exact parity of all 20 pending runtime/SQL inputs with the qualified candidate. `verification.json` pins passing native build/unit, disconnected pre-gameplay startup and SQL apply/reapply/rollback reports. Matching existing results are reused; no new gameplay was run. Twelve portable harnesses passed again in this checkout. DBC checks require `dbc_directory` in runner settings; proprietary data stays external.

Machine-bound fixture runners remain in the private workspace and original candidate; `test-scope.json` identifies them separately. Original merge and private recovery archives remain untouched.

## Release state

The owner is testing the combined Defias Development package. This PR does not assert acceptance or Cloud deployment. Client tooltip archives, certification UI and Development activation runner remain in the separate client repository/package and require linked release receipts during approved promotion. No server restart, launcher publication or Discord delivery is part of this PR.

The Shaman action lock after a rejected cache remains unresolved. Promote exact reviewed artifacts and migrations after Development acceptance, preserving Cloud player state and certifications; see AGENTS.md.

## Final offline review — November 18

The remaining review blockers are repaired. Six missing upstream tool/test files are restored byte-for-byte. Native fixtures now honor reaction, stationary and spell-hit options with upstream defaults. C++ lint and source policy pass; literal strings no longer cause false double-semicolon reports. Historical SQL is preserved through exact Git-blob validation rather than rewritten for lint.

The final native rebuild linked both worldserver and unit_tests. Native units report 6,364 passed, zero failed, 5,486 inapplicable parameter cases skipped and one existing ASan-only negative case disabled. All 64 source checks and 14 selected offline harnesses pass. The two affected harnesses and all source checks passed again after the final checker diagnostic correction. See `review-holds.json` for artifact identity and limits. Compiler warnings remain; this is not a warning-free build claim.

This supersedes the earlier missing-test, fixture and lint blockers. The original `source-inputs.json` and `verification.json` remain historical evidence; the new runtime build also includes the restored fixture options and reviewed style-only changes. No SQL bytes changed, so matching prior database replay evidence remains applicable.

The legacy real-level lift regression remains byte-for-byte preserved and explicitly blocked by owner choice. The upstream-style per-player scenario is validated but was not gameplay-run. No new gameplay or server startup was performed for this review.

The repaired candidate qualifies for source integration into our fork after published commit and remote-check verification. The owner authorized pushing the reviewed repairs to PR #113; merge remains explicitly held. Development acceptance, Cloud promotion, launcher releases and Discord announcements remain separate.
