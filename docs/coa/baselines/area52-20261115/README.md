# Area 52 deployed baseline

The November 15 source reconciliation matched 5,246 files, including 1,715 playerbots files. The retained compiler inputs matched all 2,390 workspace translation units. The clean build linked the worldserver and passed its native test target (6,104 passed, 5,486 skipped, one disabled). The four focused Free Pick regression checks passed with the two test adapter updates in this branch.

## Playerbots

From the root of a fresh **module** checkout, check out the commit in `playerbots.json`, verify the SHA-256 of `playerbots.patch`, and run `git apply --cached --binary /absolute/path/to/playerbots.patch`. Confirm `git write-tree` returns `ea205e1c465fa33f1f59ed66343e405f882b47e2`, then run `git checkout-index -a -f` in that dedicated checkout. Mount that module at `modules/mod-playerbots`. The offline check applied the patch to the pinned base and compared the resulting tree.

## Cross-faction battlegrounds

The live settings in worldserver.conf are CFBG.Enable=1, CFBG.KeepRace=1, CFBG.Battlefield.Enable=0. The runtime modules directory contains no CFBG.conf override. The clean native ConfigMgr returned 1/1/0 with both an empty module list and CFBG.conf listed but absent. Adding a module config changes precedence: preserve those three settings if installing one.

## Database evidence

`migration-ledgers.json` lists recorded names, hashes, states, and UTC epoch timestamps for auth, characters, world and playerbots. Equal timestamps are ordered by name for stable output; the database does not record their internal execution sequence. The playerbots updates ledger is empty on this realm.

`deployment-order.json` pins completed transactions by completion time, the apply hooks in manifest order, and packaged SQL by SHA-256. The SQL inventory includes verification and rollback inputs. Preserve the reviewed hooks' control flow when reconstructing a migration. Package completion order is evidence, not an executable fresh-install replay script. Character/account contents and private transaction backups are excluded. Live GM edits made outside packages are separate state; for example, Blackfathom Ascendant GUID 7748030 was added after the 161-spawn restoration.

This PR is for the fork baseline. Upstream integration, a full blank-database replay, and in-game acceptance are separate work.

## Clean database replay, November 16

A new MySQL 8.4.11 fixture ran with Docker networking disabled. The auth and character seeds came from the pinned core. The world seed was the original 318-table CoA archive. Its 3,027 ledger entries required 795 additional migrations and two covered-file newline reconciliations. The auth ledger needed seven updates and characters needed 33. All four ledgers matched the realm on name, hash and state.

`ledger-replay-order.json` pins the recovered order and byte identity. Verify it with `python3 check_replay_inputs.py --private-dir /path/to/private-migration-directory`. One historical reward rule contains a specific account ID. Its exact bytes remain private. The module's empty ledger requires its separate pilot SQL sequence in `pilot-sql-order.json`; its two subsequent bot-profile receipts were verified with the original hook checks.

The 27 empty character storage tables and two altered columns are preserved in `character-schema-after-ledger.sql` for an isolated reconstruction after the core ledger and playerbots name tables. Package-created tables must be created once when combining this schema snapshot with individual historical package hooks. Character records and account settings are outside the public baseline.

The additional world checks replayed the two Wildcard SQL files at `3f8e0717`, the creature family migration at `2b3977f1`, the original Callboard installation, Temporal Contracts, daily zone rotation, Vanity and trophy catalogs, and the guarded 161-spawn restoration. The restoration fails on a signed TINYINT family until the historical taming migration is included. The separate `blackfathom-placement.sql` reproduces the later manual spawn and was checked against the full active row.

The baseline remains open for historical gameplay-data parity. In particular the October 5 Mystic Scroll migration predates the formal transaction list and applies to the creature population then present. Its original manifest supplies the hash. Reconstruct it at that checkpoint before later creature additions. Executing it at the end would also change creatures added later. The Rootbound mount clone is one downstream test for its source template's loot ID. A passing ledger or matching table definitions does not qualify all post-ledger content.
