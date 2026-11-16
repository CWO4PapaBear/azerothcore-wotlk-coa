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
