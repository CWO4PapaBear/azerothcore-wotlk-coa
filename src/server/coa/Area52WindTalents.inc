namespace
{
void ConfigureArea52WindTalents(SpellInfo* info)
{
    if (Area52WindMetadataEnabled && info->Id == 57842)
    {
        info->AttributesEx3 |= SPELL_ATTR3_SUPPRESS_CASTER_PROCS;
        info->AttributesEx4 |= SPELL_ATTR4_SUPPRESS_WEAPON_PROCS;
    }
}
}
