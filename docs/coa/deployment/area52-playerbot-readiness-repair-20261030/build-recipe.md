# Bot readiness repair reproduction

Core baseline: deployed area52-worldforged-verification-20261030. Apply server.patch; before/after hashes are in verification-summary.json. Preserve all prior realm changes.

Module baseline: the pinned module_commit in verification-summary.json. Apply module.patch to reproduce all recorded module_changes. This full module patch retains earlier pilot work; activation replaces only the changed recruitment message source.

Use the existing WSL offline builder, tools/verify_all.py, runtime install prefix /workspace/env/freepick, and two low-priority jobs. Native build/unit and area52-companion-final checks cover the actual bot login branch, full-name command tokenizer, enchant readiness, connection startup predicate, Earthliving callback, fixed equipment, recruitment/reservation/release and deployed archetype eligibility at every level15–80. Storage/session services are bounded offline adapters, not gameplay acceptance. Original failed-login and truncated-name baselines are retained in the checks.

The package has four guarded source replacements plus the server executable. Standard deployment backup/rollback restores source and binary. There is no SQL, roster reset, config change or client asset. Normal bot-owned equipment/enchant/appearance records persist through binary rollback. Human appearance initialization and other realms are excluded from the bot-only path; the scoped Mystic command and Area52 Earthliving fixes also serve their normal native callers.

Source preparation and patch reproduction do not authorize publication. Stable activation preflight must pass before readiness; owner activation is separate.
