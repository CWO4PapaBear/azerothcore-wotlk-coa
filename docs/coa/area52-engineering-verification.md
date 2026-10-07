# Area 52 engineering verification

VERIFIED means the applicable engineering gates have passed: structural connections,
granted-ability ownership, persistence, and book/rank integration. It does not assert
combat balance, proc timing, animation correctness, or complete gameplay behavior.
CERTIFIED is reserved for separate player acceptance; these tools never assign it.

The other statuses are Structurally Connected, Unverified; Needs Investigation; and
Confirmed Defect. A confirmed defect takes priority over passed gates. Missing evidence
cannot produce VERIFIED. Requirements that do not apply, such as a higher rank for an
unranked passive, are explicitly recorded as not applicable.

`tools/area52_verification/status.py` reads an effective runtime export and the per-entry
structural audits. Each record retains all four gates and their evidence. Common native
modifier entries can use reviewed shared persistence implementation and existing player
persistence evidence. This is not a claim that every entry received an individual database
reconnect test. Custom scripts, granted spells and replacements require additional review
and are not automatically promoted by the common persistence profile.

The evidence JSON contains the effective export SHA-256, hashes of the reviewed source
files, descriptions of the common persistence/book evidence, unmatched modifier IDs, and
any confirmed defects. The generator rejects changed source or effective-data hashes.
Only provide evidence for the candidate actually reviewed. Keep private exports and client
archives outside Git. The status output contains every catalog/advancement record, including
entries that cannot be promoted yet.

```text
python tools/area52_verification/status.py --audit AUDIT_DIRECTORY --evidence EVIDENCE_JSON --output STATUS_JSON
python -B tools/verify_all.py --stages source,harness --base origin/main --harness area52_verification
```

`stage.py` stages a new Area 52 overlay using the existing MPQ utility (`lib.mpq` with
`MPQArchive` and `write_archive`). Supply its containing directory with `--mpq-tools`.
It requires the exact source archive hash, rebuilds the status report, and checks every
member after archive writing. It does not install, publish, restart, or overwrite archives.
Only enUS spell descriptions are changed; the client’s existing `@ext:` format displays
the status on SHIFT expansion. The visible text is only green VERIFIED or CERTIFIED;
all entries still needing work show NOT VERIFIED. Detailed categories remain in the reports.
CERTIFIED requires explicit player acceptance and is never inferred from engineering checks.
Repatching replaces either the original verbose label or the compact label
instead of duplicating it, and shared spell IDs receive the least favorable applicable status.
Higher ranks and known granted-spell IDs inherit the entry status.

```text
python tools/area52_verification/stage.py --audit AUDIT_DIRECTORY --evidence EVIDENCE_JSON --source AREA52_OVERLAY --expected-sha256 SOURCE_HASH --mpq-tools MPQ_TOOLS_DIRECTORY --output NEW_STAGED_OVERLAY
```

Labels are static client content. Deploy them only with the reviewed client/server pair.
Do not install a candidate's green labels over a live server lacking its required repairs.
In-game SHIFT rendering remains a separate acceptance check after installation.

## First Area 52 activation (2026-10-07)

The first labeled client/server pair was activated on the owner's Area 52 test realm.
The deployment binary passed build, unit and three focused harness checks. An isolated
native export-only scenario also passed; the runner correctly classified it as exploratory,
not as completed combined gameplay verification. Player testing remains separate.

The partial Shield of Righteousness heal repair and four unresolved book-rank mappings
(Prayer of Healing, Mongoose Bite, Aspect of the Hawk and Consecration) were excluded.
Comparison of the deployment export with the reviewed catalog found only those four
rank-chain differences among reachable spell records. All four retain Needs Investigation;
no green label depends on the excluded changes. The catalog and advancement entries
were unchanged. The Shield entry retains Confirmed Defect.

The installed overlay labels 1,666 Mystic Enchants and 773 advancement entries VERIFIED.
It also preserves the other status categories and does not assign CERTIFIED. The overlay
SHA-256 is `5c614413cde19035d5ca6171a225550696ead17536b4b38a02023732236b7146`;
the deployment binary SHA-256 is
`c8d4f3d82b04470a769806d345b369c64e2300bd5693ddc762141d11d46213ee`.

Local server activation and client installation do not publish a launcher release.
Rollback copies of the previous configuration, affected world tables, private character
data and client overlay were retained outside Git. The protected PTR services were unchanged.
Upstream was fetched again through `8b1f3a0c0`; its new changes were not imported into
this previously verified deployment candidate.
