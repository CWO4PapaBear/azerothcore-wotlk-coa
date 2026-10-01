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
