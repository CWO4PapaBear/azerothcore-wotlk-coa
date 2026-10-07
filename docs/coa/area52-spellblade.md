# Area 52 Spellblade repair

Spellblade ranks 812575–812577 promise a melee-triggered mana restoration and Replenishment. The reviewed Area 52 client defines chances of 4%, 8% and 12%, and base-mana amounts of 7%, 14% and 20%. Its third rank also promises melee weaving without resetting the swing timer.

The effective server had no proc entries for ranks two and three. Rank one's generated flags excluded ordinary melee swings and included unrelated spell events. The pending SQL supplies melee-auto and melee-ability damage flags for all three ranks, retaining each rank's own chance and its existing effect payloads. It does not increase those chances or add a second mana/healing calculation.

The cast completion path now preserves main-hand and off-hand timers while the rank-three aura is present. Ranged timer resets remain unchanged. Rank two and removal of rank three retain ordinary cast behavior.

## Verification

Reviewed upstream base: `afd8e940b654538f682185e9d82eaffde5bdfbf2`.

- Before repair, the native Hero test kept the rank-three aura active but observed zero Spellblade procs during 90 seconds of melee testing.
- After repair, `area52-spellblade-melee` verifies a landed melee attack, a Spellblade proc, casts of both existing helpers, and restored mana. It ends on success; its maximum wait allows for random proc timing.
- `area52-spellblade-swing-timers` compares ordinary casting, rank two, rank three, and removal of rank three, and checks that ranged timers still reset.
- Both scenarios passed on the independent Area 52 runtime with its effective client/server data and a real clock. The broader local six-scenario run also passed companion acquisition, Hero parry, ME grant reconciliation/book upgrades, and periodic wards.
- Focused source verification against the pinned upstream base passed all 15 selected checks in a Linux verification checkout. Windows execution failed because its temporary test directories were inaccessible; the Windows-formatted worktree pointer also required a separate Linux checkout.

These results do not claim a build or gameplay run of upstream main with this patch alone. The complete Area 52 runtime contains additional independently maintained integration. The source patch has been isolated from that work, but upstream runtime compatibility still needs its own build and configured Free Pick test environment before merge readiness is claimed. No production restart, client release, or launcher promotion is part of this source update.

No new client assets are required for the reviewed client definitions. No CoA class progression, Wildcard roll rules, character records, or spell ownership rules are changed by this patch. Player logout/reconnect and multiplayer acceptance of this particular patch have not been separately exercised. No changed-input-free flaky outcome was observed during this repair; the proc test remains probabilistic. Total investigation time and human gameplay acceptance are not measured here.
