#include "AscensionFreePick.h"
/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Config.h"
#include "AllSpellScript.h"
#include "DBCStores.h"
#include "Area52HolyContracts.h"
#include "AscensionMysticEnchantCatalog.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Area52CryotherapistRules.h"
#include "Area52WizardFrostboltRules.h"
#include "Area52Worldforged.h"
#include "Area52WorldforgedCatalog.h"
#include <set>
#include "Random.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "WorldSession.h"
#include <algorithm>
#include <array>

namespace
{
bool Enabled = false;
constexpr std::array<uint32, 8> CryoPenance = {47540, 302000, 302001, 302002, 302003, 53005, 53006, 53007};
constexpr std::array<uint32, 8> IcyPenance = {92754, 302039, 302040, 302041, 302042, 92755, 92756, 92757};
constexpr uint32 Light = 926573, Hook = 92580, Deconstruction = 954081, GhostlyBlow = 983240;
constexpr uint32 Exorcist = 986280;
constexpr std::array<uint32, 13> HeroicStrike = {78, 284, 285, 1608, 11564, 11565, 11566, 11567,
    25286, 29707, 30324, 47449, 47450};
constexpr std::array<uint32, 10> ExorcistSlash = {986281, 986347, 986348, 986349, 986350, 986351,
    986352, 986353, 986354, 986355};
constexpr std::array<uint32, 8> Consecration = {26573, 20116, 20922, 20923, 20924, 27173, 48818, 48819};
constexpr std::array<uint32, 9> Devastate = {20243, 302035, 302036, 302037, 302038, 30016, 30022, 47497, 47498};
constexpr std::array<uint32, 9> Deconstruct = {954082, 954443, 954444, 954445, 954446, 954447, 954448, 954449, 954450};
bool Applies(Player const* player)
{
    if (!Enabled || !player || player->getClass() != CLASS_HERO)
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~uint32(0x400));
}
int32 DeconstructionDuration(uint32 stacks)
{
    return 3000 * std::min<uint32>(stacks, 5);
}
void Replacement(Player* player, uint32 original, uint32 replacement, bool active, uint32 enchant)
{
    bool const replacing = player->GetTemporarySpellReplacement(original) == replacement;
    AscensionMysticEnchant::SetGrantedSpell(player, enchant, replacement, active && player->HasSpell(original), false);
    if (active && player->HasActiveSpell(original))
    {
        player->SetTemporarySpellReplacement(original, replacement);
    }
    else
    {
        if (replacing)
            player->SetTemporarySpellReplacement(original, 0);
    }
}
void SynchronizeCryotherapist(Player* player)
{
    bool cryotherapist = Applies(player) && AscensionMysticEnchant::GrantsActive(player, 86576);
    for (std::size_t i = 0; i < CryoPenance.size(); ++i)
        Replacement(player, CryoPenance[i], IcyPenance[i], cryotherapist, 86576);
    if (!cryotherapist && Applies(player))
    {
        player->RemoveAurasDueToSpell(92753, player->GetGUID());
        player->RemoveAurasDueToSpell(92768, player->GetGUID());
        std::vector<Unit*> marked;
        for (Aura* aura : player->GetSingleCastAuras())
            if (aura->GetId() == 92752 && aura->GetCasterGUID() == player->GetGUID())
                marked.push_back(aura->GetUnitOwner());
        for (Unit* target : marked)
            if (target)
                target->RemoveAurasDueToSpell(92752, player->GetGUID());
    }
}
void Synchronize(Player* player)
{
    if (!player || !player->GetSession() || player->GetSession()->PlayerLogout())
        return;
    SynchronizeCryotherapist(player);
    bool slash = Applies(player) && AscensionMysticEnchant::GrantsActive(player, Exorcist);
    bool ownsHeroic = std::any_of(HeroicStrike.begin(), HeroicStrike.end(),
        [player](uint32 id) { return player->HasActiveSpell(id); });
    uint32 selected = 0;
    if (slash && ownsHeroic)
        for (uint32 id : ExorcistSlash)
            if (SpellInfo const* info = sSpellMgr->GetSpellInfo(id))
                if (info->SpellLevel <= player->GetLevel())
                    selected = id;
    for (uint32 id : ExorcistSlash)
        AscensionMysticEnchant::SetGrantedSpell(player, Exorcist, id, id == selected);
    for (uint32 id : HeroicStrike)
    {
        if (std::find(ExorcistSlash.begin(), ExorcistSlash.end(),
            player->GetTemporarySpellReplacement(id)) != ExorcistSlash.end())
            player->SetTemporarySpellReplacement(id, 0);
    }
    bool light = Applies(player) && AscensionMysticEnchant::GrantsActive(player, Light) && !player->HasAura(81315);
    if (!light && Applies(player))
    {
        player->RemoveAurasDueToSpell(926590, player->GetGUID());
        for (uint32 id = 926574; id <= 926588; id += 2)
            player->RemoveAurasDueToSpell(id, player->GetGUID());
    }
    bool deconstruct = Applies(player) && AscensionMysticEnchant::GrantsActive(player, Deconstruction);
    bool hook = Applies(player) && AscensionMysticEnchant::GrantsActive(player, Hook);
    for (std::size_t i = 0; i < Consecration.size(); ++i)
        Replacement(player, Consecration[i], 926574 + uint32(i) * 2, light, Light);
    for (std::size_t i = 0; i < Devastate.size(); ++i)
        Replacement(player, Devastate[i], Deconstruct[i], deconstruct, Deconstruction);
    AscensionMysticEnchant::SetGrantedSpell(player, Hook, 92583, hook);
    Replacement(player, 14251, 983241,
        Applies(player) && AscensionMysticEnchant::GrantsActive(player, GhostlyBlow), GhostlyBlow);
}
#include "Area52GlacialLight.inc"
#include "Area52StayFrosty.inc"
#include "Area52CosmicReprisal.inc"
#include "Area52Worldforged.inc"
class EpicWorld final : public WorldScript
{
public:
    EpicWorld() : WorldScript("Area52EpicWorld") { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};
void ConfigureNimbleMind(SpellInfo* info)
{
    if (info->Id == 978701)
    {
        info->StancesNot = 0xFFFFFFFF;
        info->SpellFamilyName = SPELLFAMILY_MAGE;
        info->SpellFamilyFlags[0] |= 0x10000000;
    }
}
class EpicContracts final : public GlobalScript
{
public:
    EpicContracts() : GlobalScript("Area52EpicContracts") { }
    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!Enabled)
            return;
        ConfigureNimbleMind(info);
        ConfigureStayFrosty(info);
        ConfigureCosmicReprisal(info);
        ConfigureWorldforged(info);
        if (info->Id == 983723)
            info->AuraInterruptFlags &= ~AURA_INTERRUPT_FLAG_LEAVE_COMBAT;
        if (info->Id == 35395 || (info->Id >= 35601 && info->Id <= 35607))
            info->SchoolMask = SPELL_SCHOOL_MASK_NORMAL;
        if (info->Id == 986281 || (info->Id >= 986347 && info->Id <= 986355))
        {
            info->SchoolMask = SPELL_SCHOOL_MASK_NORMAL | SPELL_SCHOOL_MASK_HOLY;
            info->CasterAuraSpell = 0;
        }
        if (info->Id == 965983)
        {
            info->ProcFlags = 0;
            info->ProcCharges = 0;
            info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
            info->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_ABILITY_IGNORE_AURASTATE;
            info->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
            info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_ADD_FLAT_MODIFIER;
            info->Effects[EFFECT_2].BasePoints = 99;
            info->Effects[EFFECT_2].DieSides = 1;
            info->Effects[EFFECT_2].MiscValue = SPELLMOD_CRITICAL_CHANCE;
            info->Effects[EFFECT_2].TargetA = TARGET_UNIT_CASTER;
            info->Effects[EFFECT_2].SpellClassMask = flag96();
        }
        if (info->Id == 986282)
            info->Effects[EFFECT_1].Effect = 0;
        if (info->Id == 54748)
        {
            info->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
            info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_DUMMY;
            info->Effects[EFFECT_2].BasePoints = -1;
            info->Effects[EFFECT_2].DieSides = 1;
            info->Effects[EFFECT_2].TargetA = TARGET_UNIT_CASTER;
        }
        if (info->Id == 20207)
            info->Effects[EFFECT_2].Effect = 0;
        if (info->Id == 411135)
        {
            info->ProcFlags = 0;
            info->ProcCharges = 0;
        }
        if (info->Id == 411134)
            info->Effects[EFFECT_0].SpellClassMask = flag96();
        if (info->Id == 20216)
        {
            info->Effects[EFFECT_1].Effect = 0;
            for (auto& effect : info->Effects)
                effect.SpellClassMask = flag96();
            info->ProcFlags |= PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS | PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS |
                PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
        }
        if (info->Id == 926590)
            info->Effects[1].BasePoints = -2;
        if (info->Id >= 926575 && info->Id <= 926589 && (info->Id % 2) == 1)
            info->Effects[0].BonusMultiplier = 0.0264f;
        if (info->Id == 92583)
            info->Effects[0].BonusMultiplier = 0.2f;
        auto const rank = std::find(Deconstruct.begin(), Deconstruct.end(), info->Id);
        if (rank != Deconstruct.end())
        {
            constexpr std::array<int32, 9> points = {14, 20, 29, 43, 64, 95, 140, 208, 309};
            info->Effects[1].BasePoints = points[rank - Deconstruct.begin()];
            info->Effects[1].BonusMultiplier = 0.3f;
            info->Effects[2].BasePoints = -1;
        }
    }
};
class NimbleMindForms final : public UnitScript
{
public:
    NimbleMindForms() : UnitScript("Area52NimbleMindForms", true, {UNITHOOK_ON_AURA_APPLY}) { }
    void OnAuraApply(Unit* unit, Aura* aura) override
    {
        Player* player = unit ? unit->ToPlayer() : nullptr;
        if (!Applies(player) || !aura)
            return;
        if (aura->GetSpellInfo()->HasAura(SPELL_AURA_MOD_SHAPESHIFT) ||
            (aura->GetId() == 978701 && player->GetShapeshiftForm() != FORM_NONE))
            player->RemoveAurasDueToSpell(978701);
    }
};
class EpicPlayer final : public PlayerScript
{
public:
    EpicPlayer() : PlayerScript("Area52EpicPlayer") { }
    void Refresh(Player* player, uint32 spell)
    {
        if (Applies(player) && (std::find(CryoPenance.begin(), CryoPenance.end(), spell) != CryoPenance.end() || spell == 14251 || std::find(HeroicStrike.begin(), HeroicStrike.end(), spell) != HeroicStrike.end() ||
            std::find(Consecration.begin(), Consecration.end(), spell) != Consecration.end() ||
            std::find(Devastate.begin(), Devastate.end(), spell) != Devastate.end()))
        {
            Synchronize(player);
            RefreshWorldforged(player);
        }
    }
    void OnPlayerLevelChanged(Player* player, uint8) override
    {
        if (Applies(player))
        {
            Synchronize(player);
            RefreshWorldforged(player);
        }
    }
    void OnPlayerLearnSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
    void OnPlayerForgotSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
};
class aura_area52_epic_equipment : public AuraScript
{
    PrepareAuraScript(aura_area52_epic_equipment);
    bool Load() override { return GetUnitOwner() && Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({92583, 9654556, 926590, 954083, 58567}) &&
            ValidateSpellInfo(Consecration) && ValidateSpellInfo(Devastate) && ValidateSpellInfo(Deconstruct);
    }
    void Apply(AuraEffect const*, AuraEffectHandleModes) { Synchronize(GetTarget()->ToPlayer()); }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetTarget()->ToPlayer();
        Synchronize(player);
        RefreshWorldforged(player);
        if (GetId() == Light)
        {
            player->RemoveAurasDueToSpell(926590, player->GetGUID());
            for (uint32 id = 926574; id <= 926588; id += 2)
                player->RemoveAurasDueToSpell(id, player->GetGUID());
        }
        if (GetId() == Deconstruction)
            player->RemoveAurasDueToSpell(954083, player->GetGUID());
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_epic_equipment::Apply, EFFECT_0,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_epic_equipment::Remove, EFFECT_0,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};
class spell_area52_sanctity_trigger : public SpellScript
{
    PrepareSpellScript(spell_area52_sanctity_trigger);
    bool checked = false;
    bool Load() override { return Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({965983}); }
    void Hit()
    {
        if (checked || GetHitDamage() <= 0)
            return;
        checked = true;
        for (uint32 id : {35397u, 35396u, 32043u})
            if (Aura* talent = GetCaster()->GetAura(id))
            {
                if (roll_chance_i(talent->GetSpellInfo()->ProcChance))
                    GetCaster()->CastSpell(GetCaster(), 965983, true);
                break;
            }
    }
    void Register() override { AfterHit += SpellHitFn(spell_area52_sanctity_trigger::Hit); }
};
class aura_area52_focus_immunity : public AuraScript
{
    PrepareAuraScript(aura_area52_focus_immunity);
    bool active = false;
    bool Load() override { return Applies(GetUnitOwner()->ToPlayer()); }
    void Apply(AuraEffect const* effect, AuraEffectHandleModes)
    {
        active = effect->GetAmount() == 1;
        if (active)
            Change(true);
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        if (active)
            Change(false);
    }
    void Change(bool apply)
    {
        for (uint32 mechanic = 1; mechanic < MAX_MECHANIC; ++mechanic)
            if ((Area52Holy::FocusMechanics & (uint64(1) << mechanic)) &&
                mechanic != MECHANIC_SILENCE && mechanic != MECHANIC_INTERRUPT)
                GetTarget()->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, mechanic, apply);
        GetTarget()->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, apply);
        GetTarget()->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, apply);
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_focus_immunity::Apply, EFFECT_2,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_focus_immunity::Remove, EFFECT_2,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
    }
};
class aura_area52_lights_grace : public AuraScript
{
    PrepareAuraScript(aura_area52_lights_grace);
    bool Load() override { return Applies(GetUnitOwner()->ToPlayer()); }
    bool Check(ProcEventInfo& event)
    {
        SpellInfo const* info = event.GetSpellInfo();
        if (!info)
            return false;
        Player const* actor = event.GetActor() ? event.GetActor()->ToPlayer() : nullptr;
        return actor && (actor->MatchesSpellInteraction(info->Id, 635) ||
            actor->MatchesSpellInteraction(info->Id, 596));
    }
    void Register() override { DoCheckProc += AuraCheckProcFn(aura_area52_lights_grace::Check); }
};
class spell_area52_exorcist_boon : public SpellScript
{
    PrepareSpellScript(spell_area52_exorcist_boon);
    bool Load() override { return Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({986283, 986284, 986285}); }
    void Select(SpellEffIndex index)
    {
        PreventHitDefaultEffect(index);
        Unit* caster = GetCaster();
        uint32 helper = 986283 + urand(0, 2);
        caster->m_Events.AddEventAtOffset([caster, helper]()
        {
            if (caster->IsAlive())
                caster->CastSpell(caster, helper, true);
        }, Milliseconds(1));
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_exorcist_boon::Select, EFFECT_0,
            SPELL_EFFECT_ASCENSION_TRIGGER_RANDOM_SPELL);
    }
};
class spell_area52_exorcist_cooldown : public SpellScript
{
    PrepareSpellScript(spell_area52_exorcist_cooldown);
    bool handled = false;
    bool Load() override { return Applies(GetCaster()->ToPlayer()); }
    void Reduce(SpellEffIndex index)
    {
        PreventHitDefaultEffect(index);
        if (handled)
            return;
        handled = true;
        Player* player = GetCaster()->ToPlayer();
        int32 delta = GetEffectValue();
        if (!player || delta >= 0)
            return;
        uint32 root = GetSpellInfo()->Id == 986284 ? 53385 : 20271;
        SpellInfo const* original = sSpellMgr->GetSpellInfo(root);
        if (!original)
            return;
        std::set<uint32> spells = {root};
        auto category = sSpellsByCategoryStore.find(original->GetCategory());
        if (category != sSpellsByCategoryStore.end())
            for (auto const& [item, id] : category->second)
                if (!item)
                    if (SpellInfo const* info = sSpellMgr->GetSpellInfo(id))
                        if (info->SpellFamilyName == original->SpellFamilyName)
                            spells.insert(id);
        for (auto const& [id, entry] : player->GetSpellMap())
        {
            if (entry->State == PLAYERSPELL_REMOVED)
                continue;
            auto origins = player->GetSpellInteractionOrigins(id);
            if (std::any_of(origins.begin(), origins.end(),
                [&spells](uint32 origin) { return spells.contains(origin); }))
                spells.insert(id);
        }
        for (uint32 id : spells)
        {
            uint32 remaining = player->GetSpellCooldownDelay(id);
            if (!remaining)
                continue;
            if (uint64(-int64(delta)) >= remaining)
                player->RemoveSpellCooldown(id, true);
            else
                player->ModifySpellCooldown(id, delta);
        }
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_exorcist_cooldown::Reduce, EFFECT_ALL,
            SPELL_EFFECT_ASCENSION_MODIFY_COOLDOWN);
    }
};
class spell_area52_exorcist_slash : public SpellScript
{
    PrepareSpellScript(spell_area52_exorcist_slash);
    bool Load() override { return Applies(GetCaster()->ToPlayer()); }
    uint32 artOfWar = 0;
    SpellCastResult Check()
    {
        artOfWar = 0;
        for (uint32 id : {853489u, 59578u, 53489u})
            if (GetCaster()->HasAura(id))
            {
                artOfWar = id;
                return SPELL_CAST_OK;
            }
        return SPELL_FAILED_CASTER_AURASTATE;
    }
    void Consume()
    {
        if (artOfWar)
            GetCaster()->RemoveAurasDueToSpell(artOfWar);
    }
    void Cast()
    {
        Unit* target = GetExplTargetUnit();
        if (target && target->IsPlayer())
        {
            Player* player = GetCaster()->ToPlayer();
            player->ModifySpellCooldown(GetSpellInfo()->Id, player->GetSpellCooldownDelay(GetSpellInfo()->Id));
        }
    }
    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_exorcist_slash::Check);
        OnCast += SpellCastFn(spell_area52_exorcist_slash::Consume);
        AfterCast += SpellCastFn(spell_area52_exorcist_slash::Cast);
    }
};
class spell_area52_emanate_damage : public SpellScript
{
    PrepareSpellScript(spell_area52_emanate_damage);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Light); }
    void Hit()
    {
        if (GetHitDamage() > 0 && !GetCaster()->HasAura(81315))
            GetCaster()->CastSpell(GetCaster(), 926590, true);
    }
    void Damage(SpellEffIndex)
    {
        SetEffectValue(GetEffectValue() + int32(0.022f * GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK)));
    }
    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_emanate_damage::Damage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterHit += SpellHitFn(spell_area52_emanate_damage::Hit);
    }
};
class spell_area52_flesh_hook : public SpellScript
{
    PrepareSpellScript(spell_area52_flesh_hook);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Hook); }
    SpellCastResult Check()
    {
        Unit* target = GetExplTargetUnit();
        return target && target->IsPlayer() ? SPELL_FAILED_BAD_TARGETS : SPELL_CAST_OK;
    }
    void Damage(SpellEffIndex)
    {
        SetEffectValue(GetEffectValue() + int32(0.2f * GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK)));
        if (float extra = AscensionFreePick::AgilityAttackPower(GetCaster()->ToPlayer(), false); extra > 0)
            GetSpell()->SetScriptValue(AscensionFreePick::AgilityEffectDamageKey, uint32(0.2f * extra));
    }
    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_flesh_hook::Check);
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_flesh_hook::Damage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
class spell_area52_flesh_pull : public SpellScript
{
    PrepareSpellScript(spell_area52_flesh_pull);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Hook); }
    void Pull(SpellEffIndex index)
    {
        PreventHitDefaultEffect(index);
        Unit* target = GetHitUnit();
        if (!target || target->IsPlayer() || !target->IsAlive() || target->HasUnitState(UNIT_STATE_ROOT))
            return;
        Unit* caster = GetCaster();
        float speed = 50.0f;
        float vertical = target->GetDistance(caster) / speed * 0.5f * 19.291105f;
        target->GetMotionMaster()->MoveJump(caster->GetPositionX(), caster->GetPositionY(),
            caster->GetPositionZ() + 1.0f, speed, vertical);
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_flesh_pull::Pull, EFFECT_0, SPELL_EFFECT_PULL_TOWARDS_DEST);
    }
};
class spell_area52_deconstruct : public SpellScript
{
    PrepareSpellScript(spell_area52_deconstruct);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Deconstruction); }
    void Damage(SpellEffIndex)
    {
        SetEffectValue(GetEffectValue() + int32(0.195f * GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK)));
        if (float extra = AscensionFreePick::AgilityAttackPower(GetCaster()->ToPlayer(), false); extra > 0)
            GetSpell()->SetScriptValue(AscensionFreePick::AgilityEffectDamageKey, uint32(0.195f * extra));
    }
    void Hit()
    {
        Unit* target = GetHitUnit();
        if (!target || GetHitDamage() <= 0)
            return;
        if (Aura* sunder = target->GetAura(58567))
            if (Aura* buff = GetCaster()->AddAura(954083, GetCaster()))
            {
                int32 duration = DeconstructionDuration(sunder->GetStackAmount());
                buff->SetMaxDuration(duration);
                buff->SetDuration(duration);
            }
    }
    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_deconstruct::Damage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterHit += SpellHitFn(spell_area52_deconstruct::Hit);
    }
};
class aura_area52_deconstruction_power : public AuraScript
{
    PrepareAuraScript(aura_area52_deconstruction_power);
    bool Load() override { return GetUnitOwner() && Applies(GetUnitOwner()->ToPlayer()); }
    void Amount(AuraEffect const*, int32& amount, bool&)
    {
        amount = int32(GetUnitOwner()->GetLevel() * 1.2f);
    }
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_area52_deconstruction_power::Amount,
            EFFECT_1, SPELL_AURA_ASCENSION_MOD_SPELL_POWER_FLAT);
    }
};

class aura_area52_ghostly_blow : public AuraScript
{
    PrepareAuraScript(aura_area52_ghostly_blow);
    bool Load() override { return GetCaster() && Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({983240, 983241, 983242, 14251}); }
    void Tick(AuraEffect const*)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || !player->IsAlive() || !target->IsAlive() ||
            !AscensionMysticEnchant::GrantsActive(player, GhostlyBlow) || !player->HasSpell(14251))
        {
            PreventDefaultAction();
            Remove();
            return;
        }
        if (!player->IsWithinDistInMap(target, GetSpellInfo()->GetMaxRange(false)))
        {
            PreventDefaultAction();
            return;
        }
        GetAura()->RefreshDuration();
    }
    void Refresh(AuraEffect const*) { PreventDefaultAction(); }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_ghostly_blow::Tick,
            EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_ghostly_blow::Refresh,
            EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};
class ghostly_blow_damage : public UnitScript
{
public:
    ghostly_blow_damage() : UnitScript("ghostly_blow_damage", true, {UNITHOOK_ON_PERIODIC_DAMAGE_RESULT}) { }
    void OnPeriodicDamageResult(Unit*, Unit* caster, uint32 damage, SpellInfo const* info) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!damage || !info || info->Id != 983242 || !Applies(player) || !player->IsAlive() ||
            !AscensionMysticEnchant::GrantsActive(player, GhostlyBlow))
            return;
        if (player->HasAura(13713) || player->HasAura(13853) || player->HasAura(13854))
            player->ModifySpellCooldown(14278, -1000);
    }
};
#include "Area52Cryotherapist.inc"
}
void AddAscensionMysticKnightEpicScripts()
{
    new EpicWorld();
    new GlacialLightSpells();
    new StayFrostyCasts();
    new WorldforgedAuras();
    WorldforgedRegistered = true;
    new StayFrostyProcs();
    new CosmicReprisalAuras();
    RegisterSpellScript(aura_area52_stay_frosty);
    new GlacialLightAbsorbs();
    new CryotherapistContracts();
    new spell_area52_icy_penance_bolt();
    RegisterSpellScript(spell_area52_frozen_light_route);
    RegisterSpellScript(spell_area52_cryotherapist_gate);
    RegisterSpellScript(spell_area52_icy_spirit);
    RegisterSpellScript(aura_area52_icy_penance_channel);
    RegisterSpellScript(aura_area52_cryotherapy);
    RegisterSpellScript(aura_area52_spirit_frost);
    RegisterSpellScript(aura_area52_fingers_of_light_consumption);
    RegisterSpellScript(aura_area52_frozen_light);
    new CryotherapyHealingCap();
    new EpicContracts();
    new EpicPlayer();
    new NimbleMindForms();
    new ghostly_blow_damage();
    RegisterSpellScript(aura_area52_ghostly_blow);
    RegisterSpellScript(aura_area52_epic_equipment);
    RegisterSpellScript(spell_area52_exorcist_boon);
    RegisterSpellScript(spell_area52_exorcist_cooldown);
    RegisterSpellScript(aura_area52_lights_grace);
    RegisterSpellScript(aura_area52_focus_immunity);
    RegisterSpellScript(spell_area52_sanctity_trigger);
    RegisterSpellScript(spell_area52_exorcist_slash);
    RegisterSpellScript(spell_area52_emanate_damage);
    RegisterSpellScript(spell_area52_flesh_hook);
    RegisterSpellScript(spell_area52_flesh_pull);
    RegisterSpellScript(spell_area52_deconstruct);
    RegisterSpellScript(aura_area52_deconstruction_power);
}

bool HasArea52EpicEquipmentHandlers(uint32 spell)
{
    if (Area52Worldforged::Find(spell))
        return Enabled && WorldforgedRegistered;
    if (Enabled && spell == 414032)
    {
        std::list<AuraScript*> handlers;
        sScriptMgr->CreateAuraScripts(spell, handlers);
        bool found = false;
        for (auto* script : handlers)
        {
            found |= *script->_GetScriptName() == "aura_area52_stay_frosty";
            delete script;
        }
        return found;
    }
    if (!Enabled || (spell != 86576 && spell != Light && spell != Hook && spell != Deconstruction && spell != GhostlyBlow && spell != Exorcist))
        return false;
    std::list<AuraScript*> auras;
    sScriptMgr->CreateAuraScripts(spell, auras);
    bool found = false;
    for (auto* script : auras)
    {
        found |= *script->_GetScriptName() == "aura_area52_epic_equipment";
        delete script;
    }
    if (!found)
        return false;
    std::vector<std::pair<uint32, std::string>> required;
    if (spell == 86576)
    {
        required = {{92752, "spell_area52_cryotherapist_gate"}, {92762, "spell_area52_frozen_light_route"},
            {92766, "spell_area52_icy_spirit"}};
        for (uint32 id : IcyPenance)
            required.emplace_back(id, "spell_area52_cryotherapist_gate");
    }
    if (spell == Exorcist)
    {
        required.emplace_back(986282, "spell_area52_exorcist_boon");
        required.emplace_back(986284, "spell_area52_exorcist_cooldown");
        required.emplace_back(986285, "spell_area52_exorcist_cooldown");
        for (uint32 id : ExorcistSlash)
            required.emplace_back(id, "spell_area52_exorcist_slash");
    }
    if (spell == Hook)
        required = {{92583, "spell_area52_flesh_hook"}, {9654556, "spell_area52_flesh_pull"}};
    if (spell == Light)
        for (uint32 id = 926575; id <= 926589; id += 2)
            required.emplace_back(id, "spell_area52_emanate_damage");
    if (spell == Deconstruction)
        for (uint32 id : Deconstruct)
        {
            required.emplace_back(id, "spell_area52_deconstruct");
            required.emplace_back(id, "spell_area52_blademaster_empower");
        }
    for (auto const& [id, name] : required)
    {
        std::list<SpellScript*> scripts;
        sScriptMgr->CreateSpellScripts(id, scripts);
        found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == name;
            delete script;
        }
        if (!found)
            return false;
    }
    if (spell == Deconstruction || spell == GhostlyBlow)
    {
        std::list<AuraScript*> helpers;
        sScriptMgr->CreateAuraScripts(spell == GhostlyBlow ? 983242 : 954083, helpers);
        found = false;
        for (auto* script : helpers)
        {
            found |= *script->_GetScriptName() == (spell == GhostlyBlow ? "aura_area52_ghostly_blow" :
                "aura_area52_deconstruction_power");
            delete script;
        }
        if (!found)
            return false;
    }
    if (spell == 86576)
    {
        std::vector<std::pair<uint32, std::string>> auraBindings = {
            {92752, "aura_area52_frozen_light"},
            {86577, "aura_area52_cryotherapy"}, {92753, "aura_area52_spirit_frost"},
            {44543, "aura_area52_fingers_of_light_consumption"},
            {44545, "aura_area52_fingers_of_light_consumption"}};
        for (uint32 id : IcyPenance)
            auraBindings.emplace_back(id, "aura_area52_icy_penance_channel");
        for (auto const& [id, name] : auraBindings)
        {
            std::list<AuraScript*> helpers;
            sScriptMgr->CreateAuraScripts(id, helpers);
            found = false;
            for (auto* script : helpers)
            {
                found |= *script->_GetScriptName() == name;
                delete script;
            }
            if (!found)
                return false;
        }
    }
    return true;
}

void AscensionMysticEnchant::RefreshEpicReplacements(Player* player)
{
    RefreshWorldforged(player);
    SynchronizeCosmicReprisal(player);
    if (Applies(player))
    {
        Synchronize(player);
    }
}

bool AscensionMysticEnchant::HasGlacialLight(Unit const* target)
{
    return Enabled && GlacialLightPresent(target);
}

void AscensionMysticEnchant::ConsumeGlacialLightFingersForCast(Spell* spell)
{
    ConsumeGlacialLightFingers(spell);
}

void Area52Worldforged::Damage(Unit* attacker, Unit* target, SpellInfo const* spell, uint32 damage,
    uint32 healthBefore, uint32 maxHealth, bool periodic, bool killingBlow)
{
    Player* player = attacker ? attacker->ToPlayer() : nullptr;
    if (!WorldforgedRegistered || !Applies(player) || !target || target == player || !spell ||
        !damage || !player->IsAlive() || player->HasAura(780999))
        return;
    std::vector<Candidate> candidates;
    std::set<uint32> seen;
    for (auto const& [id, application] : player->GetAppliedAuras())
    {
        auto entry = Find(id);
        Aura* aura = application->GetBase();
        if (!entry || aura->GetCasterGUID() != player->GetGUID() || !seen.insert(id).second ||
            !AscensionMysticEnchant::GrantsActive(player, id) || !WorldforgedMatches(player, spell, *entry))
            continue;
        candidates.push_back({id, aura->GetStackAmount(), entry->chance, entry->family, entry->event, true});
    }
    Event const event{true, true, true, false, periodic, killingBlow, damage, healthBefore, maxHealth};
    uint32 const winner = Select(std::move(candidates), event, RandomEngine::Instance(),
        [](uint32 chance) { return roll_chance_i(chance); });
    if (auto entry = Find(winner))
        ProcWorldforged(player, *entry);
}

uint32 Area52Worldforged::Bonus(Unit* attacker, Unit* target, SpellInfo const* spell, uint32 damage)
{
    Player* player = attacker ? attacker->ToPlayer() : nullptr;
    if (!WorldforgedRegistered || !Applies(player) || !target || target == player || !spell || !damage)
        return damage;
    uint32 bonus = 0;
    std::set<uint32> seen;
    for (auto const& [id, application] : player->GetAppliedAuras())
    {
        auto entry = Find(id);
        if (!entry || (entry->family != Family::Gigantic && entry->family != Family::Necrotic) ||
            application->GetBase()->GetCasterGUID() != player->GetGUID() || !seen.insert(id).second ||
            !AscensionMysticEnchant::GrantsActive(player, id) || !WorldforgedMatches(player, spell, *entry))
            continue;
        bonus += DamageBonus(entry->family, application->GetBase()->GetStackAmount(), target->GetHealth(), target->GetMaxHealth());
    }
    return uint32(std::min<uint64>(uint64(damage) * (100 + bonus) / 100, INT32_MAX));
}

int32 Area52Worldforged::PrimaryStat(Aura* aura, int32 fallback)
{
    Player* player = aura ? aura->GetUnitOwner()->ToPlayer() : nullptr;
    if (!Enabled || !player || player->getClass() != CLASS_HERO)
        return fallback;
    uint64 stat = aura->GetScriptValue(781600);
    if (!stat)
    {
        stat = WorldforgedSelectedStat(player) + 1;
        aura->SetScriptValue(781600, stat);
    }
    return int32(stat - 1);
}
