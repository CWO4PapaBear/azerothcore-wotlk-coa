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

### Local client alias test

The next isolated test retains `Area-52` and the two boolean settings above. In the installed
client's `Interface/GlueXML/CharacterCreate.lua`, the eligibility predicate becomes:

```lua
return (GetRealmName() == "Area 52 - Free-Pick" or GetRealmName() == "Area-52") and C_Config.GetBoolConfig("CONFIG_CHARACTER_CREATION_ARCHETYPES_ENABLED")
```

The test uses a loose override extracted from that client's own archive; no client assets are
distributed here. It requires client support for loose interface overrides. Preserve any existing
override before testing. Removing only the newly installed override restores archive behavior;
restore the previous boolean configuration to disable the server-side test setting.

UI loading, catalog population, and build activation remain unverified. Do not deploy this
experiment as a completed archetype implementation.

The alias test reached character selection but crashed when opening new-character creation.
The supplied report identifies an access violation at `0x004E204F`, writing to `0xFFFFFEB0`,
inside native `ResetCharCustomize()` called by `CharacterCreate_OnShow` at line 1517.
This occurs before role-list population and does not establish a build-activation server defect.
The experimental loose override was removed and the archetype flag disabled; `Area-52` and
the Season 9 menu were preserved. Baseline character creation must be retested, then effective
race/class data and the native customization path investigated before another UI activation.

## Native realm identity investigation

Reviewed upstream revision: `381e3305` (October 1, 2026). Keep these identifiers distinct:

| Value | Observed use |
| --- | --- |
| `Area 52 - Free-Pick` | Exact realm-name gate in `C_CharacterCreate.CanCreateArchetype()` |
| `Area52` | Artwork key in the client's fallback realm-card table |
| `area-52` | Installed client overlay directory; activation mapping still needs verification |
| `Area-52` | Locally tested working auth realm name |

The authserver's `BuildRealmCardName` uses the actual realm name as its card lookup key and sends
an explicit unlocked field of `1`. Client `RealmList.lua` joins cards to real realm entries by exact
name and splits the name at a hyphen only for presentation. Its fallback table is conditional on
the absence of `C_RealmSelect`, so that table alone does not establish native DLL behavior.

`AscensionCompat.cpp` sends separate data-path and realm-name strings in `SMSG_REALM_INFO`.
The data-path string is currently empty. The advancement menu is independently selected by the
legacy boolean above; the archetype gate needs both its boolean and the full realm name.

Before another native-name test, trace the installed DLL's data-directory selection, verify the
effective race/class and archetype tables, and inspect the actual card's unlocked field and realm
online flags. Do not equate the artwork key with the overlay directory or enable a data-path switch
without verifying the DLL's behavior. The intended permanent solution should use the native identity
and configuration mechanisms where supported, rather than an unverified loose UI alias.

### Native configuration test

The next test removes the loose client alias and sets the real auth database realm name to
`Area 52 - Free-Pick`. In `authserver.conf`:

```ini
RealmCards.Enable = 1
RealmCards.GameMode = 0
RealmCards.Image = "Area52"
```

In `etc/modules/coa.conf`:

```ini
CoA.ClientBooleanConfigs = "CONFIG_LEGACY_CHARACTER_ADVANCEMENT_ENABLED=1,CONFIG_CHARACTER_CREATION_ARCHETYPES_ENABLED=1"
```

Preserve other client overrides when applying these values. Back up both configuration files and
the old realm name. Start authentication before the worldserver: authserver startup explicitly marks
realms offline. Restarting authentication after an already-ready worldserver requires checking and
restoring the online flag for that verified-running realm. A listening socket alone is not proof that the realm card
is available. Do not change realm IDs, account data, client overlay paths or unrelated realm services.

The inspected client archive contains 3 archetype roles, 9 categories and 56 archetypes. Their presence
does not establish native DLL loading or build delivery. This configuration still requires client
acceptance: banner art, realm entry, Season 9 menu, new-character creation without a crash, and populated
archetype choices. Keep build activation unverified until its server-side behavior is tested.
