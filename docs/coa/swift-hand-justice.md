# Swift Hand of Justice healing

The Area 52 tester supplies grant item 1642991. It equips aura 59906 and XP bonus 57353.
The existing `spell_item_swift_hand_justice_dummy` handler casts heal 59913 for 2% of maximum health.
Both the effective Spell.dbc and SQL proc row supplied zero trigger flags, preventing the handler from firing.

The pending world migration restores `PROC_FLAG_KILL` (2), preserving the existing XP/honor requirement
(`AttributesMask` 1). The DBC's 100% chance remains authoritative. This also repairs stock item 42991,
which uses the same aura. No C++ handler or XP bonus changes are required.

Reviewed community upstream revision: `fc359be9bf79ffb532c192d7afe754be21feba40`.
Upstream's `rev_20260916_70_discerning_eye_kill_proc.sql` addresses the corresponding mana trinket.

Focused verification passed: migration replay, exact row scope, preserved XP/honor gate, and isolated client
visual change. No live combat healing measurement has been performed. Source publication does not activate
the database migration or release a client package.

The separately staged Area 52 client overlay clears Spell.dbc record 59906's first SpellVisual field
(7578 to 0). It preserves every other field and archive member, including heal 59913 and XP bonus 57353.
This disables the equip visual without altering the shared SpellVisual definition. Proprietary client data
is not included in this source contribution.
