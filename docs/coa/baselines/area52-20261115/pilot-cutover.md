# Area 52 account and character cutover

Advertise the exact realm name `Area 52 - Free-Pick` on the pilot and at cutover. Character Creation checks that literal name before enabling archetypes. Distinguish the pilot by its login and world endpoints, and verify the name and world port together in the authenticated realm list.

## Verified Mystic Enchant access before the first player login

The production worldserver reads an external verification report through `CoA.FreePickMysticEnchants.VerificationFile`. A database copy preserves equipped slots and permanent collection ownership. The worldserver also needs this report to reconstruct temporary tester access. On the GM pilot, a missing report caused login synchronization to clear three Dark Ranger spell grants and four saved action buttons even though the enchant remained equipped.

1. Capture the source realm's current report and SHA-256 at cutover, together with the effective enable setting, tester-access setting, and excluded accounts. Use the source realm's current values. Keep temporary access distinct from permanent collection ownership.
2. Stage the report at the destination's configured path. From inside the destination worldserver container, verify that the file is readable and its SHA-256 matches the source. The pilot's launch script contains an example.
3. Start the destination before opening it to players. Check the **current startup** log for the loaded Verified-access count and compare it with the source for the captured report. Investigate a missing-file error, zero count, or unexplained count difference before opening logins. The pilot loaded 2,742 entries on November 17, 2026. That is a historical observation, not a fixed future target.
4. Preserve the pre-login character snapshot. Compare the canary character's equipped enchant IDs, saved grant owners, learned spells, and specialization-specific action buttons before and after login. Check the client's spellbook and bars, then relog once to check persistence. If a login clears valid grants or bars, hold wider access and recover the affected placements from the pre-login snapshot after correcting the access input.

### Pilot evidence — November 17, 2026

Darkwing Goose (GUID 111) had Dark Ranger 997115 equipped. The affected specialization was 0. The saved buttons were 6 and 75 for Shadow Artillery (998204), 9 for Blood Arrow (997517), and 10 for Shadow Arrow (997526). After the report was restored, the pilot worldserver restarted, and the four placements were recovered, the owner confirmed all three abilities appeared on the bars. A second relog and the separate Dark Arrow skeleton proc were not confirmed by that observation.

## Cloud staging and launcher routing

The owner selected cloud staging for the merged realm on November 17. Inventory the destination host, available memory, storage, CPU, database version, network ports and existing services before installation. Capture the final candidate binary, source, pinned playerbots input, ordered migrations, configuration, terrain/DBC overlays and external verification report as one deployment inventory. The GM pilot disables playerbots; use the approved production bot settings when qualifying the destination.

Stage the destination with restricted player access and a dedicated database. Rehearse the upgrade using a fresh authorized copy of current production data. Final account and character transfer must use a consistent snapshot at cutover, preserving account IDs, character GUIDs, collection ownership, grants, action bars, mail and reward state. The pilot GM account copy is a test fixture. Retain a rollback snapshot and a single authoritative writer during transfer.

Preserve the advertised realm name `Area 52 - Free-Pick`. Configure the auth endpoint and the publicly reachable world endpoint in the realm list together. Validate both ports from a tester connection before routing the launcher there.

The launcher supports `endpoint-migrations.json` with schema 1 and exact `channel`, `before`, `after` routes. The Area 52 route currently requires port 3725 at both endpoints. A cloud auth listener or port mapping on 3725 can use this existing mechanism. A different auth port requires a reviewed launcher change. Explicit LAN/localhost connection settings stay intact unless their exact address is deliberately included in a route. Verify old tester addresses, fallback addresses, the owner's local override and preserved backups before release.

The cloud address and destination capacity remain to be established. Publish the endpoint migration only after destination acceptance and a separately authorized cutover. The local production realm remains the source of current account and character progress until then.
