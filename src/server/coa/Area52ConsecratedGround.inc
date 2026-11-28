bool Area52ConsecratedGroundRegistered = false;

bool HasArea52ConsecratedGroundHandlers()
{
    return Area52ConsecratedGroundRegistered;
}

int Area52ConsecratedGroundAmount(AuraEffect const* effect, Unit const* target)
{
    if (!effect || effect->GetId() != 965903 || !target)
        return 0;
    Unit* caster = effect->GetCaster();
    Creature* ground = caster ? caster->ToCreature() : nullptr;
    Unit* owner = ground ? ground->GetOwner() : nullptr;
    if (!ground || ground->GetEntry() != 300055 || !owner || !AscensionFreePick::Applies(owner->ToPlayer()))
        return 0;
    return target->GetGUID() == owner->GetGUID() ? 12 : 5;
}

struct npc_area52_consecrated_ground : public ScriptedAI
{
    npc_area52_consecrated_ground(Creature* creature) : ScriptedAI(creature) { }
    void IsSummonedBy(WorldObject* summoner) override
    {
        Player* player = summoner ? summoner->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(player))
            return;
        me->SetOwnerGUID(player->GetGUID());
        me->SetFaction(player->GetFaction());
        me->SetReactState(REACT_PASSIVE);
        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
        me->CastSpell(me, 965903, true);
    }
    void AttackStart(Unit*) override { }
};
