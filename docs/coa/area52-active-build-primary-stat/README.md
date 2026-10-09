# Area 52 active-build Primary Stat and catch-up

This focused patch targets the existing private Area 52 build-storage and Free Pick implementation. Those dependencies are not present in upstream main; this is a preserved integration patch, not a standalone upstream implementation.

An active plan now selects its Primary Stat through the Free Pick advancement catalog. Strength, Agility, Intellect, Spirit and Duality map to the client-defined paths. Unsupported Stamina fails safely. Build publication accepts the existing client Duality enum value 6.

Activation with Auto-Learn off selects the stat without learning plan abilities. Restoring the active plan or changing its checkbox applies the stat while retaining existing abilities. With Auto-Learn on, activation, login, enabling the checkbox and level-up catch up the planned abilities and talents through the current level, then use the existing rank-upgrade path. Level, prerequisite, essence and rarity validation remain enforced. A failed selection does not partially apply. Automatic stat reconciliation does not charge an individual unlearn fee. Fresh activation retains the existing reset/confirmation behavior and charges.

No client UI change is required: the installed BuildCreator checkbox already sends the activation request with both auto-learn flags. Player-built plans do not grant Mystic Enchants. Featured Draft activation policy is unchanged.

Reviewed upstream main: dea3901dbf9b958f8b8bfd0963e627765e8ebe86. Local effective patch-B defines the PrimaryStat enum and C_PrimaryStat mappings; the effective Area 52 catalog supplies the five admitted paths.

Validation: VERIFY ALL: PASSED (build and freepick_advancement harness, 2026-10-09); changed-file C++ codestyle passed. The harness checks all five stat mappings, automatic level catch-up, manual restoration, future-rank exclusion, invalid/missing paths, and existing level, prerequisite and point restrictions. Report: outputs/Area52_Active_Build_Stat/verification-final/report.json in the working environment. In-game acceptance remains separate from these structural regression checks. No server restart or client-channel promotion is part of this source publication.
