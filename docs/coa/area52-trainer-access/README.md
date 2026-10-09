# Area 52 trainer and approved-learning integration

Ports the relevant trainer architecture from upstream PR #6910, commit 784142dafb3ad10864d971ab61faad02e68227df: Hero rank-trainer recognition and refresh after purchase. Keeps Area 52 progression and free rank training isolated. Reviewed upstream main through 6b853197b41e13a744334a7e3f7e0a432ad23cb8.

Class trainers and clicked training books expose eligible learned-ability rank ladders, including future ranks. Companion books still upgrade automatically on summon/level-up without opening a menu; the existing training visuals remain. Beginner books retain their below-level-10 restriction. Selected talent/ability grants and active Mystic Enchant grants are included, without training talent ranks as ordinary spell ranks.

The authority guard blocks new outside-source class abilities while retaining racial and explicitly approved learning scopes. Login cleanup follows rewarded quest spell/trigger/learn graphs and removes class spells without current approved ownership. It preserves Advancement and active ME grants. Arbitrary script-only historical grants without quest spell data cannot be attributed by this cleanup.

No mailbox or other world placements. PR #6910 supplies no spawn locations. This patch targets the preserved private Area 52 integration; it is not a standalone cherry-pick to upstream main. Other staged changes are excluded.

VERIFY ALL: PASSED (build, unit, area52_trainer_access harness); combined activation verification also passed all seven selected harnesses. Coverage includes production rank traversal, learning authority, approved grants, quest cleanup/idempotence and book/menu integration. Gameplay and reconnect certification remain player acceptance. No live behavior is claimed by these source checks.
