# Portable Featured catalog worker

This optional Linux service ships in the server repository. It listens to the worldserver's
`CoA.FeaturedCatalog.SignalPath` using inotify, debounces saves, and renders pinned client archives.
It has no Windows task, SSH bridge, Docker socket, or personal-host dependency. It needs Python 3.12+,
a MySQL client and, for publication, GitHub CLI. The included container supplies those dependencies.

## Saved data and delivery

The committed Featured revision is the authority. New Draft starts already select that revision through
`CHAR_SEL_A52_CREATION_DRAFT` or `CHAR_SEL_A52_BUILD_PUBLIC`; rewards read its saved description.
Existing Draft assignments remain pinned. The worker never writes character or world databases.
It exports equipment to ItemSet and explicit Wardrobe slots, reward bundles to VanityCollection,
scene details to patch-E, and summaries to both patch-E and patch-M. Unrelated archive members are preserved.
Client menus refresh after the updated client catalogs are downloaded and the client is restarted.
A server save cannot alter an already running client's loaded DBC tables.

## Configuration

Create a private worker.json with schema 1, mode, state_directory, signal_path, database, renderer,
release and github sections. Paths in the container use /inputs, /state, /config and /signals.
Start with mode `stage`; this prepares files without network writes. Other modes require an operator's
explicit deployment authorization: `github_pr` uploads immutable release assets and opens one review PR;
`github_direct` uploads assets and promotes the channel using a non-forced Git update.
PR mode never approves or merges. It waits for the exact candidate pointer to reach the live branch.

Database keys are defaults_file, characters, world and optional executable (default mysql).
Use a mode-0600 MySQL defaults file with a dedicated SELECT-only account for area52_build,
area52_build_archetype, area52_build_revision, area52_build_featured and coa_client_vanity_collection.
Do not grant mutation privileges or mount the realm's root password. No credentials belong in Git.

Renderer inputs are hash-pinned objects `{ "path": "...", "sha256": "..." }`:
details, items, incarnations, glue, local_glue, archive; rewards contains sets, vanity, appearances,
mounts and defaults. summary_catalogs contains realm, base and archive. Set vanity_prices true to
include configured store prices. Supply the licensed client data separately; none is distributed here.
Use the existing approved catalog extraction to obtain these records. Archive means patch-E;
summary_catalogs.archive means patch-M. outbox and mpq_tools are assigned by this worker.

Release keys: repository, pinned manifest and pointer objects, archive_sha256 and base_archive_sha256.
The baseline pointer must hash-match the baseline manifest; archive hashes must match its inventory.
GitHub keys: repository (same as release.repository), channel_path (e.g. channels/area52.json),
base_branch (default main). GitHub credentials need contents write and pull requests write.
Use a server-owned GitHub App/token scoped to that client repository, never a tester credential.

Set the compose environment paths and REALM_NETWORK to the existing realm network. The worldserver's
signal directory must be mounted at /signals and state must be writable by the configured UID.
Run `docker compose --profile featured-catalog up -d --build` only during an authorized deployment.
The service is opt-in and is not started by compiling worldserver. Retire the previous catalog publisher
before enabling it; retain the independent realm keepalive. Server owners can run this service alongside
any compatible server installation without this workspace or its Windows machine.

## Recovery and review

State persists notifications, jobs, asset hashes, write intents and PR receipts. Idle operation waits on
filesystem events; retries back off to 30 minutes. Startup reconciles missed saves. An uncertain write is
read back, never blindly replayed. An unresolved write stops that job for operator review. Closed PRs,
changed branch contents and remote asset mismatches fail closed. Never delete receipts to retry a write.
Only one worker may use a state directory. Other publishers must reconcile the feed before promotion.
A changed live baseline blocks delivery rather than overwriting another release's files. Re-pin the current
approved archives and inputs in a reviewed configuration before continuing; do not edit immutable jobs.
Review PRs against their recorded baseline before merging. A merge is publication and requires approval.
No Discord posting is performed by this service.

## Offline checks

Run `python3 -B tools/verify_all.py --stages harness --harness portable_featured_catalog` from the server root.
Tests exercise local filesystem events, saved-revision projections and mocked GitHub failure recovery.
They do not log into the realm, activate packages, upload assets or create real pull requests.
