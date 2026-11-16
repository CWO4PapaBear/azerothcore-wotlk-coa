/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "Area52ActiveBuild.h"
#include "Area52DraftPet.h"
#include "Area52FeaturedPets.h"
#include "AscensionTamingData.h"
#include "AscensionHeroPets.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"

namespace AscensionTaming
{
namespace
{
bool IsKind(uint32 creatureEntry, uint32 kind)
{
    CreatureTemplate const* creature = sObjectMgr->GetCreatureTemplate(creatureEntry);
    return creature && creature->type == kind;
}

bool IsHunterPetOfKind(PetStable::PetInfo const& pet, uint32 kind)
{
    return pet.Type == HUNTER_PET && IsKind(pet.CreatureId, kind);
}

void MoveToSlot(Player* player, uint32 petNumber, PetSaveMode slot)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_PET_SLOT_BY_ID);
    stmt->SetData(0, uint8(slot));
    stmt->SetData(1, player->GetGUID().GetCounter());
    stmt->SetData(2, petNumber);
    CharacterDatabase.Execute(stmt);
}

std::optional<std::size_t> FreeStableSlot(PetStable const& stable)
{
    for (std::size_t slot = 0; slot < stable.MaxStabledPets && slot < stable.StabledPets.size(); ++slot)
        if (!stable.StabledPets[slot])
            return slot;
    return std::nullopt;
}

void StableUnslottedHunterPets(Player* player, PetStable& stable)
{
    for (auto pet = stable.UnslottedPets.begin(); pet != stable.UnslottedPets.end();)
    {
        std::optional<std::size_t> const slot = pet->Type == HUNTER_PET ? FreeStableSlot(stable) : std::nullopt;
        if (!slot)
        {
            ++pet;
            continue;
        }
        MoveToSlot(player, pet->PetNumber, PetSaveMode(PET_SAVE_FIRST_STABLE_SLOT + *slot));
        stable.StabledPets[*slot] = std::move(*pet);
        pet = stable.UnslottedPets.erase(pet);
    }
}

bool HasUnslottedHunterPet(PetStable const& stable)
{
    for (PetStable::PetInfo const& pet : stable.UnslottedPets)
        if (pet.Type == HUNTER_PET)
            return true;
    return false;
}

bool CanLoad(Player* player, PetStable::PetInfo const& pet)
{
    if (!pet.Health)
    {
        player->SendTameFailure(PET_TAME_DEAD);
        return false;
    }
    CreatureTemplate const* creature = sObjectMgr->GetCreatureTemplate(pet.CreatureId);
    if (!creature || !creature->IsTameable(player->CanTameExoticPets()))
    {
        player->SendTameFailure(creature && creature->IsTameable(true) ? PET_TAME_CANT_CONTROL_EXOTIC
                                                                         : PET_TAME_NOPET_AVAILABLE);
        return false;
    }
    return true;
}

struct Plan
{
    std::optional<std::size_t> WantedSlot;
    std::optional<std::size_t> WantedUnslotted;
    std::optional<std::size_t> ParkSlot;

    bool HasWanted() const { return WantedSlot || WantedUnslotted; }
};

SpellCastResult MakePlan(Player* player, PetStable const& stable, Pet const* out, uint32 kind, Plan& plan)
{
    for (std::size_t slot = 0; slot < stable.StabledPets.size() && !plan.HasWanted(); ++slot)
        if (stable.StabledPets[slot] && IsHunterPetOfKind(*stable.StabledPets[slot], kind))
            plan.WantedSlot = slot;
    for (std::size_t index = 0; index < stable.UnslottedPets.size() && !plan.HasWanted(); ++index)
        if (IsHunterPetOfKind(stable.UnslottedPets[index], kind))
            plan.WantedUnslotted = index;

    if (plan.WantedSlot && !CanLoad(player, *stable.StabledPets[*plan.WantedSlot]))
        return SPELL_FAILED_DONT_REPORT;
    if (plan.WantedUnslotted && !CanLoad(player, stable.UnslottedPets[*plan.WantedUnslotted]))
        return SPELL_FAILED_DONT_REPORT;

    if (out ? out->getPetType() != HUNTER_PET : !stable.CurrentPet || stable.CurrentPet->Type != HUNTER_PET)
        return SPELL_CAST_OK;

    if (out ? !out->IsAlive() : !stable.CurrentPet->Health)
    {
        player->SendTameFailure(PET_TAME_DEAD);
        return SPELL_FAILED_DONT_REPORT;
    }
    plan.ParkSlot = plan.WantedSlot ? plan.WantedSlot : FreeStableSlot(stable);
    if (!plan.ParkSlot)
    {
        player->SendTameFailure(PET_TAME_TOO_MANY);
        return SPELL_FAILED_DONT_REPORT;
    }
    return SPELL_CAST_OK;
}

void ParkCurrent(Player* player, PetStable& stable, Pet* out, std::optional<std::size_t> slot)
{
    if (!slot)
    {
        if (out)
            player->RemovePet(out, PET_SAVE_NOT_IN_SLOT);
        else if (stable.CurrentPet)
        {
            MoveToSlot(player, stable.CurrentPet->PetNumber, PET_SAVE_NOT_IN_SLOT);
            stable.UnslottedPets.push_back(std::move(*stable.CurrentPet));
            stable.CurrentPet.reset();
        }
        return;
    }

    PetSaveMode const mode = PetSaveMode(PET_SAVE_FIRST_STABLE_SLOT + *slot);
    if (out)
        player->RemovePet(out, mode);
    else
        MoveToSlot(player, stable.CurrentPet->PetNumber, mode);
    std::swap(stable.StabledPets[*slot], stable.CurrentPet);
}

void Swap(Player* player, PetStable& stable, Pet* out, Plan const& plan)
{
    PetStable::PetInfo wanted;
    if (plan.WantedSlot)
    {
        wanted = std::move(*stable.StabledPets[*plan.WantedSlot]);
        stable.StabledPets[*plan.WantedSlot].reset();
    }
    else
    {
        wanted = std::move(stable.UnslottedPets[*plan.WantedUnslotted]);
        stable.UnslottedPets.erase(stable.UnslottedPets.begin() + *plan.WantedUnslotted);
    }

    ParkCurrent(player, stable, out, plan.ParkSlot);
    MoveToSlot(player, wanted.PetNumber, PET_SAVE_AS_CURRENT);
    stable.CurrentPet = std::move(wanted);
}

bool GrantStarter(Player* player, FamilyCall const& call)
{
    if (player->GetPetGUID())
        return false;
    Pet* pet = player->CreateTamedPetFrom(call.Starter, call.SpellId);
    if (!pet)
    {
        PetStable const* stable = player->GetPetStable();
        LOG_ERROR("coa", "Starter pet {} for {} of {} could not be created: current {} unslotted {}", call.Starter,
            call.SpellId, player->GetName(), stable && stable->CurrentPet ? stable->CurrentPet->CreatureId : 0,
            stable ? stable->UnslottedPets.size() : 0);
        return false;
    }

    uint8 const level = player->GetLevel();
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, level > 1 ? level - 1 : level);
    pet->GetMap()->AddToMap(pet->ToCreature(), true);
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, level);
    player->SetMinion(pet, true);
    pet->InitTalentForLevel();
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    player->PetSpellInitialize();
    LOG_INFO("coa", "Granted {} the starter pet {} for spell {}", player->GetName(), call.Starter, call.SpellId);
    return true;
}
}

char const* SelectWardrobeBeast(Player* player, uint32 entry, uint32 summon, uint32 dismiss)
{
    auto const* creature = sObjectMgr->GetCreatureTemplate(entry);
    if (!creature || !creature->IsTameable(true))
        return "APPLY_APPEARANCES_BAD_CREATURE_TEMPLATE";
    if (!creature->IsTameable(player->CanTameExoticPets()))
        return "APPLY_APPEARANCES_CANNOT_TAME_EXOTIC";
    if (!player->HasSpell(summon))
        return "APPLY_APPEARANCES_PET_SUMMON_SPELL_NOT_LEARNED";
    if (!player->HasSpell(dismiss))
        return "APPLY_APPEARANCES_PET_DISMISS_SPELL_NOT_LEARNED";
    if (player->IsInCombat())
        return "APPLY_APPEARANCES_COSMETIC_PET_IN_COMBAT";

    auto const draft = Area52Build::GetDraftVanityContext(player);
    if (!draft.Ready || (draft.Build && Area52Build::FeaturedHunterPet(draft.Pet, draft.Archetype)) ||
        Area52DraftPetBusy(player) || player->GetTemporaryUnsummonedPetNumber() || player->GetCharmGUID() ||
        !player->IsAlive() || !player->IsInWorld() || !player->FindMap() || player->IsBeingTeleported())
        return "APPLY_APPEARANCES_UNKNOWN";

    auto& stable = player->GetOrInitPetStable();
    auto* current = player->GetPet();
    if (current && (current->isBeingLoaded() || Area52DraftPetTracked(current) || !stable.CurrentPet ||
        stable.CurrentPet->PetNumber != current->GetCharmInfo()->GetPetNumber()))
        return "APPLY_APPEARANCES_UNKNOWN";
    if (stable.CurrentPet && (stable.CurrentPet->Type != HUNTER_PET ||
        Area52WardrobePetReserved(player, stable.CurrentPet->PetNumber)))
        return "APPLY_APPEARANCES_UNKNOWN";
    if (current ? !current->IsAlive() : stable.CurrentPet && !stable.CurrentPet->Health)
        return "APPLY_APPEARANCES_PET_DEAD_NON_SAFE_ZONE";
    if (HasUnslottedHunterPet(stable))
        return "APPLY_APPEARANCES_UNKNOWN";

    auto restore = [player](uint32 number)
    {
        auto pet = std::make_unique<Pet>(player, HUNTER_PET);
        if (!pet->LoadPetFromDB(player, 0, number, false))
            return false;
        pet.release();
        return true;
    };
    if (stable.CurrentPet && stable.CurrentPet->CreatureId == entry)
        return current || restore(stable.CurrentPet->PetNumber) ? nullptr : "APPLY_APPEARANCES_UNKNOWN";

    std::optional<std::size_t> wanted;
    for (std::size_t i = 0; i < stable.StabledPets.size(); ++i)
        if (stable.StabledPets[i] && stable.StabledPets[i]->CreatureId == entry && stable.StabledPets[i]->Type == HUNTER_PET)
        {
            if (!CanLoad(player, *stable.StabledPets[i]) || Area52WardrobePetReserved(player, stable.StabledPets[i]->PetNumber))
                return "APPLY_APPEARANCES_UNKNOWN";
            wanted = i;
            break;
        }
    auto park = wanted ? wanted : FreeStableSlot(stable);
    if (stable.CurrentPet && !park)
    {
        player->SendTameFailure(PET_TAME_TOO_MANY);
        return "APPLY_APPEARANCES_UNKNOWN";
    }

    auto previous = stable.CurrentPet;
    if (current && previous)
        current->FillPetInfo(&*previous);
    auto wantedPet = wanted ? stable.StabledPets[*wanted] : decltype(stable.CurrentPet){};
    auto tx = CharacterDatabase.BeginTransaction();
    auto slotUpdate = [&](uint32 number, PetSaveMode slot)
    {
        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_PET_SLOT_BY_ID);
        stmt->SetData(0, uint8(slot));
        stmt->SetData(1, player->GetGUID().GetCounter());
        stmt->SetData(2, number);
        tx->Append(stmt);
    };
    if (previous)
    {
        auto mode = PetSaveMode(PET_SAVE_FIRST_STABLE_SLOT + *park);
        if (current)
            player->RemovePet(current, mode, false, tx);
        else
            slotUpdate(previous->PetNumber, mode);
    }
    stable.CurrentPet.reset();
    bool loaded = false;
    if (wantedPet)
    {
        loaded = restore(wantedPet->PetNumber);
        if (loaded)
            slotUpdate(wantedPet->PetNumber, PET_SAVE_AS_CURRENT);
    }
    else
    {
        std::unique_ptr<Pet> pet(player->CreateTamedPetFrom(entry, summon));
        if (pet && pet->GetMap()->AddToMap(pet->ToCreature(), true))
        {
            player->SetMinion(pet.get(), true);
            pet->InitTalentForLevel();
            pet->SavePetToDB(PET_SAVE_AS_CURRENT, tx);
            player->PetSpellInitialize();
            pet.release();
            loaded = true;
        }
    }
    // This rare Wardrobe action needs the ASYNC statement connection and a definite
    // commit result before acknowledging the appearance or allowing another selection.
    if (loaded && CharacterDatabase.AsyncCommitTransaction(tx).m_future.get())
    {
        if (previous)
            stable.StabledPets[*park] = std::move(previous);
        return nullptr;
    }
    if (auto failed = player->GetPet())
        player->RemovePet(failed, PET_SAVE_AS_CURRENT, false, CharacterDatabase.BeginTransaction());
    stable.CurrentPet = previous;
    if (wanted)
        stable.StabledPets[*wanted] = std::move(wantedPet);
    if (previous && current)
        restore(previous->PetNumber);
    return "APPLY_APPEARANCES_UNKNOWN";
}



class spell_ascension_family_call : public SpellScript
{
    PrepareSpellScript(spell_ascension_family_call);

    SpellCastResult CheckCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        FamilyCall const* call = FindFamilyCall(GetSpellInfo()->Id);
        if (!player || !call || !AscensionHeroPets::Applies(player))
            return SPELL_CAST_OK;

        if (player->GetCharmGUID())
            return SPELL_CAST_OK;

        PetStable& stable = player->GetOrInitPetStable();
        Pet* out = player->GetPet();
        if (out ? out->getPetType() == HUNTER_PET && IsKind(out->GetEntry(), call->Kind)
                : stable.CurrentPet && IsHunterPetOfKind(*stable.CurrentPet, call->Kind))
            return SPELL_CAST_OK;

        StableUnslottedHunterPets(player, stable);

        Plan plan;
        if (SpellCastResult const result = MakePlan(player, stable, out, call->Kind, plan); result != SPELL_CAST_OK)
            return result;

        if (plan.HasWanted())
        {
            Swap(player, stable, out, plan);
            return SPELL_CAST_OK;
        }

        FamilyCall starter = *call;
        if (starter.Kind == KIND_ELEMENTAL)
        {
            auto const archetype = Area52Build::GetReadyFeaturedArchetype(player);
            if (!archetype)
                return SPELL_FAILED_TRY_AGAIN;
            constexpr uint32 Cryotherapist = 13;
            constexpr uint32 TheradrimShardling = 11783;
            if (*archetype == Cryotherapist)
                starter.Starter = TheradrimShardling;
        }
        if (!starter.Starter || !sObjectMgr->GetCreatureTemplate(starter.Starter))
        {
            player->SendTameFailure(PET_TAME_NOPET_AVAILABLE);
            return SPELL_FAILED_DONT_REPORT;
        }
        if (HasUnslottedHunterPet(stable))
        {
            player->SendTameFailure(PET_TAME_TOO_MANY);
            return SPELL_FAILED_DONT_REPORT;
        }

        ParkCurrent(player, stable, out, plan.ParkSlot);
        GrantStarter(player, starter);
        return SPELL_FAILED_DONT_REPORT;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_ascension_family_call::CheckCast);
    }
};

class AscensionTamingPlayer final : public PlayerScript
{
public:
    AscensionTamingPlayer() : PlayerScript("AscensionTamingPlayer", { PLAYERHOOK_ON_FORGOT_SPELL }) { }

    void OnPlayerForgotSpell(Player* player, uint32 spellId) override
    {
        FamilyCall const* call = FindFamilyCall(spellId);
        if (!call || !AscensionHeroPets::Applies(player))
            return;
        if (Pet* out = player->GetPet(); out && out->getPetType() == HUNTER_PET && IsKind(out->GetEntry(), call->Kind))
            player->RemovePet(out, PET_SAVE_AS_CURRENT);
    }
};
}

void AddAscensionTamingScripts()
{
    RegisterSpellScriptWithArgs(AscensionTaming::spell_ascension_family_call, "spell_ascension_family_call");
    new AscensionTaming::AscensionTamingPlayer();
}
