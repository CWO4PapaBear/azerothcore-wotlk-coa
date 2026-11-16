/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
bool WorldforgedRegistered = false;
constexpr std::array<uint32, 11> WorldforgedBuffs = {
    781000, 781300, 781600, 781900, 782200, 782201, 782500, 782501, 782800, 783100, 783101};

uint32 WorldforgedSelectedStat(Player* player)
{
    for (auto const& [spell, stat] : std::array<std::pair<uint32, uint32>, 4>{{
        {84864, STAT_STRENGTH}, {84865, STAT_AGILITY}, {84866, STAT_INTELLECT}, {84867, STAT_SPIRIT}}})
        if (player->HasAura(spell))
            return stat;
    return MAX_STATS;
}

bool WorldforgedTriggerMatches(uint32 spell, uint32 parent, std::set<uint32>& visited)
{
    if (!parent || !visited.insert(parent).second || visited.size() > 128)
        return false;
    if (sSpellMgr->GetFirstSpellInChain(spell) == sSpellMgr->GetFirstSpellInChain(parent))
        return true;
    for (uint32 rank = sSpellMgr->GetFirstSpellInChain(parent); rank; rank = sSpellMgr->GetNextSpellInChain(rank))
        if (auto info = sSpellMgr->GetSpellInfo(rank))
            for (auto const& effect : info->Effects)
                if (effect.TriggerSpell && WorldforgedTriggerMatches(spell, effect.TriggerSpell, visited))
                    return true;
    return false;
}

bool WorldforgedMatches(Player* player, SpellInfo const* spell, Area52Worldforged::Definition const& entry)
{
    for (uint32 root : entry.roots)
    {
        if (!root)
            continue;
        if (player->MatchesSpellInteraction(spell->Id, root))
            return true;
        std::set<uint32> visited;
        if (WorldforgedTriggerMatches(spell->Id, root, visited))
            return true;
        for (uint32 ancestor : player->GetSpellInteractionOrigins(spell->Id))
        {
            visited.clear();
            if (WorldforgedTriggerMatches(ancestor, root, visited))
                return true;
        }
    }
    return false;
}

int32 WorldforgedAmount(uint32 buff, uint32 level, uint32 copies)
{
    using Area52Worldforged::Scaled;
    switch (buff)
    {
        case 781000: return level * copies;
        case 781300: return Scaled(level, copies, 2, 5);
        case 781600: return Scaled(level, copies, 3, 10);
        case 782200: case 782201: case 782500: case 782501: return 3 * copies;
        case 783101: return -3 * int32(copies);
        default: return 2 * copies;
    }
}

void ConfigureWorldforged(SpellInfo* info)
{
    if (Area52Worldforged::Find(info->Id))
    {
        info->ProcFlags = 0;
        for (auto& effect : info->Effects)
            if (effect.Effect == SPELL_EFFECT_APPLY_AURA)
            {
                effect.ApplyAuraName = SPELL_AURA_DUMMY;
                effect.SpellClassMask = flag96(0, 0, 0);
                effect.TriggerSpell = 0;
            }
        return;
    }
    if (info->Id != 780999 && std::find(WorldforgedBuffs.begin(), WorldforgedBuffs.end(), info->Id) == WorldforgedBuffs.end())
        return;
    info->AttributesEx4 |= SPELL_ATTR4_AURA_EXPIRES_OFFLINE;
    info->AttributesCu |= SPELL_ATTR0_CU_FORCE_AURA_SAVING;
    if (info->Id == 780999)
        return;
    info->Effects[EFFECT_2].Effect = 0;
    if (info->Id == 781000 || info->Id == 781300)
        info->Effects[EFFECT_1].Effect = 0;
    if (info->Id == 781600)
    {
        info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_MOD_STAT;
        info->Effects[EFFECT_0].MiscValue = -1;
    }
}

void RefreshWorldforged(Player* player)
{
    if (!player || !Enabled || player->getClass() != CLASS_HERO || !Area52Worldforged::OwnerReady(player))
        return;
    for (uint32 id : WorldforgedBuffs)
    {
        Aura* buff = player->GetAura(id, player->GetGUID());
        if (!buff)
            continue;
        auto const* source = buff->GetTriggeredByAuraSpellInfo();
        Aura* enchant = source ? player->GetAura(source->Id, player->GetGUID()) : nullptr;
        if (!Applies(player) || !source || !Area52Worldforged::OwnsBuff(source->Id, id) || !enchant ||
            !AscensionMysticEnchant::GrantsActive(player, source->Id))
        {
            buff->Remove();
            continue;
        }
        if (AuraEffect* effect = buff->GetEffect(EFFECT_0))
        {
            if (id == 781600 && buff->GetScriptValue(781600) != WorldforgedSelectedStat(player) + 1)
            {
                effect->ChangeAmount(0);
                buff->SetScriptValue(781600, WorldforgedSelectedStat(player) + 1);
            }
            effect->ChangeAmount(WorldforgedAmount(id, player->GetLevel(), enchant->GetStackAmount()));
        }
    }
}

void ProcWorldforged(Player* player, Area52Worldforged::Definition const& entry)
{
    Aura* enchant = player->GetAura(entry.enchant, player->GetGUID());
    if (!enchant || player->HasAura(780999))
        return;
    uint32 const copies = enchant->GetStackAmount();
    player->CastSpell(player, 780999, true);
    if (!player->HasAura(780999))
        return;
    for (uint32 id : WorldforgedBuffs)
        if (Area52Worldforged::OwnsBuff(entry.enchant, id))
        {
            int32 amount = WorldforgedAmount(id, player->GetLevel(), copies);
            player->CastCustomSpell(player, id, &amount, nullptr, nullptr, true, nullptr, enchant->GetEffect(EFFECT_0));
        }
    using Area52Worldforged::Family;
    if (entry.family == Family::Fighter)
    {
        player->EnergizeBySpell(player, 781001, 10 * copies, POWER_ENERGY);
        player->EnergizeBySpell(player, 781002, 100 * copies, POWER_RAGE);
    }
    if (entry.family == Family::Wizard)
        player->EnergizeBySpell(player, 781301, Area52Worldforged::Scaled(player->GetCreateMana(), copies, 4, 100), POWER_MANA);
    if (entry.family == Family::Primal)
    {
        int32 heal = Area52Worldforged::Scaled(player->GetLevel(), copies, 17, 10);
        player->CastCustomSpell(player, 781601, &heal, nullptr, nullptr, true, nullptr, enchant->GetEffect(EFFECT_0));
    }
}

class WorldforgedAuras final : public UnitScript
{
public:
    WorldforgedAuras() : UnitScript("Area52WorldforgedAuras", true, {UNITHOOK_ON_AURA_APPLY, UNITHOOK_ON_AURA_REMOVE}) { }
    void OnAuraApply(Unit* unit, Aura* aura) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (player && aura && aura->GetId() >= 84864 && aura->GetId() <= 84867)
            RefreshWorldforged(player);
    }
    void OnAuraRemove(Unit* unit, AuraApplication* application, AuraRemoveMode) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (!Enabled || !player || player->getClass() != CLASS_HERO || !Area52Worldforged::OwnerReady(player) || !application ||
            application->GetBase()->GetCasterGUID() != player->GetGUID() ||
            !Area52Worldforged::Find(application->GetBase()->GetId()))
            return;
        for (uint32 id : WorldforgedBuffs)
            if (Aura* buff = player->GetAura(id, player->GetGUID()))
                if (auto source = buff->GetTriggeredByAuraSpellInfo(); source && source->Id == application->GetBase()->GetId())
                    buff->Remove();
    }
};
