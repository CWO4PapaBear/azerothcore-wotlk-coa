/*
 * CoA: implementations of the generic, parameterised triggers.
 * The logic is deliberately identical to the originals in GenericTriggers.cpp -
 * the only difference is that the spell name comes from the qualifier instead
 * of from the const field AiNamedObject::name.
 */
#include "CoaGenericContext.h"

#include "AscensionSpecialization.h"

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Helpers.h"
#include "LastSpellCastValue.h"
#include "ObjectMgr.h"
#include "ServerFacade.h"
#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
#include "CoaLegendaryClasses.h"
#include "CoaLegendaryPets.h"
#include "CoaSpecialization.h"
#include "GenericBuffUtils.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "FleeManager.h"
#include <cmath>
#include <map>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <limits>
#include <utility>

// How far an enemy cleave, cone or spin around the caster reaches; 0 when the spell reaches its target.
static float CoaCasterCentredReach(Player* bot, SpellInfo const* info)
{
    float reach = 0.0f;
    for (SpellEffectInfo const& effect : info->Effects)
    {
        if (!effect.IsEffect())
            continue;
        uint32 const a = effect.TargetA.GetTarget();
        uint32 const b = effect.TargetB.GetTarget();
        bool const cone = a == TARGET_UNIT_CONE_ENEMY_24 || a == TARGET_UNIT_CONE_ENEMY_54 || a == TARGET_UNIT_CONE_ENEMY_104;
        bool const around = a == TARGET_SRC_CASTER && b == TARGET_UNIT_SRC_AREA_ENEMY;
        if (!cone && !around)
            return 0.0f;
        reach = std::max(reach, effect.CalcRadius(bot));
    }
    return reach;
}

// Whether the bot holds the power the spell costs (CanCastSpell probes ignoring it).
static bool CoaHasPowerFor(Player* bot, SpellInfo const* info)
{
    if (info->PowerType >= MAX_POWERS)
        return true;
    int32 const cost = info->CalcPowerCost(bot, info->GetSchoolMask());
    return cost <= 0 || int32(bot->GetPower(Powers(info->PowerType))) >= cost;
}

// In a dungeon a damage dealer or healer does not open on an idle creature while its group has a living tank: the
// tank pulls. Houndmaster's Call and Unleash the Hounds sent the hounds through 60 idle mobs, Blood Steward included.
static bool CoaPullsAheadOfTank(Player* bot, Unit* target)
{
    if (!target || !target->IsCreature() || target->IsInCombat() || !bot->GetMap()->IsDungeon() ||
        PlayerbotAI::IsTank(bot) || !bot->IsHostileTo(target))
        return false;
    Group* group = bot->GetGroup();
    if (!group)
        return false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && member->IsAlive() && member->GetMapId() == bot->GetMapId() &&
            PlayerbotAI::IsTank(member))
            return true;
    }
    return false;
}

bool CoaCastAction::isUseful()
{
    if (!CastSpellAction::isUseful())
        return false;
    if (CoaPullsAheadOfTank(bot, GetTarget()))
        return false;
    uint32 const id = AI_VALUE2(uint32, "spell id", spell);
    SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
    if (!info)
        return true;
    if (!CoaHasPowerFor(bot, info))
        return false;
    // Celestial Cleave cast 20 yd from its target hit nothing and logged OK
    Unit* target = GetTarget();
    if (target && target != bot)
        if (float const reach = CoaCasterCentredReach(bot, info))
            return bot->IsWithinDistInMap(target, reach + target->GetCombatReach());
    return true;
}

bool CoaBuffPartyAction::isUseful()
{
    return AI_VALUE2(uint32, "spell id", spell) && BuffOnPartyAction::isUseful();
}

bool CoaCastMeleeAction::isUseful()
{
    if (!CastMeleeSpellAction::isUseful() || CoaPullsAheadOfTank(bot, GetTarget()))
        return false;
    uint32 const id = AI_VALUE2(uint32, "spell id", spell);
    SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
    return !info || CoaHasPowerFor(bot, info);
}

// The dispel type of a cure spell, taken from the spell itself.
//
// SPELL_EFFECT_DISPEL carries the type as its MiscValue - 1 magic, 2 curse,
// 3 disease, 4 poison. Dispel Magic (527) is the calibration case: effect 38,
// MiscValue 1.
//
// The name is resolved against the bot's own spellbook, so a spell the bot has
// not learned yields 0, and so does a spell that dispels nothing.
uint32 CoaDispelTypeOf(PlayerbotAI* botAI, std::string const& spellName)
{
    if (spellName.empty())
        return 0;

    uint32 const spellId =
        botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellName)->Get();
    if (!spellId)
        return 0;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return 0;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (info->Effects[i].Effect == SPELL_EFFECT_DISPEL)
            return uint32(info->Effects[i].MiscValue);

    return 0;
}

bool CoaHasAuraTrigger::IsActive()
{
    if (qualifier.empty())
        return false;
    // as in HasAuraTrigger::IsActive
    return botAI->HasAura(qualifier, GetTarget(), false, false, -1, true);
}

bool CoaTargetHasAuraTrigger::IsActive()
{
    if (qualifier.empty())
        return false;
    Unit* target = GetTarget();
    return target && botAI->HasAura(qualifier, target, false, false, -1, true);
}

bool CoaHasNoAuraTrigger::IsActive()
{
    if (qualifier.empty())
        return false;
    // as in HasNoAuraTrigger::IsActive
    return !botAI->HasAura(qualifier, GetTarget());
}

bool CoaAoeHasAuraTrigger::IsActive()
{
    return !qualifier.empty() && MediumAoeTrigger::IsActive() &&
           botAI->HasAura(qualifier, bot, false, false, -1, true);
}

std::string CoaAuraStacksTrigger::SpellPart() const
{
    size_t comma = qualifier.rfind(',');
    return comma == std::string::npos ? qualifier : qualifier.substr(0, comma);
}

int CoaAuraStacksTrigger::StackPart() const
{
    size_t comma = qualifier.rfind(',');
    if (comma == std::string::npos)
        return 1;
    return atoi(qualifier.c_str() + comma + 1);
}

bool CoaAuraStacksTrigger::IsActive()
{
    std::string const spellName = SpellPart();
    if (spellName.empty())
        return false;
    // as in HasAuraStackTrigger::IsActive
    // checkDuration MUST be false.
    //
    // With checkDuration=true PlayerbotAI::GetAura skips every aura whose
    // GetDuration() is -1, that is every PERMANENT one. Class resources are
    // exactly that: Felfury, Spirit, Insanity and the rest stack indefinitely
    // until they are spent.
    //
    // With true the trigger therefore NEVER fired. It became visible in game on
    // 15 Sep 2026: the Felsworn cast neither Azzinoth's Assault nor Sargeron
    // Smite (both `aura stacks::Felfury,2`), and Spirit Eclipse never happened
    // on the Witch Doctor.
    return botAI->GetAura(spellName, GetTarget(), false, false, StackPart()) != nullptr;
}

std::string CoaResourceTrigger::NamePart() const
{
    size_t comma = qualifier.rfind(',');
    return comma == std::string::npos ? qualifier : qualifier.substr(0, comma);
}

int CoaResourceTrigger::PercentPart() const
{
    size_t comma = qualifier.rfind(',');
    if (comma == std::string::npos)
        return 100;
    return atoi(qualifier.c_str() + comma + 1);
}

// The core resources under their own names. The compatibility module only
// carries its OWN resources (Insanity, Static, Solar Power and so on) - rage,
// energy, runic power, focus and mana come from the core and were not available
// to this trigger at all.
//
// It showed on 15 Sep 2026: Command: Undead costs 30 runic power, so the
// Necromancer bot could only cast the spender late, and we had no way to put it
// behind its builder. Same thing on the Primalist with rage.
static std::map<std::string, Powers> const CORE_RESOURCES = {
    {"mana", POWER_MANA},
    {"rage", POWER_RAGE},
    {"focus", POWER_FOCUS},
    {"energy", POWER_ENERGY},
    {"runicpower", POWER_RUNIC_POWER},
    {"runic power", POWER_RUNIC_POWER},
};

bool CoaResourceTrigger::IsActive()
{
    std::string const name = NamePart();
    if (name.empty())
        return false;

    // The Ascension-specific resources (Static, Felfury, Insanity, Solar Power
    // and the rest) are not reachable: mod-ascension-compat's public API
    // exposes specializations, talents and class abilities, but nothing to read
    // a custom resource. So only the core powers below work here.
    //
    // No rotation currently needs more - all 71 of them get by on aura stacks,
    // buff missing, debuff missing and can cast. If that changes, the shortest
    // route is asking for a percent accessor in AscensionSpecialization.h.

    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    auto itr = CORE_RESOURCES.find(lower);
    if (itr == CORE_RESOURCES.end())
        return false;  // Neither a module nor a core resource of that name

    Powers const type = itr->second;
    if (!bot->HasActivePowerType(type))
        return false;  // The class does not carry this resource

    uint32 const maximum = bot->GetMaxPower(type);
    if (!maximum)
        return false;

    return int32(bot->GetPower(type) * 100 / maximum) >= PercentPart();
}

// Wide enough for a bot to see its summons anywhere in a fight, and narrow
// enough that creatures at the other end of the zone do not count.
static constexpr float SEARCH_RANGE = 60.0f;

/* The aura of that name the buff lines look for, a passive namesake set aside. Every Draconic
 * Pyromancer wears its specialization aura (92128) under the name of its form, Draconic Aspect
 * (802117): the form counted as worn, so neither "buff missing" nor "cast buff" ever took it (#29;
 * never up on the bench either, 03/10). When the aura found by name is passive and the spell the bot
 * casts under that name is not, the one worn is that spell's own aura, if any. */
static Aura* CoaWornBuff(PlayerbotAI* botAI, std::string const& name, Unit* target, bool owner, bool duration)
{
    Aura* aura = botAI->GetAura(name, target, owner, duration);
    if (!aura || !aura->GetSpellInfo()->IsPassive())
        return aura;

    uint32 const id = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
    SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
    if (!info || info->IsPassive())
        return aura;

    Aura* worn = owner ? target->GetAura(id, botAI->GetBot()->GetGUID()) : target->GetAura(id);
    if (worn && duration && worn->GetDuration() == -1)
        return nullptr;
    return worn;
}

bool CoaCastBuffAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !AI_VALUE2(uint32, "spell id", spell) || !CastSpellAction::isUseful())
        return false;
    return ai::buff::BuffBelowRefreshTarget(botAI, CoaWornBuff(botAI, spell, target, isOwner, checkDuration),
                                            beforeDuration);
}

bool CoaBuffMissingTrigger::IsActive()
{
    Unit* target = GetTarget();
    bool active = target && ai::buff::BuffBelowRefreshTarget(
                                botAI, CoaWornBuff(botAI, spell, target, checkIsOwner, checkDuration), beforeDuration);
    bool cooling = false;
    if (active)
    {
        // no id: a spell the bot has not learned (Greater Rite at level 48 read IMPOSSIBLE every tick)
        uint32 const id = AI_VALUE2(uint32, "spell id", spell);
        SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
        active = info && !CoaHealerAvoidsForm(bot, info) && CoaRotationMayCast(botAI, bot, info);
        cooling = active && bot->HasSpellCooldown(id);
    }
    return backoff.Allow(active, cooling);
}

/* How long a debuff target must still live, with AiPlayerbot.CoaShortLivedDebuffs.
 *
 * The base DebuffTrigger and CastDebuffSpellAction want 8 s of life left on the target (its health
 * over "estimated group dps"), whatever the spell. Right for a long DoT on a creature about to die;
 * wrong for the rest. A trial or trash creature never has 8 s left, so on the dungeon trials of
 * 02-03/10 only 7 of the 64 such lines of the web DPS rotations ever fired (Conjure Storm, Melt
 * Reality, Unmake, Earthquake, Hex of Malice... never cast; nuit-0310/aoe/AOE.md).
 *
 * Here the life needed follows the spell:
 *   - half its duration: the DoT pays back its global cooldown once half its ticks land
 *     (Unmake, 1.5 s: 0.75 s; Conjure Storm, 10 s: 5 s; a DoT of 16 s or more keeps the 8 s);
 *   - 2 s at most when it also hits at once (school or weapon damage, leech): that part is never
 *     lost (Hex of Malice, Nerubian Sting, Conjure Storm, Blade of Faith, Ravage);
 *   - 8 s for a permanent aura or one without a duration, as before.
 * And for a spell that strikes an area (ground effect, area around the target or the caster), the
 * life left is that of the enemies inside the area, not of the current target alone (CoaDebuffLifeTime).
 */
namespace
{
bool CoaHitsAtOnce(SpellInfo const* info)
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (info->Effects[i].Effect)
        {
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_HEALTH_LEECH:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                return true;
            default:
                break;
        }
    }
    return false;
}

float CoaDebuffNeedLifeTime(SpellInfo const* info, float base)
{
    if (!info)
        return base;

    float need = base;
    int32 const duration = info->GetMaxDuration();
    if (duration > 0)
        need = std::min(need, float(duration) / 2000.0f);
    if (CoaHitsAtOnce(info))
        need = std::min(need, 2.0f);
    return need;
}

// The radius of the spell's enemy area, and whether it is centred on the caster. 0 when it has none.
float CoaEnemyArea(Player* bot, SpellInfo const* info, bool& aroundCaster)
{
    aroundCaster = false;
    float radius = 0.0f;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        SpellEffectInfo const& effect = info->Effects[i];
        if (!effect.IsEffect())
            continue;

        bool const persistent = effect.IsEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA);
        SpellImplicitTargetInfo const* area = nullptr;
        if (effect.TargetA.IsArea() && effect.TargetA.GetCheckType() == TARGET_CHECK_ENEMY)
            area = &effect.TargetA;
        else if (effect.TargetB.IsArea() && effect.TargetB.GetCheckType() == TARGET_CHECK_ENEMY)
            area = &effect.TargetB;
        if (!area && !persistent)
            continue;

        // Centred on the caster: an area around it or its cone, or a place set at its feet
        // (TARGET_DEST_CASTER). Otherwise on the target, where a bot puts its ground effects.
        if (area && (area->GetReferenceType() == TARGET_REFERENCE_TYPE_CASTER ||
                     area->GetReferenceType() == TARGET_REFERENCE_TYPE_SRC))
            aroundCaster = true;
        else if (effect.TargetA.GetObjectType() == TARGET_OBJECT_TYPE_DEST &&
                 effect.TargetA.GetReferenceType() == TARGET_REFERENCE_TYPE_CASTER)
            aroundCaster = true;
        radius = std::max(radius, std::max(effect.CalcRadius(bot), 5.0f));
    }
    return radius;
}

float CoaDebuffLifeTime(PlayerbotAI* botAI, Unit* target, SpellInfo const* info)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    float const dps = context->GetValue<float>("estimated group dps")->Get();
    if (dps <= 0.0f)
        return std::numeric_limits<float>::max();

    float health = float(target->GetHealth());
    bool aroundCaster = false;
    float const radius = info ? CoaEnemyArea(botAI->GetBot(), info, aroundCaster) : 0.0f;
    if (radius > 0.0f)
    {
        Unit* centre = aroundCaster ? static_cast<Unit*>(botAI->GetBot()) : target;
        for (ObjectGuid const guid : context->GetValue<GuidVector>("attackers")->Get())
        {
            if (guid == target->GetGUID())
                continue;
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->IsInWorld() && unit->GetMapId() == target->GetMapId() &&
                centre->GetExactDist(unit) <= radius)
                health += float(unit->GetHealth());
        }
    }
    return health / dps;
}

bool CoaShortLivedDebuffAllowed(PlayerbotAI* botAI, Unit* target, std::string const& spell, float base)
{
    uint32 const id = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell)->Get();
    SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
    return CoaDebuffLifeTime(botAI, target, info) >= CoaDebuffNeedLifeTime(info, base);
}

// AiPlayerbot.CoaShortLivedDebuffs: 1 for every bot, 2 for the bots whose GUID is a multiple of 4 only
// (half of the even-GUID "web" bots of the A/B bench; the other half, GUID % 4 == 2, is the control).
bool CoaShortLivedDebuffsFor(Player* bot)
{
    uint32 const mode = sPlayerbotAIConfig.coaShortLivedDebuffs;
    return mode == 1 || (mode == 2 && bot->GetGUID().GetCounter() % 4 == 0);
}
}  // namespace

// The life the base classes ask of a debuff target (DebuffTrigger, CastDebuffSpellAction defaults).
static constexpr float BASE_DEBUFF_LIFE = 8.0f;

bool CoaDebuffMissingTrigger::IsActive()
{
    bool active;
    if (CoaShortLivedDebuffsFor(bot))
    {
        // DebuffTrigger::IsActive with the life check below in place of its fixed one.
        Unit* target = GetTarget();
        active = target && target->IsAlive() && target->IsInWorld() && BuffTrigger::IsActive() &&
                 CoaShortLivedDebuffAllowed(botAI, target, spell, needLifeTime);
    }
    else
        active = DebuffTrigger::IsActive();

    bool cooling = false;
    if (active)
    {
        uint32 const id = AI_VALUE2(uint32, "spell id", spell);
        active = !CoaHealerSavesManaFrom(bot, id ? sSpellMgr->GetSpellInfo(id) : nullptr);
        cooling = active && id && bot->HasSpellCooldown(id);
    }
    return backoff.Allow(active, cooling);
}

bool CoaCastDebuffAction::isUseful()
{
    if (!CoaShortLivedDebuffsFor(bot))
        return CastDebuffSpellAction::isUseful();

    // CastDebuffSpellAction::isUseful with the life check below in place of its fixed one.
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    return CastAuraSpellAction::isUseful() && CoaShortLivedDebuffAllowed(botAI, target, spell, BASE_DEBUFF_LIFE);
}

// What this spell would heal, all its ticks counted, and the health the group is missing around
// the bot. A heal worth far more than what is missing is a heal poured into full health.
namespace
{
float CoaHealWorth(Player* bot, SpellInfo const* info, Unit* target)
{
    float total = 0.0f;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        SpellEffectInfo const& effect = info->Effects[i];
        if (effect.Effect == SPELL_EFFECT_HEAL)
            total += float(std::max(0, effect.CalcValue(bot)));
        else if (effect.Effect == SPELL_EFFECT_HEAL_PCT)
            total += float(target->GetMaxHealth()) * float(std::max(0, effect.CalcValue(bot))) / 100.0f;
        else if ((effect.Effect == SPELL_EFFECT_APPLY_AURA || effect.Effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY ||
                  effect.Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID) &&
                 effect.ApplyAuraName == SPELL_AURA_PERIODIC_HEAL)
        {
            int32 const duration = info->GetMaxDuration();
            uint32 const ticks = effect.Amplitude > 0 && duration > 0 ? uint32(duration / effect.Amplitude) : 1;
            total += float(std::max(0, effect.CalcValue(bot))) * float(std::max(1u, ticks));
        }
    }
    return total;
}

float CoaMissingAround(Player* bot)
{
    float missing = float(bot->GetMaxHealth() - bot->GetHealth());
    if (Group* group = bot->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                if (member != bot && member->IsAlive() && member->IsInWorld() && bot->GetDistance(member) < 30.0f)
                    missing += float(member->GetMaxHealth() - member->GetHealth());
    return missing;
}
}  // namespace

// A summon line that stays true does not bring the summon: a Knight of Xoroth stood 170 s in an Arathi Basin
// on "can cast::Call: Hellfire Imp", recast without end and never in combat (CoA test realm, 06/10). Ten
// seconds without the summon standing, the line is set aside 30 s, as rotation buff lines are.
bool CoaCanCastTrigger::IsActive()
{
    bool const active = CanCastNow();
    uint32 const id = AI_VALUE2(uint32, "spell id", spell);
    SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
    if (!info || !info->HasEffect(SPELL_EFFECT_SUMMON))
        return active;
    return summonBackoff.Allow(active);
}

bool CoaCanCastTrigger::CanCastNow()
{
    if (!SpellCanBeCastTrigger::IsActive())
        return false;

    uint32 const id = AI_VALUE2(uint32, "spell id", spell);
    SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
    if (!info)
        return true;

    // A resurrection in a rotation line is aimed at the current target, an enemy (Spiritual Ascension,
    // Prayer Beads: jealous-sound/azerothcore-wotlk-coa#5732, #5633). The dead are raised by "coa resurrect".
    for (SpellEffectInfo const& effect : info->Effects)
        if (effect.Effect == SPELL_EFFECT_RESURRECT || effect.Effect == SPELL_EFFECT_RESURRECT_NEW)
            return false;

    // A rotation line asks for a heal whenever it is off cooldown, whoever needs it. When it is
    // worth three times what the group is missing, it would land on full health: not now.
    if (float const worth = CoaHealWorth(bot, info, bot))
        if (worth > CoaMissingAround(bot) * 3.0f)
            return false;

    // A spell that burns or drains a power the target does not have does nothing: a Necromancer's
    // Glacial Tap (Power Burn) was refused 51 times in a row on dungeon trash without mana (23/09).
    for (SpellEffectInfo const& effect : info->Effects)
        if (effect.Effect == SPELL_EFFECT_POWER_BURN || effect.Effect == SPELL_EFFECT_POWER_DRAIN)
            if (Unit* target = AI_VALUE(Unit*, "current target"))
                if (target->GetMaxPower(Powers(effect.MiscValue)) == 0)
                    return false;

    // A boss is never charmed: Enslave Elemental, a Felsworn rotation line, charmed Noxxion in Maraudon and
    // broke the encounter (jealous-sound/azerothcore-wotlk-coa#4835). Hellbound Leash (Knight of Xoroth) too.
    if (Unit* target = AI_VALUE(Unit*, "current target"))
        if (Creature* creature = target->ToCreature())
            if (creature->IsDungeonBoss() || creature->isWorldBoss())
                for (SpellEffectInfo const& effect : info->Effects)
                    if (effect.IsAura() && (effect.ApplyAuraName == SPELL_AURA_MOD_CHARM ||
                                            effect.ApplyAuraName == SPELL_AURA_MOD_POSSESS ||
                                            effect.ApplyAuraName == SPELL_AURA_AOE_CHARM))
                        return false;

    int32 const duration = info->GetMaxDuration();
    if (bot->HasAura(id) && (duration < 0 || duration > 60 * IN_MILLISECONDS))
        return false;

    // Travel utility has no place in a rotation: Grace of the Moon, a water walk at 40% of base mana
    // that any damage cancels, took 79% of a Starcaller healer's mana in one fight. Looked for in the
    // spell a CoA ability triggers too: Tinker's Parachute Pack hands its slow fall out through
    // another spell, and bots kept putting parachutes on the group.
    auto const travelUtility = [](SpellInfo const* spell)
    {
        for (SpellEffectInfo const& effect : spell->Effects)
            if (effect.Effect == SPELL_EFFECT_APPLY_AURA || effect.Effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY ||
                effect.Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID)
                switch (effect.ApplyAuraName)
                {
                    case SPELL_AURA_WATER_WALK: case SPELL_AURA_FEATHER_FALL: case SPELL_AURA_HOVER:
                    case SPELL_AURA_WATER_BREATHING:
                        return true;
                    default:
                        break;
                }
        return false;
    };
    if (travelUtility(info))
        return false;

    bool summons = false;
    std::vector<uint32> summoned;
    for (SpellEffectInfo const& effect : info->Effects)
    {
        if (effect.Effect == SPELL_EFFECT_TRIGGER_SPELL && effect.TriggerSpell)
            if (SpellInfo const* triggered = sSpellMgr->GetSpellInfo(effect.TriggerSpell))
                if (travelUtility(triggered))
                    return false;
        if (effect.Effect == SPELL_EFFECT_SUMMON)
        {
            summons = true;
            if (effect.MiscValue > 0)
                summoned.push_back(uint32(effect.MiscValue));
        }
    }

    // A ward or effigy of which only one may stand: not again while the bot's own still stands
    // (Healing Ward was put down 15 times in one fight, 18% of base mana each). The Cultist's summons
    // come from a script, without UNIT_CREATED_BY_SPELL: the creature the spell names counts too.
    // Tentacle of Yogg-Saron was still recast all fight long in 1.7
    // (jealous-sound/azerothcore-wotlk-coa#6093, after #5739).
    auto const sameSummon = [&summoned](Unit const* unit)
    { return std::find(summoned.begin(), summoned.end(), unit->GetEntry()) != summoned.end(); };
    if (summons && getMSTimeDiff(summonCheckedAt, getMSTime()) < 3 * IN_MILLISECONDS && summonCheckedAt)
    {
        if (summonStanding)
            return false;
    }
    else if (summons)
    {
        summonCheckedAt = getMSTime();
        summonStanding = false;
        std::list<Unit*> nearby;
        Acore::AnyUnitInObjectRangeCheck check(bot, SEARCH_RANGE);
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> search(bot, nearby, check);
        Cell::VisitObjects(bot, search, SEARCH_RANGE);
        for (Unit* unit : nearby)
            if (unit && unit->IsAlive() && !unit->IsPlayer() &&
                (unit->GetUInt32Value(UNIT_CREATED_BY_SPELL) == id || sameSummon(unit)) &&
                (unit->GetOwnerGUID() == bot->GetGUID() || unit->GetCreatorGUID() == bot->GetGUID()))
            {
                summonStanding = true;
                return false;
            }
    }

    return !CoaHealerSavesManaFrom(bot, info) && !CoaHealerAvoidsForm(bot, info) &&
           CoaRotationMayCast(botAI, bot, info);
}

void CoaSummonMissingTrigger::Qualify(std::string const qual)
{
    auto const blank = [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };
    std::string name = qual;
    name.erase(std::find_if_not(name.rbegin(), name.rend(), blank).base(), name.end());
    name.erase(name.begin(), std::find_if_not(name.begin(), name.end(), blank));
    Qualified::Qualify(name);
    entry = 0;
    nameKey.clear();
    if (name.empty())
        return;

    if (std::all_of(name.begin(), name.end(), [](char c) { return std::isdigit(static_cast<unsigned char>(c)); }))
    {
        entry = uint32(std::strtoul(name.c_str(), nullptr, 10));
        if (!sObjectMgr->GetCreatureTemplate(entry))
            entry = 0;
    }
    else
        nameKey = CoaNameKey(name);
}

// With a qualifier, only the creature it names; without one, any creature of the bot.
bool CoaSummonMissingTrigger::Counts(Unit* unit) const
{
    if (qualifier.empty())
        return true;

    if (entry)
        return unit->GetEntry() == entry;

    return CoaNameIs(unit->GetName(), nameKey);
}

bool CoaSummonMissingTrigger::IsActive()
{
    // A qualifier that is neither a known creature entry nor valid UTF-8: stay quiet rather than summon forever.
    // A misspelt name is not caught here and keeps the trigger on.
    if (!qualifier.empty() && !entry && nameKey.empty())
        return false;

    for (Unit* unit : bot->m_Controlled)
        if (unit && unit->IsAlive() && unit->GetOwnerGUID() == bot->GetGUID() && Counts(unit))
            return false;

    // NOT through AI_VALUE("nearest npcs"): that value is cached. Freshly
    // summoned creatures only appear in it after the next refresh, and during
    // that window the trigger keeps the way clear - which is how the Witch
    // Doctor put down a whole field of Serpent Wards on 15 Sep 2026. Hence our
    // own, uncached search here.
    std::list<Unit*> nearby;
    Acore::AnyUnitInObjectRangeCheck check(bot, SEARCH_RANGE);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> search(bot, nearby, check);
    Cell::VisitObjects(bot, search, SEARCH_RANGE);

    for (Unit* unit : nearby)
        if (unit && unit->IsAlive() && !unit->IsPlayer() &&
            (unit->GetOwnerGUID() == bot->GetGUID() ||
             (!qualifier.empty() && unit->GetCreatorGUID() == bot->GetGUID())) &&
            Counts(unit))
            return false;

    return true;
}

bool CoaSpellReadyTrigger::IsActive()
{
    if (qualifier.empty())
        return false;
    // as in SpellNoCooldownTrigger::IsActive
    uint32 spellId = AI_VALUE2(uint32, "spell id", qualifier);
    if (!spellId)
        return false;

    return !bot->HasSpellCooldown(spellId);
}

bool CoaAllTrigger::IsActive()
{
    if (qualifier.empty())
        return false;

    for (std::string const& part : split(qualifier, '&'))
    {
        Trigger* trigger = context->GetTrigger(part);
        if (!trigger || !trigger->IsActive())
            return false;
    }
    return true;
}

// Enemies within sight that are fighting someone: how many are on the bot, how many on anyone else.
static std::pair<uint32, uint32> CoaAggroSplit(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers)
{
    uint32 onBot = 0;
    uint32 elsewhere = 0;
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || bot->GetDistance(unit) > sPlayerbotAIConfig.sightDistance)
            continue;

        Unit* victim = unit->GetVictim();
        if (!victim)
            continue;

        if (victim == bot)
            ++onBot;
        else
            ++elsewhere;
    }
    return {onBot, elsewhere};
}

bool CoaSteadyAggroTrigger::IsActive()
{
    auto const [onBot, elsewhere] = CoaAggroSplit(bot, botAI, AI_VALUE(GuidVector, "attackers"));
    return onBot && !elsewhere;
}

bool CoaAggroLostTrigger::IsActive()
{
    return CoaAggroSplit(bot, botAI, AI_VALUE(GuidVector, "attackers")).second > 0;
}

bool CoaChannelingTrigger::IsActive()
{
    Spell const* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    return channel && !qualifier.empty() && channel->GetSpellInfo()->SpellName[0] == qualifier;
}

bool CoaPartyLowManaTrigger::IsActive()
{
    std::vector<std::string> const parts = split(qualifier, ',');
    if (parts.size() != 2)
        return false;

    int const wanted = std::atoi(parts[0].c_str());
    int const percent = std::atoi(parts[1].c_str());
    if (wanted <= 0 || percent <= 0)
        return false;

    auto const low = [&](Player* member)
    {
        return member && member->IsAlive() && member->GetMaxPower(POWER_MANA) > 0 &&
            member->GetMapId() == bot->GetMapId() && bot->GetDistance(member) <= 35.0f &&
            member->GetPowerPct(POWER_MANA) < percent;
    };

    int count = 0;
    Group* group = bot->GetGroup();
    if (!group)
        return low(bot) && wanted <= 1;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        if (low(ref->GetSource()) && ++count >= wanted)
            return true;
    return false;
}

// CoaAiObjectContext.cpp: the interrupt coordination.
Spell* CoaInterruptibleSpell(Unit* unit);
bool CoaMayKick(Player* bot, Unit* caster, Spell* cast);
void CoaNoteKick(PlayerbotAI* botAI, Player* bot, Unit* caster, uint32 enemySpellId, SpellInfo const* kick);

static constexpr float CoaGroupReach = 40.0f;

// The bot and the living members of its group on its map within reach.
static std::vector<Player*> CoaGroupInReach(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {bot};

    std::vector<Player*> members;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->IsInWorld() && member->GetMapId() == bot->GetMapId() &&
            (member == bot || bot->GetDistance(member) <= CoaGroupReach))
            members.push_back(member);
    }
    return members;
}

static Player* CoaMainTank(Player* bot)
{
    Player* tank = nullptr;
    for (Player* member : CoaGroupInReach(bot))
    {
        if (PlayerbotAI::IsMainTank(member))
            return member;
        if (!tank && PlayerbotAI::IsTank(member))
            tank = member;
    }
    return tank;
}

// Keep role ownership unchanged: this is only a support target when no tank is available.
static Player* CoaSupportTarget(Player* bot)
{
    if (Player* tank = CoaMainTank(bot))
        return tank;
    Player* selected = nullptr;
    for (Player* member : CoaGroupInReach(bot))
        if (!selected || member->GetHealthPct() < selected->GetHealthPct())
            selected = member;
    return selected;
}

// A tank may initiate a pull. Without one, assist existing combat, never an idle selection.
static Unit* CoaAssistTarget(Player* bot)
{
    auto targetOf = [&](Player* member, bool allowPull) -> Unit*
    {
        if (!member)
            return nullptr;
        Unit* target = member->GetVictim();
        if (!target)
            target = member->GetSelectedUnit();
        if (!target || !target->IsInWorld() || target->GetMap() != bot->GetMap() ||
            !target->IsAlive() || !bot->IsValidAttackTarget(target) || CoaPullsAheadOfTank(bot, target))
            return nullptr;
        if (!allowPull)
        {
            bool engaged = member->GetVictim() == target;
            for (Player* ally : CoaGroupInReach(bot))
                engaged = engaged || target->GetVictim() == ally || ally->GetVictim() == target;
            if (!member->IsInCombat() || !target->IsInCombat() || !engaged)
                return nullptr;
        }
        return target;
    };
    if (Unit* target = targetOf(CoaMainTank(bot), true))
        return target;
    for (bool human : {true, false})
        for (Player* member : CoaGroupInReach(bot))
            if (member != bot && (GET_PLAYERBOT_AI(member) == nullptr) == human)
                if (Unit* target = targetOf(member, false))
                    return target;
    return targetOf(bot, false);
}

Unit* CoaGroundHealAction::GetTarget()
{
    return CoaSupportTarget(bot);
}

static uint32 CoaOwnStacks(PlayerbotAI* botAI, Unit* unit, std::string const& aura)
{
    Aura* own = botAI->GetAura(aura, unit, true);
    return own ? std::max<uint32>(own->GetStackAmount(), 1) : 0;
}

// "<name>,<number>": the name and the number after the last comma; 0 when there is none.
static std::pair<std::string, int> CoaNameAndNumber(std::string const& qualifier)
{
    size_t const comma = qualifier.rfind(',');
    if (comma == std::string::npos)
        return {qualifier, 0};
    return {qualifier.substr(0, comma), std::atoi(qualifier.c_str() + comma + 1)};
}

bool CoaNoneTrigger::IsActive()
{
    if (qualifier.empty())
        return false;

    for (std::string const& part : split(qualifier, '&'))
        if (Trigger* trigger = context->GetTrigger(part))
            if (trigger->IsActive())
                return false;
    return true;
}

bool CoaPartyHurtTrigger::IsActive()
{
    auto const [members, percent] = CoaNameAndNumber(qualifier);
    int const wanted = std::atoi(members.c_str());
    if (wanted <= 0 || percent <= 0)
        return false;

    int count = 0;
    for (Player* member : CoaGroupInReach(bot))
        if (member->GetHealthPct() < percent && ++count >= wanted)
            return true;
    return false;
}

bool CoaPartyAuraCountTrigger::IsActive()
{
    auto const [aura, wanted] = CoaNameAndNumber(qualifier);
    if (aura.empty() || wanted <= 0)
        return false;

    int count = 0;
    for (Player* member : CoaGroupInReach(bot))
        if (CoaOwnStacks(botAI, member, aura) && ++count >= wanted)
            return true;
    return false;
}

bool CoaPartyAuraSpreadTrigger::IsActive()
{
    auto const [aura, wanted] = CoaNameAndNumber(qualifier);
    if (aura.empty() || wanted <= 0)
        return false;

    int carrying = 0;
    bool lacking = false;
    for (Player* member : CoaGroupInReach(bot))
    {
        if (CoaOwnStacks(botAI, member, aura))
            ++carrying;
        else
            lacking = true;
    }
    return lacking && carrying < wanted;
}

bool CoaTankAuraBelowTrigger::IsActive()
{
    auto const [aura, stacks] = CoaNameAndNumber(qualifier);
    Player* tank = CoaSupportTarget(bot);
    return tank && !aura.empty() && stacks > 0 && CoaOwnStacks(botAI, tank, aura) < uint32(stacks);
}

bool CoaTankHealthTrigger::IsActive()
{
    auto const [low, high] = CoaNameAndNumber(qualifier);
    Player* tank = CoaSupportTarget(bot);
    if (!tank)
        return false;

    float const health = tank->GetHealthPct();
    return health >= std::atoi(low.c_str()) && health <= high;
}

bool CoaTargetStacksBelowMaxTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive() || qualifier.empty())
        return false;

    Aura* aura = botAI->GetAura(qualifier, target);
    return !aura || aura->GetStackAmount() < aura->GetSpellInfo()->StackAmount;
}

bool CoaTargetAuraExpiringTrigger::IsActive()
{
    auto const [name, seconds] = CoaNameAndNumber(qualifier);
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive() || name.empty() || seconds <= 0)
        return false;

    Aura* aura = botAI->GetAura(name, target);
    return aura && aura->GetDuration() > 0 && aura->GetDuration() < seconds * IN_MILLISECONDS;
}

bool CoaLastCastNotTrigger::IsActive()
{
    LastSpellCast& last = AI_VALUE(LastSpellCast&, "last spell cast");
    SpellInfo const* info = last.id ? sSpellMgr->GetSpellInfo(last.id) : nullptr;
    return !info || qualifier != info->SpellName[0];
}

bool CoaNoImbueTrigger::IsActive()
{
    Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    return weapon && !weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT);
}

namespace
{
constexpr float KickPackRadius = 10.0f;
constexpr int32 KickLandsMs = 200;

struct CoaKickRule
{
    std::string spell;
    bool alone = false;
    int32 delayMs = 0;
};

CoaKickRule ParseKickRule(std::string const& qualifier)
{
    std::vector<std::string> const parts = split(qualifier, ',');
    CoaKickRule rule;
    if (parts.empty())
        return rule;

    rule.spell = parts[0];
    if (parts.size() > 1)
    {
        if (parts[1] == "alone")
            rule.alone = true;
        else
            rule.delayMs = std::atoi(parts[1].c_str());
    }
    return rule;
}

// Whether another living enemy stands close enough to the caster for a knockback to scatter the pack.
bool PackAround(Player* bot, Unit* caster)
{
    std::list<Unit*> nearby;
    Acore::AnyUnitInObjectRangeCheck check(caster, KickPackRadius);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> search(caster, nearby, check);
    Cell::VisitObjects(caster, search, KickPackRadius);
    for (Unit* unit : nearby)
        if (unit != caster && unit->IsAlive() && !unit->IsPlayer() && bot->IsValidAttackTarget(unit))
            return true;
    return false;
}

int32 CastRunningMs(Spell* cast)
{
    int32 const total = cast->getState() == SPELL_STATE_CASTING ? cast->GetSpellInfo()->GetDuration()
                                                                  : cast->GetCastTime();
    return total - std::max(cast->GetCastTimeRemaining(), 0);
}

// The enemy whose cast this rule interrupts now: the current target first, then the attackers.
Unit* FindKickTarget(PlayerbotAI* botAI, Player* bot, CoaKickRule const& rule, Spell*& cast, uint32& kickId)
{
    kickId = rule.spell.empty() ? 0 : botAI->GetAiObjectContext()->GetValue<uint32>("spell id", rule.spell)->Get();
    SpellInfo const* kick = kickId ? sSpellMgr->GetSpellInfo(kickId) : nullptr;
    if (!kick || bot->HasSpellCooldown(kickId))
        return nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    std::vector<Unit*> candidates;
    if (Unit* target = context->GetValue<Unit*>("current target")->Get())
        candidates.push_back(target);
    for (ObjectGuid const guid : context->GetValue<GuidVector>("attackers")->Get())
        if (Unit* attacker = botAI->GetUnit(guid))
            candidates.push_back(attacker);

    float const range = kick->GetMaxRange(false);
    for (Unit* unit : candidates)
    {
        if (!unit->IsAlive() || !bot->IsValidAttackTarget(unit))
            continue;
        Spell* running = CoaInterruptibleSpell(unit);
        if (!running || std::max(running->GetCastTimeRemaining(), 0) < KickLandsMs)
            continue;
        bool const inReach = range <= NOMINAL_MELEE_RANGE ? bot->IsWithinMeleeRange(unit)
                                                          : bot->IsWithinDistInMap(unit, range);
        if (!inReach || (rule.delayMs && CastRunningMs(running) < rule.delayMs) ||
            (rule.alone && PackAround(bot, unit)))
            continue;
        if (!rule.delayMs && !CoaMayKick(bot, unit, running))
            continue;

        cast = running;
        return unit;
    }
    return nullptr;
}

// Group members within reach for a cast on an ally, best first by the given order.
template <typename Before>
Player* PickMember(Player* bot, Before before, std::function<bool(Player*)> const& eligible)
{
    Player* best = nullptr;
    for (Player* member : CoaGroupInReach(bot))
        if (eligible(member) && (!best || before(member, best)))
            best = member;
    return best;
}
}  // namespace

bool CoaEnemyCastingTrigger::IsActive()
{
    Spell* cast = nullptr;
    uint32 kickId = 0;
    return FindKickTarget(botAI, bot, ParseKickRule(qualifier), cast, kickId) != nullptr;
}

bool CoaKickAction::isPossible()
{
    Spell* cast = nullptr;
    uint32 kickId = 0;
    return FindKickTarget(botAI, bot, ParseKickRule(qualifier), cast, kickId) != nullptr;
}

bool CoaKickAction::Execute(Event /*event*/)
{
    Spell* cast = nullptr;
    uint32 kickId = 0;
    Unit* caster = FindKickTarget(botAI, bot, ParseKickRule(qualifier), cast, kickId);
    if (!caster)
        return false;

    uint32 const enemySpellId = cast->GetSpellInfo()->Id;
    if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, caster))
        ServerFacade::instance().SetFacingTo(bot, caster);
    if (!botAI->CastSpell(kickId, caster))
        return false;

    CoaNoteKick(botAI, bot, caster, enemySpellId, sSpellMgr->GetSpellInfo(kickId));
    return true;
}

bool CoaCastOnTankAction::isPossible()
{
    Player* tank = CoaSupportTarget(bot);
    return tank && !qualifier.empty() && botAI->CanCastSpell(qualifier, tank);
}

bool CoaCastOnTankAction::Execute(Event /*event*/)
{
    Player* tank = CoaSupportTarget(bot);
    return tank && botAI->CastSpell(qualifier, tank);
}

static Player* CoaSpreadTarget(PlayerbotAI* botAI, Player* bot, std::string const& qualifier)
{
    std::vector<std::string> const parts = split(qualifier, ',');
    if (parts.empty())
        return nullptr;

    std::string const& spell = parts[0];
    bool const othersOnly = parts.size() > 1 && parts[1] == "others";
    auto const rank = [](Player* member)
    { return PlayerbotAI::IsTank(member) ? 0 : PlayerbotAI::IsHeal(member) ? 1 : 2; };
    return PickMember(
        bot,
        [&](Player* a, Player* b)
        { return rank(a) != rank(b) ? rank(a) < rank(b) : a->GetHealthPct() < b->GetHealthPct(); },
        [&](Player* member)
        {
            return (!othersOnly || member != bot) && !CoaOwnStacks(botAI, member, spell) &&
                botAI->CanCastSpell(spell, member);
        });
}

bool CoaSpreadAction::isPossible() { return !qualifier.empty() && CoaSpreadTarget(botAI, bot, qualifier); }

bool CoaSpreadAction::Execute(Event /*event*/)
{
    Player* target = CoaSpreadTarget(botAI, bot, qualifier);
    return target && botAI->CastSpell(split(qualifier, ',')[0], target);
}

static Player* CoaAuraTarget(PlayerbotAI* botAI, Player* bot, std::string const& qualifier)
{
    std::vector<std::string> const parts = split(qualifier, ',');
    if (parts.size() != 2)
        return nullptr;

    return PickMember(
        bot, [](Player* a, Player* b) { return a->GetHealthPct() < b->GetHealthPct(); },
        [&](Player* member)
        { return CoaOwnStacks(botAI, member, parts[1]) && botAI->CanCastSpell(parts[0], member); });
}

bool CoaCastOnAuraAction::isPossible() { return CoaAuraTarget(botAI, bot, qualifier) != nullptr; }

bool CoaCastOnAuraAction::Execute(Event /*event*/)
{
    Player* target = CoaAuraTarget(botAI, bot, qualifier);
    return target && botAI->CastSpell(split(qualifier, ',')[0], target);
}

bool CoaLegendaryPetAction::isUseful() { return (!bot->GetPet() || !bot->GetPet()->IsAlive()) && CoaLegendaryPetEntry(bot); }

bool CoaLegendaryPetAction::Execute(Event /*event*/)
{
    if (Pet* pet = bot->GetPet(); pet && !pet->IsAlive())
        return botAI->CastSpell("Revive Pet", bot);
    if (!bot->GetPetStable() || !bot->GetPetStable()->CurrentPet)
        return CoaCreateLegendaryPet(bot) != nullptr;
    return botAI->CastSpell("Call Pet", bot) || botAI->CastSpell("Revive Pet", bot);
}

bool CoaLegendaryUnreadyTrigger::IsActive() { return CoaLegendaryClassUnready(bot); }

bool CoaLegendarySetupAction::isUseful() { return !bot->IsInCombat() && bot->IsAlive(); }

bool CoaLegendarySetupAction::Execute(Event /*event*/)
{
    bool const hadClass = GetCoaLegendaryClass(bot) != nullptr;
    EnsureCoaLegendaryClass(bot);
    if (!hadClass && GetCoaLegendaryClass(bot))
        botAI->ResetStrategies();
    return true;
}

bool CoaCryotherapistAction::Execute(Event)
{
    Player* tank = qualifier == "catch up" ? CoaMainTank(bot) : CoaSupportTarget(bot);
    auto cast=[&](char const* name,Unit* unit)
    {return unit && botAI->CanCastSpell(name,unit) && botAI->CastSpell(name,unit);};
    auto ward=[&](char const* name,Unit* unit)
    {return unit && !botAI->HasAura(name,unit) && cast(name,unit);};
    auto ground=[&](char const* name,Unit* unit)
    {
        if(!unit) return false;
        uint32 id=AI_VALUE2(uint32,"spell id",name);
        float x=unit->GetPositionX(),y=unit->GetPositionY(),z=unit->GetPositionZ();
        return id && botAI->CanCastSpell(id,x,y,z) && botAI->CastSpell(id,x,y,z);
    };
    if(qualifier=="emergency") return tank && tank->GetHealthPct()<25 && cast("Lay on Hands",tank);
    if(qualifier=="mana")
    {
        if(!bot->GetMaxPower(POWER_MANA) || uint64(bot->GetPower(POWER_MANA))*100 >= uint64(bot->GetMaxPower(POWER_MANA))*30)
            return false;
        return ward("Innervate",bot) || cast("Evocation",bot);
    }
    if(qualifier=="pet autocast")
    {
        Pet* pet=bot->GetPet();
        if(!pet || !pet->IsAlive() || pet->GetEntry()!=11783) return false;
        bool changed=false;
        for(auto const& [id,spell]:pet->m_spells)
            if(spell.state!=PETSPELL_REMOVED && spell.active!=ACT_ENABLED)
                if(auto info=sSpellMgr->GetSpellInfo(id); info && info->IsAutocastable())
                {pet->ToggleAutocast(info,true);changed=true;}
        return changed;
    }
    if(qualifier=="catch up")
    {
        if(!tank && bot->GetGroup())
            for(GroupReference* ref=bot->GetGroup()->GetFirstMember();ref;ref=ref->next())
                if(auto m=ref->GetSource();m && m!=bot && m->IsAlive() && m->IsInWorld() && m->GetMap()==bot->GetMap() && PlayerbotAI::IsTank(m))
                {tank=m;break;}
        if(!tank || bot->IsWithinDistInMap(tank,32.f) || !bot->IsWithinLOSInMap(tank) ||
            (nextCatchUp && int32(getMSTime()-nextCatchUp)<0)) return false;
        bool ok=ground("Angelic Feather",bot);
        if(!ok){bot->SetFacingToObject(tank);ok=cast("Shimmer",bot);}
        if(ok) nextCatchUp=getMSTime()+3000;
        return ok;
    }
    if(qualifier=="frozen light") return ward("Frozen Light",tank);
    if(qualifier=="tank wards") return ward("Power Word: Shield",tank) || ward("Prayer of Mending",tank) || ward("Frozen Light",tank);
    if(qualifier=="cleanse")
    {
        auto cleanse=[&](Player* who)
        {
            if(!who) return false;
            if((botAI->HasAuraToDispel(who,DISPEL_DISEASE) || botAI->HasAuraToDispel(who,DISPEL_POISON) ||
                botAI->HasAuraToDispel(who,DISPEL_MAGIC)) && cast("Purify",who)) return true;
            return botAI->HasAuraToDispel(who,DISPEL_CURSE) && cast("Dispel Curse",who);
        };
        if(cleanse(tank) || cleanse(bot)) return true;
        for(auto member:CoaGroupInReach(bot))
            if(member!=tank && member!=bot && (!bot->IsInCombat() || (!PlayerbotAI::IsTank(member) && !PlayerbotAI::IsHeal(member))) && cleanse(member)) return true;
        return false;
    }
    if(qualifier=="escape")
    {
        Unit* enemy=nullptr;
        for(auto guid:AI_VALUE(GuidVector,"attackers"))
            if(auto u=botAI->GetUnit(guid);u && u->IsAlive() && u->GetVictim()==bot){enemy=u;break;}
        if(!enemy && !bot->getAttackers().empty()) enemy=*bot->getAttackers().begin();
        if(!enemy){escapeStep=0;return false;}
        if(auto gcd=sSpellMgr->GetSpellInfo(1752);gcd && bot->GetGlobalCooldownMgr().HasGlobalCooldown(gcd))return false;
        char const* names[]={"Fade","Cone of Cold","Frost Nova","Shimmer"};
        while(escapeStep<4)
        {
            auto i=escapeStep++;
            if(i==1) bot->SetFacingToObject(enemy);
            if(i==3) bot->SetFacingTo(enemy->GetAngle(bot));
            if(cast(names[i],i==1?enemy:bot)) return true;
        }
        return false;
    }
    auto members=CoaGroupInReach(bot);
    if(qualifier=="group heal")
        return std::count_if(members.begin(),members.end(),[&](Player* m){return !PlayerbotAI::IsTank(m) && m->GetHealthPct()<75;})>=2 && cast("Prayer of Healing",bot);
    if(qualifier=="tremor")
    {
        for(auto member:members)
            if(member->HasAuraWithMechanic((1ULL<<MECHANIC_FEAR)|(1ULL<<MECHANIC_CHARM)|(1ULL<<MECHANIC_SLEEP)))
                return cast("Tremor Totem",bot);
        return false;
    }
    Aura* frost=botAI->GetAura("Spirit Frost",bot);
    unsigned stacks=frost?frost->GetStackAmount():0;
    if(qualifier=="greater heal") return tank && stacks>=2 && stacks<=5 && tank->GetHealthPct()<75 && cast("Greater Heal",tank);
    if(qualifier=="flash heal") return tank && stacks<2 && tank->GetHealthPct()<50 && cast("Flash Heal",tank);
    Unit* target=tank?tank->GetVictim():nullptr;
    if(!target && tank) target=tank->GetSelectedUnit();
    if(!target || !target->IsAlive() || !bot->IsValidAttackTarget(target) || CoaPullsAheadOfTank(bot,target)) return false;
    if(qualifier=="hard fight")
    {
        auto c=target->ToCreature();
        if(!(c && (c->IsDungeonBoss() || c->isWorldBoss())) && !botAI->GetAiObjectContext()->GetTrigger("generic boost")->IsActive())return false;
        return ward("Beacon of Light",tank) || ground("Ring of Frost",tank);
    }
    if(qualifier=="glacial") return !botAI->HasAura("Glacial Light",target,true) && cast("Glacial Light",target);
    if(qualifier=="penance") return cast("Icy Penance",target);
    if(qualifier=="frost cycle" && stacks<2)
    {
        for(unsigned n=0;n<2;++n){unsigned i=(nextShot+n)%2;if(cast(i?"Ice Lance":"Frostbolt",target)){nextShot=1-i;return true;}}
    }
    if(qualifier=="area")
    {
        for(unsigned n=0;n<3;++n){unsigned i=(nextArea+n)%3;
            if(i==0?ground("Blizzard",target):cast(i==1?"Howling Blast":"Cone of Cold",target)){nextArea=(i+1)%3;return true;}}
    }
    return false;
}

bool CoaBulwarkAction::Execute(Event)
{
    constexpr uint32 bulwark = 922565;
    Spell const* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    bool const holding = (channel && channel->m_spellInfo->Id == bulwark) || bot->HasAura(bulwark);
    if (qualifier == "cancel")
    {
        if (bot->IsInCombat() || !holding) return false;
        WorldPacket cancel(CMSG_CANCEL_AURA, 4);
        cancel << bulwark;
        bot->GetSession()->HandleCancelAuraOpcode(cancel);
        return !bot->HasAura(bulwark) && (!bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) ||
            bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL)->m_spellInfo->Id != bulwark);
    }
    auto cast = [&](char const* name, Unit* who)
    { return who && botAI->CanCastSpell(name,who) && botAI->CastSpell(name,who); };
    auto buff = [&](char const* name)
    { return !botAI->HasAura(name,bot) && cast(name,bot); };
    if (qualifier == "armor")
        return buff(AI_VALUE2(uint32,"spell id","Ice Armor") ? "Ice Armor" : "Frost Armor");
    if (!bot->IsInCombat()) return false;
    Unit* target = AI_VALUE(Unit*,"current target");
    if (qualifier == "mongoose")
        return bot->GetPower(POWER_ENERGY)<70 && cast("Mongoose Bite",target);
    if (qualifier == "block")
        return !holding && buff("Shield Block");
    if (qualifier == "full channel" || qualifier == "pack channel")
        return !holding && (qualifier == "pack channel" || bot->GetPower(POWER_ENERGY)==100) &&
            botAI->CanCastSpell(bulwark,bot) && botAI->CastSpell(bulwark,bot);

    static constexpr char const* defensives[] = {"Avatar","Shield Wall","Last Stand","Icebound Fortitude","Combat Readiness"};
    if (qualifier == "defensive")
    {
        if (bot->GetHealthPct()>=25) return false;
        for (auto name : defensives)
            if (botAI->HasAura(name,bot)) return false;
        for (auto name : defensives)
            if (cast(name,bot)) return true;
        return false;
    }
    if (qualifier == "hard fight")
    {
        Creature* enemy=target ? target->ToCreature() : nullptr;
        if (!(enemy && (enemy->isWorldBoss() || enemy->IsDungeonBoss())) &&
            !botAI->GetAiObjectContext()->GetTrigger("generic boost")->IsActive()) return false;
        if (buff("Demoralizing Banner")) return true;
        if (bot->GetHealthPct()<25) return false;
        for (auto name : defensives)
            if (botAI->HasAura(name,bot)) return false;
        return buff("Combat Readiness") || buff("Avatar");
    }
    auto quake = [&]()
    {
        uint32 id=AI_VALUE2(uint32,"spell id","Earthquake");
        return id && botAI->CanCastSpell(id,bot->GetPositionX(),bot->GetPositionY(),bot->GetPositionZ()) &&
            botAI->CastSpell(id,bot->GetPositionX(),bot->GetPositionY(),bot->GetPositionZ());
    };
    if (qualifier=="quake") return quake();
    if (qualifier=="reflect" || qualifier=="caster quake")
    {
        if (botAI->HasAura("Spell Reflection",bot)) return false;
        auto casting = [&](Unit* unit)
        {
            if (!unit || !unit->IsAlive() || !bot->IsValidAttackTarget(unit)) return false;
            Spell* spell=unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (!spell) spell=unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
            return spell && spell->GetCastTimeRemaining()>0 &&
                (qualifier=="caster quake" || spell->GetCastTimeRemaining()<=3000);
        };
        if (qualifier=="reflect") return casting(target) && buff("Spell Reflection");
        if (casting(target)) return quake();
        for (auto guid : AI_VALUE(GuidVector,"attackers"))
            if (casting(botAI->GetUnit(guid))) return quake();
        return false;
    }
    if (qualifier=="recover")
    {
        for (auto guid : AI_VALUE(GuidVector,"attackers"))
        {
            Unit* enemy=botAI->GetUnit(guid);
            Player* ally=enemy && enemy->GetVictim() ? enemy->GetVictim()->ToPlayer() : nullptr;
            if (!enemy || !enemy->IsAlive() || !bot->IsValidAttackTarget(enemy) || !ally || ally==bot ||
                !bot->GetGroup() || ally->GetGroup()!=bot->GetGroup()) continue;
            if (guid!=rescuedEnemy || ally->GetGUID()!=rescuedAlly)
            { recoveryStep=0; rescuedEnemy=guid; rescuedAlly=ally->GetGUID(); }
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(enemy);
            bot->SetSelection(guid);
            bot->Attack(enemy,true);
            SpellInfo const* gcd=sSpellMgr->GetSpellInfo(1752);
            if (gcd && bot->GetGlobalCooldownMgr().HasGlobalCooldown(gcd)) return false;
            for (; recoveryStep<3; )
            {
                unsigned step=recoveryStep++;
                if (step==0 && cast("Frost Shock",enemy)) return true;
                if (step==1 && cast("Taunt",enemy)) return true;
                if (step==2 && cast("Intervene",ally)) return true;
            }
            recoveryStep=0;
            return false;
        }
        rescuedEnemy.Clear();
        rescuedAlly.Clear();
        recoveryStep=0;
    }
    return false;
}

bool CoaMarksmanAction::Execute(Event)
{
    time_t const start = AI_VALUE(time_t, "combat start time");
    if (start != encounter)
    {
        encounter = start;
        nextShot = escapeStep = burstStep = 0;
        usedOpener = false;
    }
    Player* tank = CoaMainTank(bot);
    Unit* target = CoaAssistTarget(bot);
    auto cast = [&](std::string const& name, Unit* unit)
    {
        return unit && botAI->CanCastSpell(name, unit) && botAI->CastSpell(name, unit);
    };
    auto aura = [&](char const* name, Unit* unit){ return unit && botAI->HasAura(name, unit); };

    if (qualifier == "aspect")
    {
        char const* name = bot->IsInCombat() ? "Aspect of the Viper" :
            bot->GetLevel() >= 74 ? "Aspect of the Dragonhawk" : "Aspect of the Hawk";
        if (bot->IsInCombat() && (!bot->GetMaxPower(POWER_MANA) ||
            uint64(bot->GetPower(POWER_MANA))*100 >= uint64(bot->GetMaxPower(POWER_MANA))*30))
            return false;
        return !aura(name, bot) && cast(name, bot);
    }
    if (qualifier == "poison")
    {
        Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (!weapon || weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
            return false;
        return botAI->CanCastSpell("Instant Poison", bot, weapon) && botAI->CastSpell("Instant Poison", bot, weapon);
    }
    if (qualifier == "escape")
    {
        Unit* threat = nullptr;
        for (auto guid : AI_VALUE(GuidVector, "attackers"))
            if (Unit* enemy = botAI->GetUnit(guid); enemy && enemy->IsAlive() && enemy->GetVictim() == bot)
            { threat = enemy; break; }
        if (!threat && !bot->getAttackers().empty())
            threat = *bot->getAttackers().begin();
        if (!threat) { escapeStep = 0; return false; }
        // Advance only across unavailable moves or after a successful cast. Pause through the GCD.
        if (SpellInfo const* gcd = sSpellMgr->GetSpellInfo(1752); gcd && bot->GetGlobalCooldownMgr().HasGlobalCooldown(gcd))
            return false;
        static constexpr char const* steps[] = {"Evasion","Feign Death","Freezing Trap","Disengage",
            "Concussive Shot","Cloak of Shadows","Vanish","Preparation","Cloak of Shadows","Vanish","Sprint"};
        while (escapeStep < std::size(steps))
        {
            unsigned step = escapeStep++;
            if (step == 7)
            {
                uint32 vanish = AI_VALUE2(uint32, "spell id", "Vanish");
                if (!vanish || !bot->HasSpellCooldown(vanish) || !cast("Preparation",bot))
                { escapeStep = 10; continue; }
                return true;
            }
            if (cast(steps[step], step == 4 ? threat : bot))
                return true;
        }
        return botAI->DoSpecificAction("flee");
    }
    if (qualifier == "burst")
    {
        Creature* boss = target ? target->ToCreature() : nullptr;
        if (!target || (!(boss && (boss->isWorldBoss() || boss->IsDungeonBoss())) &&
            !botAI->GetAiObjectContext()->GetTrigger("generic boost")->IsActive()))
            return false;
        if (aura("Rapid Fire",bot) || aura("Bloodlust",bot) || aura("Recklessness",bot))
            return false;
        static constexpr char const* names[] = {"Rapid Fire","Bloodlust","Recklessness"};
        for (unsigned i=0;i<std::size(names);++i)
        {
            unsigned index=(burstStep+i)%std::size(names);
            if (cast(names[index],bot)) { burstStep=(index+1)%std::size(names); return true; }
        }
        return false;
    }
    if (!target)
        return false;
    if (qualifier == "sniper") return bot->HasAuraType(SPELL_AURA_MOD_STEALTH) && cast("Syndicate Sniper Shot",target);
    if (qualifier == "mark") return !aura("Hunter's Mark",target) && cast("Hunter's Mark",target);
    if (qualifier == "execute") return target->GetHealthPct()<20 && cast("Kill Shot",target);
    if (qualifier == "sting") return !aura("Scorpid Sting",target) && cast("Scorpid Sting",target);
    if (qualifier == "redirect") return !aura("Misdirection",bot) && cast("Misdirection",tank);
    bool const hasComboPoints = bot->GetComboPoints() > 0 && bot->GetComboTarget() == target;
    if (qualifier == "opener")
    {
        if (!start || usedOpener || time(nullptr)-start>5 || !hasComboPoints)
            return false;
        usedOpener=cast("Slice and Dice",bot);
        return usedOpener;
    }
    if (qualifier == "shots")
    {
        if (hasComboPoints && cast("Compound Shot",target)) return true;
        static constexpr char const* shots[] = {"Steady Shot","Arcane Shot","Combo Shot"};
        for (unsigned i=0;i<std::size(shots);++i)
        {
            unsigned index=(nextShot+i)%std::size(shots);
            if (cast(shots[index],target)) { nextShot=(index+1)%std::size(shots); return true; }
        }
    }
    if (qualifier == "multi") return cast("Multi-Shot",target);
    if (qualifier == "volley")
    {
        uint32 id=AI_VALUE2(uint32,"spell id","Volley");
        return id && botAI->CanCastSpell(id,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ()) &&
            botAI->CastSpell(id,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ());
    }
    return false;
}

bool CoaWalkerTrigger::IsActive()
{
    if (qualifier == "pet")
        return CoaLegendaryPetEntry(bot) && (!bot->GetPet() || !bot->GetPet()->IsAlive());
    if (qualifier == "self curse")
        return botAI->HasAuraToDispel(bot, DISPEL_CURSE);
    if (qualifier == "below five")
        return !botAI->GetAura("Maelstrom Weapon", bot, false, false, 5);
    if (qualifier == "water")
    {
        uint32 learned = AI_VALUE2(uint32, "spell id", "Conjure Water");
        bool createsWater = false;
        for (uint32 rank = sSpellMgr->GetFirstSpellInChain(learned); rank; rank = sSpellMgr->GetNextSpellInChain(rank))
            if (SpellInfo const* info = sSpellMgr->GetSpellInfo(rank))
                for (auto const& effect : info->Effects)
                    if (effect.Effect == SPELL_EFFECT_CREATE_ITEM && effect.ItemType)
                    {
                        createsWater = true;
                        if (bot->HasItemCount(effect.ItemType, 1, false))
                            return false;
                    }
        return createsWater;
    }
    if (qualifier == "totem")
    {
        uint32 learned = AI_VALUE2(uint32, "spell id", "Windfury Totem");
        if (!learned)
            return false;
        for (auto guid : bot->m_SummonSlot)
            if (Unit* totem = botAI->GetUnit(guid); totem && totem->IsAlive() &&
                sSpellMgr->GetFirstSpellInChain(totem->GetUInt32Value(UNIT_CREATED_BY_SPELL)) ==
                sSpellMgr->GetFirstSpellInChain(learned) && bot->IsWithinDistInMap(totem, 30.f))
                return false;
        return true;
    }
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return false;
    if (qualifier == "target30")
        return bot->IsWithinDistInMap(target, 30.f);
    bool cluster = qualifier == "orb cluster";
    if (cluster && !bot->IsWithinDistInMap(target, 25.f))
        return false;
    if (!cluster && qualifier != "three nearby")
        return false;
    unsigned count = 0;
    for (auto guid : AI_VALUE(GuidVector, "possible targets"))
        if (Unit* unit = botAI->GetUnit(guid); unit && unit->IsAlive() && unit->IsInCombat() &&
            bot->IsWithinDistInMap(unit, cluster ? 25.f : 8.f))
        {
            if (cluster && unit != target && target->IsWithinDistInMap(unit, 8.f))
                return true;
            if (!cluster && ++count == 3)
                return true;
        }
    return false;
}

bool CoaDefiasAction::Execute(Event)
{
    if (!bot->IsAlive())
        return false;
    auto cast = [&](char const* name, Unit* target)
    { return target && botAI->CanCastSpell(name, target) && botAI->CastSpell(name, target); };
    auto missing = [&](char const* name, Unit* target)
    { return target && !botAI->HasAura(name, target) && cast(name, target); };
    auto known = [&](char const* name) { return AI_VALUE2(uint32, "spell id", name) != 0; };
    if (auto gcd = sSpellMgr->GetSpellInfo(1752); gcd && bot->GetGlobalCooldownMgr().HasGlobalCooldown(gcd))
        return false;
    if (qualifier == "maintenance")
    {
        if (bot->HasAuraType(SPELL_AURA_MOD_STEALTH))
            return false;
        return missing("Battle Stance", bot) || missing("Aspect of the Hawk", bot) ||
            missing("Crusader Aura", bot);
    }
    if (qualifier == "escape")
    {
        Unit* threat = nullptr;
        for (auto guid : AI_VALUE(GuidVector, "attackers"))
            if (Unit* enemy = botAI->GetUnit(guid); enemy && enemy->IsAlive() && enemy->GetVictim() == bot)
            { threat = enemy; break; }
        if (!threat)
            for (Unit* enemy : bot->getAttackers())
                if (enemy && enemy->IsAlive() && enemy->GetVictim() == bot)
                { threat = enemy; break; }
        if (!threat)
        { escapeStep = 0; return false; }
        char const* steps[] = {"Fade", "Blind", "Dispersion", "Psychic Horror", "Blink", "Sprint"};
        while (escapeStep < std::size(steps))
        {
            unsigned step = escapeStep++;
            if (step == 4)
            {
                float x, y, z;
                FleeManager flee(bot, 20.0f, 0.0f, true);
                if (!flee.CalculateDestination(&x, &y, &z) ||
                    !bot->IsWithinLOS(x, y, z) ||
                    threat->GetDistance(x, y, z) <= threat->GetDistance(bot))
                    continue;
                bot->SetFacingTo(std::atan2(y - bot->GetPositionY(), x - bot->GetPositionX()));
            }
            if (cast(steps[step], step == 1 || step == 3 ? threat : bot))
            {
                if (step == 5)
                    botAI->DoSpecificAction("flee");
                return true;
            }
        }
        return botAI->DoSpecificAction("flee");
    }
    if (qualifier == "noncombat")
    {
        if (bot->IsInCombat())
        { eating = false; return false; }
        auto members = CoaGroupInReach(bot);
        for (Player* member : members)
            if ((botAI->HasAuraToDispel(member, DISPEL_POISON) ||
                botAI->HasAuraToDispel(member, DISPEL_DISEASE) ||
                botAI->HasAuraToDispel(member, DISPEL_MAGIC)) && cast("Cleanse", member))
                return true;
        if (bot->GetHealthPct() < 75)
            eating = true;
        if (bot->GetHealthPct() >= 100)
            eating = false;
        if (eating)
            return botAI->DoSpecificAction("eat");
        for (uint8 slot : {uint8(EQUIPMENT_SLOT_RANGED), uint8(EQUIPMENT_SLOT_MAINHAND)})
            if (Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                weapon && !weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) &&
                botAI->CanCastSpell("Instant Poison", bot, weapon) && botAI->CastSpell("Instant Poison", bot, weapon))
                return true;
        static constexpr char const* buffs[][2] = {
            {"Power Word: Fortitude", "Prayer of Fortitude"},
            {"Arcane Intellect", "Arcane Brilliance"},
            {"Blessing of Kings", "Greater Blessing of Kings"},
            {"Blessing of Might", "Greater Blessing of Might"},
            {"Mark of the Wild", "Gift of the Wild"}};
        for (auto const& family : buffs)
            for (Player* member : members)
                if (!botAI->HasAura(family[0], member) && !botAI->HasAura(family[1], member) &&
                    cast(known(family[1]) ? family[1] : family[0], member))
                    return true;
        if (!bot->HasAuraType(SPELL_AURA_MOD_STEALTH) &&
            (missing("Battle Stance", bot) || missing("Aspect of the Hawk", bot) || missing("Crusader Aura", bot)))
            return true;
        uint32 refreshment = AI_VALUE2(uint32, "spell id", "Conjure Refreshment");
        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(refreshment))
        {
            bool creates = false, present = false;
            for (auto const& effect : info->Effects)
                if (effect.Effect == SPELL_EFFECT_CREATE_ITEM && effect.ItemType)
                { creates = true; present = present || bot->HasItemCount(effect.ItemType, 1, false); }
            if (creates && !present && cast("Conjure Refreshment", bot))
                return true;
        }
        return missing("Stealth", bot);
    }
    Unit* target = CoaAssistTarget(bot);
    if (!target)
        return false;
    if (qualifier == "execute")
        return target->GetHealthPct() < 20 && cast("Kill Shot", target);
    if (qualifier == "area")
    {
        if (cast("Grape Shot", target))
            return true;
        uint32 volley = AI_VALUE2(uint32, "spell id", "Volley");
        return volley && botAI->CanCastSpell(volley, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ()) &&
            botAI->CastSpell(volley, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
    }
    if (qualifier != "combat")
        return false;
    time_t const start = AI_VALUE(time_t, "combat start time");
    if (encounter != start || openingTarget != target->GetGUID())
    {
        encounter = start;
        openingTarget = target->GetGUID();
        opener = nextShot = 0;
        openedStealthed = bot->HasAuraType(SPELL_AURA_MOD_STEALTH);
    }
    char const* opening[] = {"Shadowstep", "Shadow Dance", "Premeditation", "Cheap Shot"};
    while (opener < std::size(opening))
    {
        unsigned step = opener++;
        if (step == 1 && openedStealthed)
            continue;
        if (cast(opening[step], step == 1 ? bot : target))
            return true;
    }
    char const* shots[] = {"Pistol Shot", "Grape Shot", "Mongoose Bite", "Bloodrage"};
    for (unsigned i = 0; i < std::size(shots); ++i)
    {
        unsigned step = (nextShot + i) % std::size(shots);
        if (step == 3 && botAI->HasAura("Bloodrage", bot))
            continue;
        if (cast(shots[step], step == 3 ? bot : target))
        { nextShot = (step + 1) % std::size(shots); return true; }
    }
    return false;
}
