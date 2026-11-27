# Dragon Warrior deployed qualification

Cloud native r11 and Development native r2 passed build and unit checks. Development additionally preserves nine previously deployed preview files; those unrelated files are not in this PR.

The final focused run passed 23 suites. Extended restoration covers all 45 assigned ability selections, 26 multi-rank talents and nine enchant parents. Approved contract checks passed damage thresholds, proc eligibility, rank chances, capstone handling and six-second cooldown boundaries. Two adapter compilation failures were corrected without weakening assertions and passed their rerun. SQL-r2 passed repeat application and exact rollback while preserving unrelated bindings. PR source checks passed through tools/verify_all.py, as did separate C++ and SQL lint.

Native method adapters use bounded mocks. They establish the exercised calculations, ownership and lifecycle paths; they are not full gameplay certification. No new gameplay environment was run for publication. Earlier unrelated fixture failures were retained: freepick_rules (old essence/catalog assumptions), hero_talent_proc_flags (Reborn Omen), summoned_effects (historical SQL path), blood_presence_heal (old client shape), stock_script_shapes (Fel Armor/Drain Soul references), taught_abilities (CoA class15), area52_hoplite_chest (missing ItemSet fixture), and area52_mystic_knight_epics (stale adapter API). No claim that the full repository suite is green.

Both stable deployment receipts completed and exact running binary/status hashes were read back. Client assets and the live channel passed the actual launcher validator. The catalog worker adopted the published baseline with queued notifications/retries preserved. Player/account databases were not copied. Certified status remains tester-owned.

Investigation-to-completion wall time and flaky-test count are unknown. Owner decisions retained current Area 52 Improved Execute including its capstone, six named Bloodsurge attacks and authorized replacements, Sudden Death melee triggers with 4/8/12% chance and six-second cooldown, and current Two-Handed Weapon Mastery values. No server restart is part of this source publication.
