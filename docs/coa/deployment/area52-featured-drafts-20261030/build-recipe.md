# Featured Draft candidate reproduction

The baseline is the deployed area52-playerbot-readiness-repair-20261030 build. Apply server.patch to that reconciled source, including its existing bot module snapshot and all unrelated deployed corrections. The patch adds one pending characters migration; migration deployment must precede the new server startup. Source before/after hashes are listed in verification-summary.json. Do not apply the scoped patch blindly to a different core baseline.

Use the existing WSL offline builder and tools/verify_all.py build/unit stages. Focused entitlement, Lua first-login/import, assignment/reward SQL and exact migration/client rollback checks qualify the new paths without running gameplay. Build provenance accompanies the final candidate. The runtime adapter exercises production entitlement, slot validation and aura synchronization, while external player/session and equipment services are bounded adapters.

Only the specified client MPQ changes. Extensions.dll, account data, collection ownership, certification/status records, bot templates/equipment and unrelated managed client files are preserved. Publication is prepared on a feature branch; no main merge is part of activation.

After deployment, a GM copies a build, selects DRAFT BUILD (Featured), chooses its creation archetype and saves it. No draft is seeded automatically. Subsequent assignments resolve on the server through stable client references, so saving a different registered revision requires no client catalog rebuild. Already-active characters retain their pinned revision.
