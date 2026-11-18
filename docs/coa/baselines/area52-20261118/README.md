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

## PR merge holds

The source review restored twelve missing upstream checkers and 549 missing upstream scenario assets byte-for-byte, and added explicit verification of the existing Area 52 conditional registration paths. Thirteen source checks now pass. Two still fail on level-scaling scenarios whose creature field is unsupported by the current fixture loader. See `review-holds.json`; this PR remains draft and is not qualified to merge. SQL lint passes; existing C++ blank-line warnings remain. No gameplay conformance run was authorized or performed.
