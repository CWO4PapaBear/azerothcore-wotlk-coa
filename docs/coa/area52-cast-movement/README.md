# Area 52 cast movement audit

The installed Area 52 definitions omit movement interruption on several ordinary cast-time spell families.
The core start and mid-cast movement checks require this flag. Restore it for 107 ranks in 15 families:
Lava Burst, Healing Wave, Lightning Bolt, Chain Lightning, Prayer of Healing, Inferno, Hellfire, Scorch,
Searing Pain, Soul Fire, Lesser Healing Wave, Incinerate, Binding Heal, Chaos Bolt and Hand of Gul'dan.

The comparison pinned jealous-sound main at `fc359be9bf79ffb532c192d7afe754be21feba40`.
Its core movement-permission function matches the reviewed local implementation. The owner's PTR snapshot
contains the stationary flag on all 98 overlapping records. Eight further records extend the same
Lava Burst and Incinerate rank families. Hand of Gul'dan uses the owner's explicit stationary-casting decision. Current server SQL has no overrides on the 437 reviewed records.
The primary Area 52 client and server DBC agree on the missing flags.

## Scope and results

The audit traverses ranks and effect links from already VERIFIED/CERTIFIED catalog entries, including
installed tooltip promotions newer than the ledger. Signed cast-time values prevent instant ranged attacks
from being misidentified as long casts. Passive spells are excluded; channels are checked separately.

- 74 advancement entries, 437 distinct cast/channel records.
- 328 already have movement interruption; all reviewed channels have channel movement interruption.
- 107 repaired records across the 15 families above.
- Hypnosis 955072 explicitly permits moving in its SHIFT description; preserve its CERTIFIED tooltip.
- Silencing Shot 834491 is a triggered helper of instant spell 34490; preserve its behavior.
- Hand of Gul'dan 954611 now requires stationary casting by explicit owner decision. Its VERIFIED label
  is retained after the movement checks pass. All 74 reviewed entries pass this structural check.

This is a structural audit, not simulated combat or player certification. Talent/ME aura313 movement
permissions and existing core exceptions remain operative. Neither triggered nor instant casting is
globally restricted. Previously completed connection, ownership, persistence and rank checks still apply;
this audit does not certify unrelated mechanics or add a reconnect requirement.

## Implementation and verification

`repair.patch` contains the exact candidate C++ metadata hook, explicit rank allowlist, registration and
focused verification harnesses. The hook is gated to CoA enabled, Hero model, live realm and compatible
Free Pick game-mode mask. It ORs only the movement bit and preserves channel flags and all other data.
It requires the existing independent Area 52 runtime, including its staged Legendary batch registration
context; it is **not a standalone upstream integration or a merge-ready upstream PR**.

`VERIFY ALL: PASSED` for build, unit and `area52_cast_movement` harness; the harness compiles and exercises
the production hook for all 107 IDs, configuration exclusions, repeated application and exception IDs.
`VERIFY ALL: PASSED` for `area52_cast_movement_client`; compares every archive member and every Spell row,
preserving all fields except the 107 movement bits only. Original SHIFT
content and spacing are preserved. C++ style checks passed. Gameplay was intentionally not run.

Local reports are in `outputs/Area52_Movement_Casting/verification-hand-guldan/report.json` and
`verification-client-hand-guldan/report.json`. `audit.json` records per-entry dispositions without proprietary DBC
payloads. The staging/audit scripts are reproduction helpers for the documented owner's workspace layout,
not runtime dependencies. Client archives, DBC snapshots and personal account data are excluded here.

## Activation boundary

Source publication only. The candidate server and client have **not** been activated or released to testers.
The staged client archive is based on the installed client hash recorded in `client-stage.json`; do not
overwrite other pending client changes with it. Reconcile the seven-Legendary candidate labels and this
movement patch when preparing the combined release. Likewise merge the candidate ledger changes by entry
and check, rather than replacing a newer ledger wholesale. Keep repaired entries VERIFIED only with the
matching server/client pair. Use the required three-minute restart announcement/countdown for activation.
