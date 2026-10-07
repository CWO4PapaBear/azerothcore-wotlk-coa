# Area 52 starter equipment

Free Pick Hero creation supplies a shared weapon kit: Worn Shortsword, Worn Wooden Shield, Worn Shortbow, Worn Greatsword, Bent Staff, two Worn Daggers, and Rusted Pitchfork (39202). The pitchfork has item level 7 and no required level; its inclusion is intentional for alpha polearm builds.

Every supported race receives Corsair's Vest (519243), a gray level-1 cloth chest rather than a robe. Race-specific shirts, trousers and existing boot conventions are retained; Blood Elves use Recruit's Shirt with the common chest. Existing characters are not modified.

Free Pick Heroes may equip relics regardless of their class mask. Race, level and other item requirements remain enforced. Hero projectile-ammunition exemption already exists and is not changed here.

Validation: publication source checks passed. The corresponding isolated Area 52 runtime build, unit stage and freepick_advancement harness passed, including race-kit assertions. Publication uses its existing AscensionFreepick API; the runtime uses AscensionFreePick. New-character inventory placement and relic use still require gameplay verification. No server deployment or client release is part of this source milestone.

Upstream reviewed: 413e03e986ad5d9a3e1d95f88af7effe39972b4f. The latest upstream range does not modify the affected implementation files. Client CharStartOutfit records contain no Hero starting items, so this kit is supplied by the server. This branch continues the existing Free Pick integration source history; it is not an upstream-ready isolated PR.
