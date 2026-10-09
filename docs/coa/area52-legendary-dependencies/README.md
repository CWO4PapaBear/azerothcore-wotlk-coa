# Area 52 Legendary dependency repairs

The 54 abilities/talents linked to the 20 previously VERIFIED Legendary enchants now contain
49 VERIFIED entries and 5 tester-CERTIFIED entries in the staged ledger. This adds 27 VERIFIED
entries: the preceding 13-entry review plus the final 14-entry repair batch.

The runtime patch fixes proc filters, rank-wide overrides, Mangle child grants/ranks, damage and
cost helpers, form restrictions, Mana-forged Barrier, Siegebreaker and Deep Freeze. Power Surge
uses the owner-approved 30% chance and restores 30% of missing mana over four seconds, with a
two-second cooldown. Spellbook ownership stays durable; temporary combat auras are separate.

`area52-runtime-repairs.patch` is a focused delta against the deployed Area 52 reconstruction.
It is not a drop-in upstream CoA main patch: that branch has a different Free Pick API.
Check `source-preconditions.json` against LF-normalized UTF-8 source before applying, and use
`git apply --check` first. Do not apply its realm-specific SQL to another game-mode database.
No proprietary client archive, account data or credentials are included.

The incremental build and production-rule harness passed. Native startup exported the effective
bindings and data, followed by assertions for all 14 entries, client/rank parity and movement
flags. The native export is exploratory, so its verification wrapper reports INCOMPLETE even
when the scenario passes; the focused contract harness provides the subsequent assertions.
No simulated combat or reconnect certification is claimed. Player testing remains CERTIFICATION.
The existing Legendary labels are not a claim that every optional Legendary interaction was
newly re-certified by this dependency batch.

Client tooltip status is yellow VERIFIED, with existing SHIFT content preserved. The combined
archive is staged locally. No server activation, client installation or tester-channel promotion
is implied by this source publication.
