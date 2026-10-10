# Account-attributed certification rewards

This is a focused extension package for the existing Area 52 tester-certification deployment.
It is separate from the training-dummy contribution. Apply `certification-rewards.patch` only to
the declared Area 52 baseline; `source-preconditions.json` records the required file hashes.
The preceding `area52-tester-certification` package remains a dependency. This package is not a
standalone upstream-main migration.

## Attribution and compatibility

Existing submissions retain their account, character, timestamp and advancement entry. Mystic
Enchant records use the same ledger and confirmation protocol, with bit 31 set on the enchant
spell ID. Advancement identifiers remain unchanged. The eligibility loader rejects identifiers
that would overlap the category bit. A spell ID shared by both categories earns distinct credit.

Only the first accepted certification of an entry is credited. A repeated confirmation or
another account observing an already CERTIFIED entry does not earn additional credit. Previously
manual certification labels have no inferred contributor. Existing attributed records count
retroactively; deleted characters do not erase their account's credit.

Replace the certification eligibility file with the supplied combined ledger and deploy
`TesterCertification.lua` at `Interface/AddOns/Area52MysticRules/TesterCertification.lua`.
The existing addon TOC already loads this filename. Existing advancement menus remain intact.
Mystic Enchant certification opens through `/a52certme <enchant spell ID>`, then requires the
confirmation popup. Accepted status is broadcast to online clients and included in login sync.
ME tooltip status becomes green CERTIFIED after server acknowledgement. No client files are
installed or distributed merely by publishing this package.

## Reward configuration and delivery

The characters migration creates a rule table and an account-keyed delivery table. No account
is enabled by the shared migration. Configure a rule using the deployment's resolved account
ID, required count and item ID. The requested local rule is 30 certifications and item 9901073,
Hoplite Vanity Bundle; account identity is resolved separately in the private deployment.

Eligibility is checked on login and after a successful certification request. There is no
periodic poll. The receiving character is the character triggering the check. One item, its
mail and the delivery claim are written in the same characters-database transaction. The
delivery primary key prevents a second persisted reward for the account, including after relog.
The rule applies only while Area 52 certification is active and excludes bot sessions.

For reporting, `entry & 2147483648` identifies Mystic Enchants and `entry & 2147483647`
recovers their spell ID. Advancement entries are unchanged. Count rows with `checks_confirmed=7`
grouped by `account` for earned credit.

## Verification and activation

Focused eligibility/confirmation and reward-query tests passed through `verify_all.py`.
They cover category collisions, 29/30 thresholds, incomplete checks, account isolation and
already-delivered exclusion. Lua 5.1 checks cover confirmation-only submission, response sender
validation, tooltip updates and duplicate rejection. The packaged patch reproduces its declared
runtime and schema files in a clean temporary checkout.

Full server compilation and the final focused harness passed. The corrected mail transaction uses
the asynchronous connection required by the existing item/mail prepared statements and waits for
its result before another eligibility check can proceed. The matched server/client package is
staged, not activated or distributed.

The broad source check in the shared development checkout failed and is not claimed as a pass;
it selected 324 changed files, including unrelated work. Its final failures were Windows tooling
tests (including missing symlink privilege) and the existing script-loader flat-list check.
Focused publication source checks passed. No live mail delivery, client visual acceptance,
restart or tester release is claimed here.
