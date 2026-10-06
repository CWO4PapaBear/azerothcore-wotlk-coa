# Area 52 Mystic Knight Epic enchants

Reviewed upstream revision: 114ffd77e534d1c1570a02914869a7a2b6007b5f. Definitions come from the owner's effective Area 52 client and independently reconstructed server, not an official Ascension backend.

- Emanating Light 926573 maps eight learned Consecration ranks to the matching moving self aura. Damage helpers award 926590 stacks. Holy Ground 81315 suppresses the replacement.
- Flesh Hook 92580 temporarily teaches 92583. This worldforged Epic is the active live catalog entry, not the CoA or later PvP variant. It excludes player targets; its delayed helper pulls a valid creature toward the caster. Damage uses the client contract: base plus six times level, 20% AP and 20% SP.
- Deconstruction 954081 maps nine learned Devastate ranks to matching Deconstruct ranks. Native weapon calculation applies Sunder; its inherited per-stack bonus is removed from these exact replacement records. The separate damage effect adds the rank flat amount, 30% SP and 19.5% AP. Spell power is 1.2 times level. At the owner's request, its duration is three seconds per Sunder stack, capped at fifteen seconds, replacing the client baseline of one second per stack.

The process gate requires CoA enabled, Hero class model and live realm type. Player script gates also require Hero and no incompatible game-mode bits. Existing permanent spell ownership is retained when temporary grants are removed. Other modes retain original metadata.

Apply the pending world SQL with the binary. The standalone collection/equipment service is a separate prerequisite; apply `area52-mystic-knight-epics.patch` to the already integrated service. It only admits these three entries when their handler bindings are registered. Do not copy the full independent service into an unrelated upstream change.

Client distribution is separate. Run `python tools/Patch-Area52DeconstructionTooltips.py Spell.dbc Spell.updated.dbc`, then replace the DBC member in the effective Area 52 archive using your archive tooling. Update Spell.dbc descriptions for 954081, 954082, 954083 and 954443 through 954450 to state three seconds per stack, up to fifteen seconds. No proprietary client archives are included in this contribution. A source push does not publish launcher assets.

Validation exercises extracted production methods with controlled players and spell data, plus compilation and existing subsystem regressions. It does not replace live player tests for moving-area geometry, combat damage/mitigation, pull immunities, models, sound, or UI replacement. An initial build caught two Unit API spelling errors; an initial harness assertion was too broad and incorrectly rejected valid Apply-hook target access. Both were corrected. Live activation and account collection grants are separate deployment steps.

The final build, one unit-test aggregate, five focused harnesses and five source suites passed. Gameplay acceptance remains pending. Emanating Light physical damage taken is corrected to minus one percent per stack; the raw client row had a positive value contrary to its description.
