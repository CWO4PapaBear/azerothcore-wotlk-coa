/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AscensionFreePick.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "StringFormat.h"
#include "SpellMgr.h"
#include "PlayerScript.h"
#include "Player.h"
#include "DataMap.h"
#include "Chat.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "Unit.h"
#include <list>
#include <sstream>

namespace
{
enum HeroDruidSpells : uint32
{
    VengefulSoulRestore = 816914
};

constexpr uint32 INSECT_SWARM_FLAG0 = 0x00200000;
constexpr uint32 STARFIRE_FLAG0 = 0x4;
constexpr uint32 ARCANE_BARRAGE_FLAG1 = 0x8000;
constexpr float NATURES_REACH_YARDS = 5.0f;
constexpr uint32 NATURES_REACH_TARGETS = 10;

AuraEffect const* CastersInsectSwarm(Unit* target, Unit* caster)
{
    return target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_DRUID, INSECT_SWARM_FLAG0, 0, 0,
        caster->GetGUID());
}
}

class aura_hero_druid_natures_reach : public AuraScript
{
    PrepareAuraScript(aura_hero_druid_natures_reach);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* target = eventInfo.GetProcTarget();
        return target && target != GetTarget() && CastersInsectSwarm(target, GetTarget());
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* druid = GetTarget();
        Unit* target = eventInfo.GetProcTarget();
        AuraEffect const* swarm = CastersInsectSwarm(target, druid);
        if (!swarm)
            return;
        std::list<Unit*> nearby;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, druid, NATURES_REACH_YARDS);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(target, nearby, check);
        Cell::VisitObjects(target, searcher, NATURES_REACH_YARDS);
        uint32 spread = 0;
        for (Unit* enemy : nearby)
        {
            if (enemy == target || CastersInsectSwarm(enemy, druid) || !druid->IsValidAttackTarget(enemy))
                continue;
            druid->CastSpell(enemy, swarm->GetId(), true, nullptr, aurEff);
            if (++spread >= NATURES_REACH_TARGETS && !AscensionFreePick::Applies(druid->ToPlayer()))
                break;
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_druid_natures_reach::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_hero_druid_natures_reach::HandleProc, EFFECT_1,
            SPELL_AURA_PROC_TRIGGER_SPELL);
        OnEffectProc += AuraEffectProcFn(aura_hero_druid_natures_reach::PreventSecond, EFFECT_2,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }

    void PreventSecond(AuraEffect const*, ProcEventInfo&)
    {
        PreventDefaultAction();
    }
};

class aura_hero_druid_vengeful_soul : public AuraScript
{
    PrepareAuraScript(aura_hero_druid_vengeful_soul);

    void AfterRemove(AuraEffect const*, AuraEffectHandleModes)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;
        Unit* druid = GetTarget();
        int32 const missing = int32(druid->GetMaxPower(POWER_MANA)) - int32(druid->GetPower(POWER_MANA));
        if (missing <= 0)
            return;
        druid->EnergizeBySpell(druid, VengefulSoulRestore, CalculatePct(missing, AscensionFreePick::Applies(druid->ToPlayer()) ? 10 : 5), POWER_MANA);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_hero_druid_vengeful_soul::AfterRemove, EFFECT_0,
            SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_hero_druid_starburst_barrage : public AuraScript
{
    PrepareAuraScript(aura_hero_druid_starburst_barrage);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spell = eventInfo.GetSpellInfo();
        return spell && (spell->IsAffected(SPELLFAMILY_DRUID, flag96(STARFIRE_FLAG0, 0, 0)) ||
            spell->IsAffected(SPELLFAMILY_MAGE, flag96(0, ARCANE_BARRAGE_FLAG1, 0)));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_hero_druid_starburst_barrage::CheckProc);
    }
};

namespace
{
struct Area52RangeView : DataMap::Base
{
    uint32 Cooldown = 0;
};

class area52_range_display final : public PlayerScript
{
public:
    area52_range_display() : PlayerScript("area52_range_display",
        {PLAYERHOOK_ON_UPDATE, PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT}) { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (!AscensionFreePick::Applies(player) || !player->IsInWorld() || player->GetSession()->IsHeadlessBot())
            return;
        auto* view = player->CustomData.GetDefault<Area52RangeView>("Area52RangeView");
        view->Cooldown = diff >= view->Cooldown ? 0 : view->Cooldown - diff;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& message,
        Player* receiver) override
    {
        std::string const prefix = "A52RANGE\t";
        if (type != CHAT_MSG_WHISPER || language != LANG_ADDON || receiver != player ||
            message.compare(0, prefix.size(), prefix) != 0)
            return true;
        if (!AscensionFreePick::Applies(player) || !player->IsInWorld() || message.size() > 64)
            return false;
        auto* view = player->CustomData.GetDefault<Area52RangeView>("Area52RangeView");
        if (view->Cooldown)
            return false;
        view->Cooldown = 100;
        std::istringstream input(message.substr(prefix.size()));
        uint32 serial = 0, id = 0;
        std::string extra;
        if (!(input >> serial >> id) || (input >> extra))
            return false;
        SpellInfo const* info = sSpellMgr->GetSpellInfo(id);
        if (!info)
            return false;
        bool eligible = false;
        for (uint32 talent : {16819u, 16820u, 816819u, 17322u, 17323u, 817323u})
            if (AuraEffect const* effect = player->GetAuraEffect(talent, EFFECT_0))
                if (SpellModifier* modifier = effect->GetSpellModifier())
                    eligible |= player->IsAffectedBySpellmod(info, modifier, nullptr);
        float const maximum = eligible ? info->GetMaxRange(false, player) : 0.0f;
        float const minimum = eligible ? info->GetMinRange(false) : 0.0f;
        WorldPacket packet;
        ChatHandler::BuildChatPacket(packet, CHAT_MSG_WHISPER, LANG_ADDON, player, player,
            prefix + Acore::StringFormat("{} {} {:.2f} {:.2f}", serial, id, minimum, maximum));
        player->SendDirectMessage(&packet);
        return false;
    }
};
}

void AddSC_AscensionHeroDruid()
{
    new area52_range_display();
    RegisterSpellScript(aura_hero_druid_natures_reach);
    RegisterSpellScript(aura_hero_druid_vengeful_soul);
    RegisterSpellScript(aura_hero_druid_starburst_barrage);
}
