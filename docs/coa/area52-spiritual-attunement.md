# Spiritual Attunement Mystic Enchant

The live Area 52 catalog contains Rare enchant 965377 (item 1178509), with a four-percent effective-healing mana return. Its dummy aura had no script binding or proc row. Bind the existing spell_pal_spiritual_attunement handler, which rejects self-heals and zero effective healing and energizes through 31786. The proc row accepts received positive direct and periodic healing, normal and critical hits, with no cooldown.

The separately maintained Free Pick collection service rejects unreviewed dummy auras. Apply the accompanying narrow patch to that service to admit only this enchant when its handler, proc row and mana helper exist. That service is not included in upstream main; the SQL alone does not provide the collection/equipment backend. Other same-named talent and enchant IDs remain unchanged. The user-specific collection grant is intentionally excluded.

No client DBC edit is required. Deployment requires the patched equipment service binary and SQL together. Runtime validation must confirm 1000 effective allied healing returns 40 mana, full overhealing and self-healing return zero, periodic allied healing works, and unequipping stops the effect. Existing Spiritual Attunement talents must retain their behavior. Client gameplay acceptance is pending.

Verification: the Area 52 candidate passed tools/verify_all.py build, unit (one aggregate test), and five existing harnesses (Mystic Knight Epics, Eldritch Knight, Victory Rush, player combo points, Mystic Enchant catalog). These regression harnesses do not directly prove this enchant's healing proc. SQL lint and diff whitespace checks passed. Gameplay and full upstream verification were not run; no upstream PR or runtime activation is claimed.
