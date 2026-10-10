Area 52 shared-world repair batch, Part 1

Focused integration delta for the existing private Area 52 reconstruction, not a standalone upstream-main implementation. Source preconditions preserve the existing Free Pick changes. Seventeen upstream commits are listed with original PR/commit links. No CoA-class progression is imported.

Adaptations: Landslide updates the existing Landslide cast row to 45 seconds without replacing its other attacks. Hearthsinger includes the missing spawn from upstream rev_20261004_80_dungeon_ingame_spawns.sql at its recovered coordinates. SmartAI changes rewrite complete existing blocks, preserving unrelated behavior. The upstream Hour of Judgement threshold adjustment is not applicable because that added attack is absent in this baseline; it is not introduced by this repair batch.

Verification: build PASSED; 18 migrations including separately published specialized dummy placements applied twice to temporary copies of live world tables; idempotence and targeted checks PASSED. C++ lint PASSED. SQL lint reports inherited false positives for a valid multiline UPDATE and LEAST function calls; SQL execution passed. No gameplay certification is claimed. No client archives are changed by this batch. Existing upstream client package tracking gaps remain.

Temporary specialized dummy placements are a separate contribution. Activated with migration hashes and targeted rows verified; see activation.json. Apply the integration patch with git apply --ignore-space-change when using a CRLF checkout; normalized source preconditions are provided.
