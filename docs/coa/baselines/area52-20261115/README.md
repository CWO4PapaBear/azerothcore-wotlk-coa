# Area 52 deployed baseline

The November 15 source reconciliation matched 5,246 files, including 1,715 playerbots files. The retained compiler inputs matched all 2,390 workspace translation units. The clean build linked the worldserver and passed its native test target (6,104 passed, 5,486 skipped, one disabled). The four focused Free Pick regression checks passed with the two test adapter updates in this branch.

## Playerbots

From the root of a fresh **module** checkout, check out the commit in `playerbots.json`, verify the SHA-256 of `playerbots.patch`, and run `git apply --cached --binary /absolute/path/to/playerbots.patch`. Confirm `git write-tree` returns `ea205e1c465fa33f1f59ed66343e405f882b47e2`, then run `git checkout-index -a -f` in that dedicated checkout. Mount that module at `modules/mod-playerbots`. The offline check applied the patch to the pinned base and compared the resulting tree.

## Cross-faction battlegrounds

The live settings in worldserver.conf are CFBG.Enable=1, CFBG.KeepRace=1, CFBG.Battlefield.Enable=0. The runtime modules directory contains no CFBG.conf override. The clean native ConfigMgr returned 1/1/0 with both an empty module list and CFBG.conf listed but absent. Adding a module config changes precedence: preserve those three settings if installing one.

## Database evidence

`migration-ledgers.json` lists recorded names, hashes, states, and UTC epoch timestamps for auth, characters, world and playerbots. Equal timestamps are ordered by name for stable output; the database does not record their internal execution sequence. The playerbots updates ledger is empty on this realm.

`deployment-order.json` pins completed transactions by completion time, the apply hooks in manifest order, and packaged SQL by SHA-256. The SQL inventory includes verification and rollback inputs. Preserve the reviewed hooks' control flow when reconstructing a migration. Package completion order is evidence, not an executable fresh-install replay script. Character/account contents and private transaction backups are excluded. Live GM edits made outside packages are separate state; for example, Blackfathom Ascendant GUID 7748030 was added after the 161-spawn restoration.

This PR records reconstruction of the fork baseline. Upstream integration and in-game acceptance are separate work.

## Clean database replay, November 16

A new MySQL 8.4.11 fixture ran with Docker networking disabled. The auth and character seeds came from the pinned core. The world seed was the original 318-table CoA archive. Its 3,027 ledger entries required 795 additional migrations and two covered-file newline reconciliations. The auth ledger needed seven updates and characters needed 33. All four ledgers matched the realm on name, hash and state.

`ledger-replay-order.json` pins the recovered order and byte identity. Verify it with `python3 check_replay_inputs.py --private-dir /path/to/private-migration-directory`. One historical reward rule contains a specific account ID. Its exact bytes remain private. The module's empty ledger requires its separate pilot SQL sequence in `pilot-sql-order.json`; its two subsequent bot-profile receipts were verified with the original hook checks.

The 27 empty character storage tables and two altered columns are preserved in `character-schema-after-ledger.sql` for an isolated reconstruction after the core ledger and playerbots name tables. Package-created tables must be created once when combining this schema snapshot with individual historical package hooks. Character records and account settings are outside the public baseline.

The additional world checks replayed the two Wildcard SQL files at `3f8e0717`, the creature family migration at `2b3977f1`, the original Callboard installation, Temporal Contracts, daily zone rotation, Vanity and trophy catalogs, and the guarded 161-spawn restoration. The restoration fails on a signed TINYINT family until the historical taming migration is included. The separate `blackfathom-placement.sql` reproduces the later manual spawn and was checked against the full active row.


The UTF-8 replay inserted the 20 steps in `early-world-order.json` at their October checkpoint. Verify the SHA-256 before executing each step, including repeated hashes. The rank repair uses the activation receipt file timestamp. The October 5 scroll migration has a matching activation receipt. Applying it at this point reproduced the later Rootbound loot guard and the original heirloom inputs. Use utf8mb4 for data imports and utf8mb4_unicode_ci for the connection when a historical user-variable comparison requires it.

The Wardrobe world migration matched its original before and after checks. The mount checkpoint reconstructed item 51955's required level of 60 from the original hashed before-image. All 1,595 rows matched the mount package before and after its world-only update. These checks exclude player-owned build revisions.

The pinned historical replay independently reproduced 337 of 356 world tables. To reproduce the captured realm state, retain the private world recovery inputs alongside the separately pinned private account rule. The 19 SQL files under the preservation workspace's `private-world-recovery` directory contain only residual world differences, including GM hotspot data and historical updater timing. Their hashes, operation counts, affected columns, and before/after table fingerprints are in `world-replay-verification.json`. These are November 16 preservation inputs derived from the active realm. They do not establish the author or original deployment of each change.

For an isolated reconstruction, compare every table with its historical replay fingerprint first. Verify the private manifest and SQL hashes, then apply each per-table transaction only to its matching before-state. Confirm all 356 final fingerprints. The qualification repeated the bounded read of the active world and hashed every reconstructed table. The active passes were taken over time, without a global read lock. The result covers static world parity and the separately qualified schema and ledger checks. Account and character contents remain private.
