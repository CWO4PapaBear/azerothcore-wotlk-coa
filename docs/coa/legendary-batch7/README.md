# Area 52 Legendary batch: seven VERIFIED candidates

This patch targets the existing independent Area 52 reconstruction, not bare CoA main. Apply after the prior proc and durable Mystic grant packages. It does not import CoA class progression. Upstream reviewed: fc359be9bf79ffb532c192d7afe754be21feba40. No direct upstream repair for these five native proc enchants was found.

| Enchant | Scope |
| --- | --- |
| Eldritch Knight (81116) | Existing handlers retained; updated stale temporary-spell test to durable grants and logout guards; five Holy Wrath replacement ranks confirmed. |
| Hoplite (978217) | Existing handlers retained; replacement chains 8/12/13 ranks and shared ownership verified; existing native handler harness passed. |
| Stormbringer (965500) | Damage/actor/periodic gating; existing four-second ground damage and 25% slow confirmed. |
| Deathbringer (965501) | Single-target direct-damage restriction and three-second shared aura cooldown; native linked damage and self-heal retained. |
| Righteous Zealot (965502) | Direct-damage restriction, three-second cooldown, and 80% reduction when the triggering target is player-controlled; native party-heal target retained. |
| Purification By Light (84508) | Direct-damage restriction; connected caster-owned Holy vulnerability to strike and ground application; inherited Exorcism/Consecration modifiers and coefficient rows connected. |
| Amal'thazad's Curse (84512) | Direct-damage restriction; native buff/debuff/area-damage chain retained; explicit 0.135 AP and 0.2 SP damage coefficients from client formula. |

The five native roots retain their client proc chances. No new spellbook grants or trainable ranks are introduced for them. Eldritch Knight and Hoplite use the existing permanent grant ownership, original-ability book ranks and replacement hooks. Reconnect and combat certification are not claimed.

VERIFY ALL: PASSED for build, unit and four focused harnesses, plus the native-export catalog checks. The initial Eldritch test failed because its fixture still called an obsolete logout-removal interface; it was corrected to the production durable-grant contract. A catalog check initially requested a helper omitted from automatic graph traversal; the helper was read directly from current server DBC and confirmed to have no SQL override. Neither was an observed gameplay failure. The isolated native export passed its scenario; its runner correctly reports INCOMPLETE for combined gameplay because only an exploratory structural export was requested. No batch-sensitive or acceleration-sensitive cases; real clock used for export. No measured gameplay balance claim.

Seven yellow VERIFIED tooltip replacements are staged separately in the private client archive. Original descriptions and SHIFT sections are preserved. Client archive, personal data and database contents are excluded from Git. Deploy the binary, the one pending world migration through the normal updater, and the candidate status ledger together; install and publish the corresponding client patch only after activation. Use the required three-minute announced restart and retain rollback copies. Nothing in this source push activates the live realm or promotes the tester channel.
