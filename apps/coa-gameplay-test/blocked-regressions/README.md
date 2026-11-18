# Blocked legacy scaling regression

`level-scaling-damage-engagement.json` is preserved byte-for-byte from the Area 52 merge candidate. The owner explicitly selected preservation as a blocked regression on November 18, 2026. It is not a passing test and is intentionally outside the runnable scenario catalog.

Its assertions require the retired realm-wide CoA.LevelScaling/CoA.LevelScalingMaxLift path: a creature changes real level from 1 to 6 before opening damage, survives the formerly lethal hit, and retains that combat level. The current merged native code has no readers for those settings. Merely accepting or deleting its unused level_scaling field cannot implement that behavior.

The separate `../scenarios/level-scaling-personal-opening-hit.json` exercises current Destiny Weaver controls and asserts real-versus-viewer level, damage on successive hits, and toggling the view off. It does not replace, satisfy or weaken the legacy real-level/lethal-hit assertions. Both runnable scenarios require the documented realm settings; offline JSON/source validation is not gameplay success.
