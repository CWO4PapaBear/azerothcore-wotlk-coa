# Advanced training dummy test implementation

This is an independent reconstruction for Area 52 testing, not recovered Ascension server code.
No city spawns or existing training dummy bindings are replaced by this package.

## Sources

- Ascension August 20, 2023 changelog: https://ascension.gg/de/changelog/1?page=71
- Configurable Model #001 quotes: https://db.ascension.gg/?npc=967171
- Owner handoff and historical execute documentation: https://ascension.gg/en/changelog/1?page=170
- Local creature-cache research: `hertigservices/ascension-data` snapshot
  b615898e8f48b9d00ce1d35d94f7bacab28df8c9617563dc9c943d3e6106c10c.
- Independent upstream reviewed at 0facf53ae0a68369bfaac845a51a0b369c71777b.

## Implemented test behavior

- Nonlethal incoming damage, stationary combat, and per-attacker idle cleanup.
- Execute targets stay at 18% health after being attacked, then restore on disengage.
- Dynamic targets acquire the first player's level and creature-base stats; pet attacks resolve to their owner.
  A dynamic target is reserved to that player until release and resets to level 1 afterward.
- Model #001 supports information, idle vulnerability toggle, owner-only level adjustment and crowd-control
  immunity toggle. Combat end restores default crowd-control immunity.
- Healing targets accept effective healing after `/poke`, enter combat for meters, and refresh their health
  deficit. A second `/poke` stops practice and clears auras/combat. Disabled healing is zeroed.
- Tank targets use ordinary melee timing and damage, allowing the normal mitigation, rage and attack-speed
  aura paths. They do not chase. `/poke` by the owner stops practice; range/death/logout releases ownership.
  A lethal hit is capped to leave one health and stops attacks.

## Explicit limitations of this test version

- Tank simulated-death/overkill and on-death procs are **not implemented**. Calling the normal death path
  would kill players and can trigger resurrection, durability, achievement and encounter side effects.
  The safety cap must not be presented as equivalent to the historical simulated-death behavior.
- Fixed Azeroth/Outland/Northrend variants use 63/73/83 templates. Automatic realm-expansion switching
  is not yet wired; deployment must choose the appropriate tier.
- Stat magnitudes use the reconstruction's creature base-stat tables, not a recovered original damage
  formula. Tank intensity, armor, healing behavior and simultaneous-user behavior require owner acceptance.
- Healing dummies remain targetable for `/poke`; disabled healing is rejected. This is not a full client
  reproduction of the historical inactive presentation.
- A five-second idle timeout, 60-yard disengage radius, 50% healing deficit, and level-adjustment bounds
  1–83 are explicit reconstruction defaults rather than recovered constants.
- No simulated combat tests or in-game acceptance are claimed. Compile and source checks do not prove
  defensive threshold proc timing, effective healing meter behavior, or scaling on the first attack.

## Owner test checklist before city placement

Use temporary GM spawns in an isolated area. Test 666953 (single), 666925 (execute), 666928 (tank),
666935 (healing), 967254 (dynamic), 967182 (dynamic tank), and 967171 (configurable).
Repeat dynamic tests at different levels and with pets. Check independent users, loss of combat after
stopping, aura cleanup, level reset, tank rage/slows/nonlethal stop, and healing activation/meters.
No permanent placement SQL is included. Do not enable automated city replacement until behavior and
the separate surveyed placement plan have been accepted.
