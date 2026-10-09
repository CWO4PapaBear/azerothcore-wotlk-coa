# Area 52 tester certification

Adds a right-click certification action to Character Advancement abilities and talents. A tester
confirms expected function, logout/login persistence, and action-bar removal after unlearning.
The server accepts only VERIFIED entries from its ledger, records the first certification, and
shares CERTIFIED status immediately with online Area 52 players. Later logins request a snapshot.

The record contains the authenticated account, character, timestamp, all three confirmed checks,
and the ledger digest. Client messages cannot select another account or certify an unverified
entry. Writes are checked before success is returned. Repeated submissions are idempotent and
requests are rate limited. No spell, action-bar, build, or currency state is changed.

`area52-certification.patch` is a focused delta against the deployed Area 52 reconstruction,
not a drop-in patch for CoA main. CoA main does not contain this reconstruction's
`AscensionFreePick.h` API. Verify the LF-normalized hashes in `source-preconditions.json` and
run `git apply --check` before applying. Unrelated changes in the development checkout are
excluded. No upstream PR or compatibility claim is implied.

## Activation

1. Apply the focused patch to the matching Area 52 source and run the focused verification
   through `tools/verify_all.py`, including build and `--harness area52_certification`.
2. Apply `rev_20261009_06_area52_certification.sql` through the normal character-database
   updater before starting the candidate binary.
3. Mount `certification-ledger.json` read-only and set:

   ```ini
   CoA.FreePick.TesterCertification = 1
   CoA.FreePick.CertificationFile = "/tester-certification/certification-ledger.json"
   ```

4. Distribute the matching `TesterCertification.lua` and updated `Area52MysticRules.toc`
   from the Area52-FreePick-Client repository through the validated Area 52 launcher feed.
5. Use the required in-game announcement and three-minute countdown for server activation.
   Have two testers confirm that the menu, cancellation, certification, and shared green
   SHIFT status work; reconnect one client to check synchronization.

The compact Mystic Enchant test-access file is not the certification ledger. It omits the
38 prior tester certifications. This package preserves those existing certifications and adds
new database records on top of the currently VERIFIED entry set. Investigation/defect entries
are excluded even if a previous certification record exists.

For future progress tables and client status releases, merge entry IDs from
`area52_tester_certification` with the engineering ledger's `certified` flags, counting each
entry once. Do not publish the table's account or character columns. Certification is player
attestation, not an automated claim that gameplay was tested by the server.

Source publication and client source synchronization do not activate the feature, install
client files, or promote the tester channel. See the verification record for the actual checks.

## Verification

Build and certification-rule verification: **VERIFY ALL: PASSED**. Focused patch application:
**VERIFY ALL: PASSED**. Lua 5.1 checks passed for the menu, confirmation, acknowledgements,
shared status, timeout recovery, spoof rejection, and exact SHIFT-text preservation.

The isolated native protocol scenario passed all assertions on a real clock, including the
database-backed response and synchronization to another account. Its wrapper reports
**VERIFY ALL: INCOMPLETE** because the scenario is exploratory rather than catalog registered;
this is not a full gameplay or reconnect certification. In-game visual acceptance remains.

`native-ledger.fixture.json` is only the disposable protocol-test fixture. Never activate it:
use the complete `certification-ledger.json` for the realm. No test certification was written
to the live character database.
