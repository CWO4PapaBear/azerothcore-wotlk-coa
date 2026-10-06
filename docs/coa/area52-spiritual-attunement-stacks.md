# Spiritual Attunement equipment stacks

Mktest could collect Spiritual Attunement (965377), but confirming multiple copies rejected the entire slot update. The effective Area 52 Spell.dbc permits three stacks. The reviewed-script connection path incorrectly assigned every scripted enchant a one-copy equipment limit. This patch uses the spell's DBC stack limit for Spiritual Attunement, leaving other scripted enchant limits unchanged.

Apply integration/area52-spiritual-attunement-stacks.patch to the existing independent Free Pick runtime. That runtime file is not present in upstream main, so this focused contribution is an integration patch, not a claim that upstream main contains the complete Free Pick module. No SQL, character edits, collection grants or client changes are needed. Mktest already owns the enchant. A rebuilt Area 52 worldserver restart is required.

The focused native scenario applies three copies through the collection request near a summoned altar, asserts three stacks and a 12% aura, and checks the 3000-rune deduction. A fourth copy must reject without losing the existing three or charging again. Native outcome is recorded in the deployment verification; client visual acceptance remains separate.

Verified against upstream base afd8e940b654538f682185e9d82eaffde5bdfbf2 and the primary installed Area 52 client, whose Spell.dbc also permits three stacks. Build/unit and source stages: VERIFY ALL: PASSED. The isolated native scenario passed all 9 assertions (gameplay-20261006-141640). Aggregate gameplay: VERIFY ALL: INCOMPLETE, because only an external exploratory scenario ran. No live character state was changed. Activation requires an Area 52 restart.
