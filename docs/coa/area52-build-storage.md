# Area 52 build storage

The native Free Pick Build Creator can publish, browse and reopen build plans using the characters
database. No web service is required. Plans belong to the authenticated account; its characters can
edit them. Other accounts can browse a plan and create their own copy, but cannot change its original.

Enable `CoA.FreePickBuilds.Enable = 1` only after applying the pending characters migration.
The service also requires `CoA.Enable = 1`, `CoA.ClassModel = "hero"`, `CoA.RealmType = "live"`
and `CoA.GameModeMask = 0`. Requests are accepted only for Free Pick Hero characters.
The option defaults to disabled. Enable it before login; reconnect after changing it.

## Stored plans

Each accepted save appends an immutable revision. The native spell plan, Mystic Enchants,
equipment recommendations, descriptions, role, primary stat, icon and related-build IDs are retained.
Format version 1 stores the bounded native record as hexadecimal in a JSON `wire` field; name and
category are indexed separately. The JSON document is limited to 64 KiB and input packets to 24 KiB.
Account IDs deliberately have no foreign key into the separately hosted authentication database.

The parser rejects truncated records, excessive counts or text, duplicate spell/enchant entries and
invalid boolean values. The service checks spell existence and level bounds. These are plan-storage
checks: they do not certify that a plan fits the advancement catalog, budgets or enchant ownership.
Loading opens the plan in the editor. It does not teach spells, equip enchants, spend currency or apply
progression. Applying a plan to a character requires a separate server-validated implementation.

Author, account ownership, class, timestamps and Featured status are server-controlled. An update must
match the saved timestamp and expected revision. Its database insert checks ownership and that revision;
a conflicting update cannot overwrite the winner. The response is sent only after transaction completion
and a readback of the stored record. Account-scoped content IDs reject repeated identical creates.
Copies use the copying account and do not inherit Featured status.

## Featured and limits

Featured entries are staff-controlled database records pinned to an approved revision. Public reads
continue to use that revision after an author edits the draft. Owners can retrieve their latest revision
for editing. The client cannot set Featured status. A staff management interface and the future
character-creation archetype mapping are not implemented here.

The first version accepts up to 128 stored builds per account and 1,000 revisions per build. Browse
responses contain up to 256 builds, with the requesting account's builds first. Responses are split into
bounded pages. Requests are limited to one every two seconds, with at most two queued requests per
session; pages are sent at most once per 100 ms. Logout invalidates pending session responses.
Archived records are excluded from browsing and cannot be edited; no client archive handler is included.

## Verification

Run source, build and unit checks through `tools/verify_all.py`. The focused harness selection is
`tools/verify_all.py --stages harness --harness area52_build_protocol area52_build_storage`.

The protocol harness uses independently packed synthetic records and checks round trips, truncation,
duplicates and bounds. An optional local `COA_BUILD_PACKET_SAMPLE` can verify a captured native publish
request; private player data is not distributed with the tests.

The storage harness exercises the actual prepared SQL statements, including foreign-owner rejection,
stale and simultaneous saves, pinned Featured reads, archive protection, integrity limits and rollback.
It requires Docker and a local `mysql:8.4` image (or `COA_TEST_MYSQL_IMAGE`) and removes its disposable,
network-isolated database container afterward. Native in-game browse/save/reopen and two-account tests
remain necessary before gameplay acceptance; passing schema and protocol tests alone is not sufficient.
