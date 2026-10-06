# Area 52 Eldritch Knight

Spell 81116 is the active Area 52 Mystic Enchant. Client Area 52 Spell.dbc and the effective server records contain its helpers, but neither upstream revision 36a3d8b506f8dabaee3faac3cf117a9ee2d03f79 nor the reviewed live SQL provided the proc registration or Holy Wrath replacement mapping. The local enchant system applies an aura directly, so its non-aura learn effect did not grant Eldritch Effusion.

This handler runs for Hero characters on enabled, live Free Pick realms (individual mode mask zero or 0x400). It temporarily grants Effusion and substitutes the corresponding learned Holy Wrath rank. Removing the enchant removes only temporary grants and its own weapon enchant/buffs; permanent spell ownership is preserved.

Successful, untriggered melee abilities are counted once per cast. With Eldritch Weapon they grant Knight and trigger Explosion at most once every three seconds. Knight restores 8% missing mana and builds 20 Anomaly stacks into Horror. Horror uses the client-defined defense/haste aura, Reave damage, and 3% maximum-mana restoration on melee attacks. Effusion restores 20% missing mana. Existing DBC damage values, coefficients, durations, visuals and cooldown helpers remain authoritative; no new animation assets are needed.

The three-second limit follows the active Effusion description; the hidden weapon aura has older five-second text. Horror's active text specifies maximum mana, despite the helper's older missing-mana wording. Knight and Horror retain their existing native aura stacking behavior; exact historical stacking and damage tuning remain unverified.

## Verification and deployment

The Area 52 integration candidate passed compilation, unit tests and focused regressions for temporary grants, rank removal, existing permanent ownership, cross-player/triggered-event rejection, multi-hit deduplication, three-second throttling, 20-stack promotion and missing-mana calculations. Neighboring Mystic Enchant catalog, combo-point and Victory Rush regressions passed. These are compiled isolated behavior checks, not live combat acceptance.

Apply the pending world SQL together with the new binary, retaining a backup of spell_script_names and spell_proc. No client update is required. The spell handler is separate from the collection/equipment implementation; a realm must already apply the enchant aura to the wearer.

Gameplay still must verify equipping/removing, death/relog/spec change, each Holy Wrath rank and cooldown, off-hand/multi-target behavior, temporary weapon-enchant conflicts, Knight/Horror stacking and displayed mana/visuals. Anomaly has a buff icon but no spell visual; Horror references visual 889830 and Explosion uses visual 70631, both present in the reviewed client. No claim is made about the unavailable official backend.

## Free Pick equipment integration

The independent Area 52 collection service rejects scripted enchants by default. Installing the spell handler alone does not make 81116 equipable. Apply `area52-eldritch-equipment.patch` to that service before deploying this handler there. The collection service is a separate prerequisite and is not added wholesale by this focused contribution. Upstream without that service needs its own equipment integration.

The patch admits only 81116, only with its proc entry and all eight registered script bindings available. Normal quality, passive, ownership, slot, currency, and mode restrictions remain in force. The focused harness additionally exercises every missing-binding rejection when that service source is present. It does not establish live combat or client event-loop correctness.

The first equipment test exposed a server crash: AuraScript::Load accessed GetTarget before an AuraApplication existed. The corrected handler reads GetUnitOwner with a null guard. The regression now provides an owner but no application target during Load. Live equip and combat acceptance remain pending after this repair.

Eldritch Wrath grants one Anomaly stack on AfterCast, independently of how many enemies it hits. Its existing native spell effect retains the Eldritch Knight grant. Wrath and melee share the same 20-stack Horror transition. This implements the owner's specified Area 52 behavior; it is not a claim of verified official backend behavior.
