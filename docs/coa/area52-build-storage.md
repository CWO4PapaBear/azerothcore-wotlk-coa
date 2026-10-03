# Area 52 build storage

This migration provides realm-local storage in the characters database for Free Pick builds.
It does not register packet handlers, enable the browser, change progression, or grant spells.
Other modes do not consume these tables.

Build ownership uses the authenticated account ID, allowing its characters to maintain the same builds.
Public browsing and copying are intended; only the owner may edit or archive an original.
The application must enforce these permissions, including direct build-ID requests.
Account IDs deliberately have no foreign key into the separately hosted authentication database.

Each save appends a numbered revision. The document preserves the native editor's spell plan,
Mystic Enchants, equipment-type recommendations, descriptions, role, primary stat, icon and related-build IDs.
It is a versioned JSON object limited to 64 KiB; category and name are stored separately for browsing.
Handlers must validate its structure, counts, text lengths and catalog references before saving.
Client author names, ownership and Featured flags are not authoritative.

Save handlers must lock the build row in a transaction, check ownership and the caller's expected revision,
then append exactly one revision. Conflicting edits must return a conflict instead of silently overwriting.
The latest revision is the maximum revision for a build. Revisions are immutable by application policy.
Copies receive a new build ID and the copying account as owner; copying must not transfer Featured status.
Archiving hides a build from ordinary browsing without deleting its revision history.

Featured is a separate staff-controlled designation pinned to a particular revision. Later author edits
do not silently replace the approved build. Deleting a referenced revision is prohibited until staff
remove its Featured designation. The future character-creation archetype mapping should reference a
specific approved revision; this migration does not guess or populate those mappings.

Before enabling writes, implement authenticated, Free Pick-gated handlers with request limits,
ownership checks, revision conflict handling, and permission checks for Featured changes.
Applying a saved plan requires separate server validation of progression and enchant eligibility.
Storage tests do not establish packet compatibility or gameplay correctness.

Verify the schema using `tools/verify_all.py --stages harness --harness area52_build_storage`.
The harness requires Docker and a locally available `mysql:8.4` image (or `COA_TEST_MYSQL_IMAGE`).
It starts a disposable container without network access, host mounts or published ports, and removes it afterward.
