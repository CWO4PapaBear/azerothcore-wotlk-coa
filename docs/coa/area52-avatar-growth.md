# Area 52 Avatar growth candidate

Avatar 86380 is the Area 52 advancement spell. Its three effects provide damage,
damage reduction and movement behavior but no size change. Client variant 1436380
contains a 30-percent scale effect; its different combat effects are not imported.

The pending world update links existing Grow 74996 to Avatar as an aura link
(type 2). Effective client/server DBC inspection found Grow has only MOD_SCALE,
base points 29 plus one die side, no visual, no trigger and unlimited duration.
Native aura-link handling removes Grow with Avatar, including cancellation and
death, and synchronizes stacks on Avatar refresh. This preserves all three
existing Avatar combat effects and composes with the native scale system.

Apply only to the Area 52 world database for this test. This is not a global
game-mode policy change or a CoA class implementation. The live Area 52 database
had no spell_dbc override, script binding or existing link for these spell IDs.
Reviewed upstream: afd8e940b654538f682185e9d82eaffde5bdfbf2.

The separate client experiment clones visual 20218 and moves kit 20386 from the
casting phase to caster impact for 86380 only. It preserves the ongoing state
effect and uses the installed warrior_avatar_cast.m2 asset. Source tooling lives
in Area52-FreePick-Client/tools/stage_avatar_visual.py. No replacement dwarf model
is selected by this experiment.

Acceptance requires casting Avatar, observing 1.3 times the original scale, and
confirming restoration on cancel/expiry/death and no accumulating growth on
reapplication. Client effect appearance also remains an in-game acceptance step.
Neither source publication nor staging activates this update.

Rollback the exact added link (86380, 74996, type 2), reload the linked-spell table
or restart, and restore the backed-up client archive. End an already active Avatar
before rollback so its linked aura can be removed normally.
