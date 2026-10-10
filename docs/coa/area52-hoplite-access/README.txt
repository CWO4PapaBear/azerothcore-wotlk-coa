Area 52 Hoplite collection access repair

Integration delta for the private Area 52 source baseline; not a standalone upstream-main change.

Recognize validated Draconic Sinister Strike and Sweeping Swings handlers in the connected enchant catalog, retaining native stack limits. Permit only deletions from an existing enchant layout when another slot is invalid. Additions and moves still pass normal ownership, capacity, investment and stacking validation. Combat and casting restrictions remain.

The prior live activation audit marked both enchants disconnected despite VERIFIED labels. The reported character had saved three copies of Draconic Sinister Strike. Whole-layout rejection prevented aura restoration, including Hoplite equipment permission.

Verification: repair-only build and two focused harnesses PASSED; removal-only regression harness PASSED; tooltip preservation harness PASSED; C++ style checks PASSED. Native structural scenario passed and exported all three enchant connections as true. The verification driver reports INCOMPLETE because this scenario is exploratory; this is not gameplay certification.

Shared-world changes are excluded from the repair-only executable. Server activation is tracked separately from source publication and client release assets. Local Hoplite tooltip correction separately updates 34 previously audited spell/rank labels and preserves all other archive contents.
