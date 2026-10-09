# Area 52 Legendary batch of ten

This combined patch connects ten Legendary Mystic Enchants to their Area 52 spell contracts. It includes the
previously staged Battlemage change; do not apply that earlier patch again. All ten are packaged together.
The October 9 activation installed the matching local client, applied both migrations through the normal updater,
and published client channel commit `017c429922d2b62b7e5c5c3dc28792b418451646`. The public feed passed
the actual launcher validator after GitHub's cache refreshed. Server binary SHA-256:
`ce46fc5011145ea6e08fbf52d12953c0caec18554559c7b1b238b82a628ad0dd`.

| Enchant | Root | Repair |
| --- | ---: | --- |
| Unbounded Spell Slinger | 84510 | Direct-damage proc, four-second cooldown, one Lesser Clearcasting charge and burst |
| Spell Slinger | 965503 | Six-second cooldown, two Clearcasting charges, resource eligibility and Arcane Barrage modifiers |
| Frost Lich | 965504 | Bone Chill, twelve-second cooldown, scoped PvE/PvP vulnerability and learned Fingers of Frost prerequisite |
| Harbinger of Flame | 965505 | Direct-damage proc with per-caster, per-target cooldown marker |
| Battlemage | 81258 | Highest learned Arcane Barrage, Fire Blast or Ice Lance rank; four-second cooldown and scoped PvP reduction |
| Corrupted Bear Form | 99634 | Bear-form visual, Shadow Paw, known Corruption ranks, Swipe leech and scoped instant Shadow Bolt cleave |
| Overheating | 81119 | Melee-generated heat stacks, native five-stack cap and Fire Mage Heat Stroke trigger |
| Overloaded Froststorm | 81122 | Stack thresholds, storm damage scaling and Overloaded Mind additional damage |
| Astral Tempest | 81479 | Correct stack helper, once-per-cast stacks, eight-yard storm and damage scaling |
| Wild Felfire | 965315 | Four Pure Chaos Bolt replacement ranks, durable ownership, ground effect, haste, shield removal and cooldown interactions |

The catalog requires the applicable script bindings before accepting these enchants as connected. Player handlers
and metadata changes are gated to Area 52 Free Pick. Existing durable grant ownership is reused; logout does not
remove learned replacements. Unequipping Wild Felfire removes only its owned replacement grants. Original ranks
remain the training-book source and learning hooks synchronize their replacements. Other entries use already
learned spells or transient buffs rather than granting unrelated class abilities.

Corrupted Bear follows the upstream form-synchronization concept. Its Shadow Bolt exception is limited to
Nightfall or Backlash while in Bear/Dire Bear form. The cleave uses the native 986146 damage reduction and the
standard ten-yard magic-chain range; that range is an implementation choice, not a recovered official value.
Normal bear-form threat remains in effect. Overheating retains the effective five-stack maximum; the stale helper
tooltip is corrected to read that maximum. Frost Lich follows the parent enchant's coefficients and twelve-second
cooldown rather than the conflicting helper text or upstream Wildcard's nine-second cooldown.

## Research and compatibility

Reviewed `jealous-sound/azerothcore-wotlk-coa` at `09da7d129571fc375780de5ebf70a4220e9f36f1`, then fetched
and reconciled through `5d2ef8852b337d498a52f03ff38b8f198d7911f3`. The intervening Malgorm/macOS changes
do not change these contracts. Upstream Wildcard explicitly identifies Season 10. No separate Season 10 Free Pick
backend was identified. Fallback cache `hertigservices/ascension-data` was reviewed at
`09c361f8b4d51c8a2d6ce84165ca53988d429335`. These are independent reconstruction/cache sources,
not Ascension's official backend.

The primary Area 52 installed client supplied the descriptions and effective overlay. Its starting `patch-D.MPQ`
SHA-256 is `e2345ec6c824977214c46552057cb83d1bb6f06d797b14b8c62977e7fe6d233b`.
The staged archive SHA-256 is `5c94a375b7701a62bd8549b04ea420e449957cb9f64501408e1e024650e98bdf`.
`client-changes.json` records matching numeric fields and text edits. No proprietary archive is published here.
This review does not establish the latest Discord-distributed upstream client release; release/attachment tracking
remains a gap. The current installed overlay is the reviewed baseline, not a claim of newest upstream client.

## Verification

- `verification-ready/report.json`: **VERIFY ALL: PASSED** for combined build, unit and two focused harnesses.
- `verification-ownership/report.json`: **VERIFY ALL: PASSED** for the production-function fixtures, including
  Felfire rank replacement, logout preservation, unequip removal, independent ownership, and Bear-form helpers.
- `verification-ready-bindings/report.json`: **VERIFY ALL: INCOMPLETE** because only an exploratory native
  startup scenario ran. That scenario passed (one passed, zero failed), exporting effective data after normal
  database migration. It is not a simulated combat or combined gameplay verification claim.
- `verification-catalog/report.json`: **VERIFY ALL: PASSED** for assertions on that native export: all ten
  connections, required scripts, proc contracts, coefficients and fifty related rank records.
- `verification-client/report.json`: **VERIFY ALL: PASSED** for archive round-trip, declared-field-only changes,
  ten yellow VERIFIED labels, preservation of SHIFT content and unchanged unrelated archive members.
- Changed C++ and SQL style checks pass. SQL's first Windows invocation encountered a console-encoding error
  printing its success symbol; rerunning with UTF-8 succeeded. This was not an SQL failure.

Effective cast times, channel state and movement interruption are checked across the relevant rank families.
Shadow Bolt and both Chaos Bolt families retain movement interruption. Battlemage ranks are instant.
Triggered helpers do not gain blanket movement flags. Scoped instant-cast modifiers remain intact.
In-game combat, movement behavior and reconnect persistence remain tester CERTIFICATION work. Candidate VERIFIED
labels mean structural checks passed, not that those player tests happened. No flaky outcome was observed in these
final checks; investigation-to-completion timing is unknown.

## Applying the package

`repair.patch` is against the preserved private Area 52 integration and includes its existing ownership/catalog
interfaces. It is **not** a standalone upstream-main cherry-pick. This focused publication preserves the source
without mixing unrelated private work into an upstream PR. Local native-export/client harnesses require the
recorded audit artifacts and primary client; the production-rule harness runs through `tools/verify_all.py`.

Apply both pending world migrations through the normal server updater with the matching compiled binary and
staged client definitions. Promote only the tested Area 52 client/server pair through its separate launcher channel.
Before activation, recheck the installed archive hash and reconcile any later changes rather than overwriting them.
Use the normal three-minute announcement/countdown for restarts. Activation exposed a stale mounted automatic-access
ledger, requiring a second announced restart. Future activation preflight must stage the current verification ledger
with the binary/client package and compare the expected eligible IDs with the startup access count before completion.
