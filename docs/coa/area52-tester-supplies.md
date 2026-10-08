# Area 52 Experimental Supply Crate

Area 52 alpha supplies are delivered once per character on login, including existing characters that have no delivery record. The independent `Area52.TesterCache.Enabled` setting defaults to `0`; set it to `1` in the Area 52 worldserver configuration for alpha. Turning it off stops future delivery but does not block opening previously issued crates. The existing level-10 CoA warchest is unchanged.

Custom item: `9900052`. Display: `900052`, using the existing Goblin Supplies icon `INV_Misc_BlizzCon09_GraphicsCard`. The matched Area 52 overlay must contain the new Item.dbc and ItemDisplayInfo.dbc rows; install the same tables server-side. Do not replace other contents of the mixed Area 52 overlay. No proprietary client assets are included in source.

Opening the crate uses a script, not loot-table counts. Rewards go directly to inventory; full-bag overflow is mailed. Currency/gold, inventory saves, consumed crate and overflow mail share one database transaction. The delivery record and initial mail also share one transaction. Do not truncate `area52_tester_cache_delivery` while delivery remains enabled.

Contents:
- 100,000 progression runes, item 375250 (client-facing Rune of Ascension; server item name is Rune of Descension).
- 1,000 gold.
- 500 common Unidentified Mystic Scrolls, 97866; stack size increased to 500.
- Fel Enchanted Warchest, 657112.
- QA Mystic Altar, 3648545.
- Five stackable Potion of Experience, 3818046.
- Loot-Transfigurator 5000, 190190.
- Wondrous Wisdomball, 101169.
- XP-bearing Stick on a Carrot, 339075.
- Prestigious Swift Hand of Justice, 1642991.

Innkeepers offer potion 3818046 through normal unlimited-stock vendor purchases, with a 5,000-copper (50-silver) base price and the normal faction-reputation discount. Stock is attached in memory only after an eligible Free Pick Hero opens gossip. Previously non-vendor innkeepers receive vendor capability; default-gossip non-vendors receive an explicit browse option. No persisted global npc_vendor rows are added.

Apply the two pending SQL migrations only to the Area 52 world/characters databases. The world migration changes the common scroll stack size and potion price; those data changes are database-wide. Runtime delivery, redemption, and vendor injection are Free Pick Hero gated. Bear Cave PTR is outside this package.

Implementation was compiled and unit-tested in the active Area 52 integration tree. This publication tree uses the corresponding `AscensionFreepick::IsFreepickHero` predicate; the active runtime tree calls `AscensionFreePick::Applies`. This branch is based on the owner's existing Area 52 integration history, not a clean upstream-main PR.

Activation and launcher client-asset publication are separate from this source branch. Runtime verification results are recorded when complete; do not infer readiness from compilation alone.
