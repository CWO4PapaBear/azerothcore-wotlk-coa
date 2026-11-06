# Primary Stat weapon bonuses — November 6

Engineering verification is complete for the eight granted weapon bonuses on the deployed Primary Stats r3 source/catalog/binary. Candidate certification UI and status records are prepared, not installed or published. No new server mechanics or server rebuild was needed. Existing Certified rows remain unchanged.

| Primary Stat | One-handed bonus | Two-handed bonus |
|---|---|---|
| Strength | Devastating Strikes (986203): 20% armor penetration contribution | Heavy Swings (986202): 10% more eligible physical ability damage |
| Agility | Agile Strikes (986201): 8% lower eligible ability cost/GCD | Fatal Wounds (986200): increased ability critical damage bonus, as specified by its tooltip |
| Intellect | Devastating Spells (92840): 5% spell damage | Magic Acceleration (92839): 12% spell haste |
| Spirit | Empowered Mending (92843): 5% healing | Spiritual Acceleration (92842): 10% spell haste and 3 percentage points spell crit |

## Tester workflow after deployment

Right-click the selected Primary Stat in Character Advancement. Its two named weapon bonuses appear as separate certification options. The parent may already be Certified; this does not certify or disable its children. Each child uses the existing granted-ability certification namespace (0x40000000 plus spell ID), server eligibility, known-spell ownership check, acknowledgement and independent receipt/leaderboard credit. Spellbook placement does not prevent certification.

For each bonus: test the stated amount with an eligible weapon, swap to an ineligible weapon/unarmed and back, test disarm where relevant, log out/in with the path selected, then verify changing away removes the grant/effect unless independently owned. Re-select the path and choose that bonus's Certify option. Cancel sends nothing. Pending requests, unknown spells, non-Verified and already-Certified bonuses are disabled. The server remains authoritative. The existing granted catalog also supplies source/status tooltips and discoverability without a new addon or standalone ability purchase.

Measure haste using cast-time ratios (100/112 or 100/110), not percentage subtraction from cast time. Armor penetration uses the native armor-penetration calculation/cap; 20% is not a promise of 20% more damage. Heavy Swings excludes ordinary white attacks and nonphysical abilities. Agile Strikes requires an eligible damaging melee/ranged ability. Compare controlled identical events for damage/healing, accounting for other modifiers and rounding. Mixed one-/two-handed loadouts can satisfy both native equipment predicates; the tests preserve that existing behavior.

## Evidence and limits

`provenance.json` pins the deployed source/catalog/binary to the existing build-r18 and final-offline-r4 evidence. Reused amount, equipment, linked grant/path switching, Agile Strikes and Heavy Swings/Fatal Wounds checks retain their original scope. New `offline-r7/report.json` passes six suites via the existing tools/verify_all.py runner:

- 672 native weapon-family/hand/removal/disarm cases plus shield and mixed-hand controls.
- Native haste handler apply/remove, repeated roundtrip, stronger-group exclusion, and spell-crit recalculation.
- Native damage/healing/armor modifier branches with controlled aura aggregation.
- Native equipment passive restoration/removal for all eight, including loaded spells before inventory, stale/inactive/offspec rejection, duplicate application and external aura preservation.
- Actual native certification ledger/rank/confirmation parsers for eight independent IDs.
- Lua 5.1 existing certification regressions plus separate child menus, parent already Certified, confirmation/ack/pending/ownership/isolation guards and catalog mappings.

Adapters supply external engine state, serialized selections, usable equipment and aura collections; these checks are engineering evidence, not a live client or gameplay session. Existing linked-spell evidence covers parent acquisition/path-switch cleanup and independent ownership. These are single-rank passives, not invested talents. Cast travel, resource spending, procs, charges and channels are not mechanics of these passive grants; affected attack/spell cost and damage behavior is checked where relevant. Failed r3 extractor and r5 missing-host-Boost runs are superseded by r7; the existing compiler image supplied Boost in a network-disabled automatically removed compiler process. No persistent testing environment was added.

## Prepared changes and next package

`candidate/TesterCertification.lua`, `GrantedAbilityCatalog.lua`, `certification-ledger.json` and `test-access-status.json` are staged. `verification.json` records hashes and the eight Verified entries. Preserve the completed stable r3 package. A future package must rebase these scoped deltas against then-current installed files/status and worker inputs, preserve new certifications, distribute the Lua files and any tooltip changes through the launcher, and qualify production worker rendering, publication and final elevated CheckOnly. No activation-ready package is claimed here. No live SQL, restart, activation, client installation or publication was performed.
