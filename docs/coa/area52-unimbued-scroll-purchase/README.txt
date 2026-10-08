Area 52 Unimbued Mystic Scroll purchase

The installed client calls PurchaseMysticScroll; Area 52 did not register its native 0x611 request. CoA upstream 4580ec77be857a3194dabe54505edc15ca42f0c8 supplies the compatible 0x611/0x612 protocol and result strings. The Area 52 runtime adaptation registers its existing map-thread queue, rejects malformed requests and validates state, altar, bag capacity and funds. It stores one item 992720 before deducting its BuyPrice, then persists inventory and money and returns the native result string. Current live BuyPrice is 1000 copper (10 silver).

No client update is required. Existing Area 52 guards remain in effect; other modes are excluded by the queue gate. The patch targets the Area 52 runtime reconstruction, not bare upstream.

Verification: build and unit stages passed via verify_all.py; handler binding/guard/persistence structural harness and C++ style passed. No live click test claimed. Primary client patch-B.MPQ SlotFrameReforgeTab.lua SHA256: 399264d15f3967235b1931d36f1b773db2c8ea60e800e573725a609e8ec00732.

Implemented and built, not activated. Requires Area 52 worldserver restart with the candidate; no database migration.
