# Male Hoplite: Hidden Chest

Adds existing vanity item 168665 (appearance 12179) to Male Hoplite Wardrobe set 90073 and shared Hoplite bundle 9901073. Female set 90273 is unchanged. Bundle redemption grants the physical item, appearance and vanity ownership through the existing handler. Existing owners can reclaim and reopen the shared bundle.

This is an Area 52 overlay patch against the existing private archetype bundle implementation, which is absent from upstream main. It is not a drop-in upstream feature. Apply bundle.patch to the matching Area 52 source. The client recipe patches only ItemSet and VanityCollection records in the effective Area 52 archive; binaries and proprietary archives are excluded from Git. Paths in the recipe refer to the owner's staging workspace.

Build and focused harness: VERIFY ALL: PASSED (build,harness; area52_hoplite_chest). Every rebuilt client archive member was compared to its expected bytes. The earlier unrelated ownership harness required unavailable Windows paths inside the build container; the focused harness replaced it. Gameplay acceptance remains separate. Source and client package are staged, not activated or promoted.
