# Area-52 Season 9 Free Pick

## Advancement menu

For an Area-52 realm using the Season 9 ability and talent advancement menu, set the following
in `etc/modules/coa.conf`:

```ini
CoA.ClientBooleanConfigs = "CONFIG_LEGACY_CHARACTER_ADVANCEMENT_ENABLED=1"
```

If the comma-separated setting already includes other overrides, preserve them and replace only
`CONFIG_LEGACY_CHARACTER_ADVANCEMENT_ENABLED=0` with `CONFIG_LEGACY_CHARACTER_ADVANCEMENT_ENABLED=1`.

The client selects `Ascension_CharacterAdvancementSeason9` when this boolean is true. An explicit
value of `0` selects the newer `Ascension_CharacterAdvancement` menu instead. This is a realm-specific
override; the shared configuration default remains unchanged.

Restart the affected worldserver, fully close and reopen the client, and reconnect. Confirm that the
Season 9 ability and talent menu appears. This configuration was verified on a local Area-52 realm.

This setting selects the advancement interface only. It does not enable character-creation archetypes,
Build Draft, or their build-activation services.

## Character-creation archetype eligibility test

The reviewed client gates `C_CharacterCreate.CanCreateArchetype()` on both the exact realm name
`Area 52 - Free-Pick` and `CONFIG_CHARACTER_CREATION_ARCHETYPES_ENABLED`. To test that existing UI,
preserve the Season 9 selection and use:

```ini
CoA.ClientBooleanConfigs = "CONFIG_LEGACY_CHARACTER_ADVANCEMENT_ENABLED=1,CONFIG_CHARACTER_CREATION_ARCHETYPES_ENABLED=1"
```

Back up the realm's original name and module configuration before changing its auth database
`realmlist.name`. Restart the affected local realm services and fully restart the client. Check the
realm selection, Area-52 data selection, role/category/archetype choices, and subsequent world login.
If the name change breaks Area-52 selection, restore the original name and configuration and restart
the affected services before investigating another eligibility mechanism.

This is an experiment, not a verified complete implementation. The client calls
`C_CharacterCreate.GetArchetypeRoles()`, `GetArchetypeCategories()` and `GetArchetypes()` to populate
choices. It retains `archetypeBuildID` and requests `C_BuildCreator.ActivateBuild(buildID, true, true)`
on level-1 login. Displaying the menu does not establish that the catalog, activation protocol, or
server-side build validation and delivery work. Client acceptance of the name/configuration test
failed at realm selection: the client displayed an offline realm and an access-purchase message.
The test was rolled back to `Area-52` and the previous boolean configuration. A stale offline flag
was also found after service restart, so the screenshot alone does not isolate a realm-name or
entitlement defect. Verify both the realm database flags and client eligibility before attributing
the failure to either. Keep the original realm name while investigating an alternative UI gate.
