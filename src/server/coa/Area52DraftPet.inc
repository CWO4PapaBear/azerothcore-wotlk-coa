#include "Area52FeaturedPets.h"
namespace Area52DraftPets
{
constexpr char Key[] = "area52.draft.pet";
struct Lease : DataMap::Base
{
    bool Loaded = false;
    bool Busy = false;
    bool Permanent = false;
    bool Restored = false;
    bool Missing = false;
    bool Revoked = false;
    bool Warned = false;
    std::uint32_t Number = 0;
    std::uint32_t Previous = 0;
    std::uint64_t Build = 0;
    std::uint32_t Revision = 0;
    std::chrono::steady_clock::time_point Retry{};
};

void EraseStable(PetStable& stable, std::uint32_t number)
{
    if (stable.CurrentPet && stable.CurrentPet->PetNumber == number)
        stable.CurrentPet.reset();
    for (auto& pet : stable.StabledPets)
        if (pet && pet->PetNumber == number)
            pet.reset();
    std::erase_if(stable.UnslottedPets, [number](auto const& pet) { return pet.PetNumber == number; });
}

bool Restore(Player* owner, std::uint32_t number)
{
    if (auto* current = owner->GetPet(); current && current->GetCharmInfo()->GetPetNumber() == number)
        return true;
    if (!number || owner->GetPetGUID() || !owner->FindMap() || owner->GetCharmGUID())
        return false;
    auto pet = std::make_unique<Pet>(owner, HUNTER_PET);
    if (!pet->LoadPetFromDB(owner, 0, number, false))
        return false;
    pet.release();
    return true;
}

void Load(Player* player, Lease& lease)
{
    lease.Busy = true;
    auto* session = player->GetSession();
    auto const guid = player->GetGUID();
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_DRAFT_PET);
    query->SetData(0, guid.GetCounter());
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, guid](PreparedQueryResult rows)
        {
            auto* owner = session->GetPlayer();
            if (!owner || owner->GetGUID() != guid)
                return;
            auto& current = *owner->CustomData.GetDefault<Lease>(Key);
            current.Busy = false;
            if (!rows)
                return;
            current.Loaded = true;
            auto* fields = rows->Fetch();
            auto const oldNumber = current.Number;
            current.Number = fields[0].Get<std::uint32_t>();
            current.Previous = fields[1].Get<std::uint32_t>();
            current.Build = fields[2].Get<std::uint64_t>();
            current.Revision = fields[3].Get<std::uint32_t>();
            current.Permanent = fields[4].Get<bool>();
            current.Missing = current.Number && !fields[5].Get<bool>();
            current.Revoked = fields[6].Get<bool>();
            if (oldNumber && !current.Number && !current.Permanent)
            {
                if (auto* pet = owner->GetPet(); pet && pet->GetCharmInfo()->GetPetNumber() == oldNumber)
                    owner->RemovePet(pet, PET_SAVE_AS_DELETED);
                EraseStable(owner->GetOrInitPetStable(), oldNumber);
            }
        }));
}

void Revoke(Player* player, Lease& lease)
{
    lease.Busy = true;
    auto* session = player->GetSession();
    auto const guid = player->GetGUID();
    auto const number = lease.Number;
    auto transaction = CharacterDatabase.BeginTransaction();
    auto* erase = CharacterDatabase.GetPreparedStatement(CHAR_DEL_A52_BORROWED_PET);
    erase->SetData(0, guid.GetCounter());
    erase->SetData(1, number);
    transaction->Append(erase);
    auto* remove = CharacterDatabase.GetPreparedStatement(CHAR_UPD_A52_REVOKED_PET);
    remove->SetData(0, guid.GetCounter());
    remove->SetData(1, number);
    transaction->Append(remove);
    session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(transaction)).AfterComplete(
        [session, guid, number](bool success)
        {
            auto* owner = session->GetPlayer();
            auto* current = owner && owner->GetGUID() == guid ? owner->CustomData.Get<Lease>(Key) : nullptr;
            if (!current || current->Number != number)
                return;
            if (!success)
            {
                current->Busy = false;
                return;
            }
            current->Busy = false;
            current->Loaded = false;
            current->Restored = false;
        });
}

bool Park(Player* player, Lease& lease)
{
    auto& stable = player->GetOrInitPetStable();
    std::vector<std::size_t> free;
    for (std::size_t i = 0; i < stable.MaxStabledPets && i < stable.StabledPets.size(); ++i)
        if (!stable.StabledPets[i])
            free.push_back(i);
    std::size_t needed = stable.CurrentPet ? 1 : 0;
    for (auto const& pet : stable.UnslottedPets)
        if (pet.Type == HUNTER_PET)
            ++needed;
    if (needed > free.size())
    {
        if (!lease.Warned)
            ChatHandler(player->GetSession()).SendSysMessage("Your Draft combat pet is waiting for free stable space. Your existing pets have been preserved.");
        lease.Warned = true;
        return false;
    }
    if (stable.CurrentPet)
    {
        auto const slot = free.back();
        free.pop_back();
        lease.Previous = stable.CurrentPet->PetNumber;
        auto const mode = PetSaveMode(PET_SAVE_FIRST_STABLE_SLOT + slot);
        if (auto* pet = player->GetPet())
            player->RemovePet(pet, mode);
        else
        {
            auto* move = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_PET_SLOT_BY_ID);
            move->SetData(0, uint8(mode));
            move->SetData(1, player->GetGUID().GetCounter());
            move->SetData(2, lease.Previous);
            CharacterDatabase.Execute(move);
        }
        std::swap(stable.StabledPets[slot], stable.CurrentPet);
    }
    for (auto it = stable.UnslottedPets.begin(); it != stable.UnslottedPets.end();)
    {
        if (it->Type != HUNTER_PET)
        {
            ++it;
            continue;
        }
        auto const slot = free.back();
        free.pop_back();
        auto* move = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_PET_SLOT_BY_ID);
        move->SetData(0, uint8(PET_SAVE_FIRST_STABLE_SLOT + slot));
        move->SetData(1, player->GetGUID().GetCounter());
        move->SetData(2, it->PetNumber);
        CharacterDatabase.Execute(move);
        stable.StabledPets[slot] = std::move(*it);
        it = stable.UnslottedPets.erase(it);
    }
    return !player->GetPetGUID();
}

void Grant(Player* player, Lease& lease, Area52Build::DraftVanityContext const& context, uint32 entry)
{
    auto const* creature = sObjectMgr->GetCreatureTemplate(entry);
    if (!creature || !creature->family || player->GetTemporaryUnsummonedPetNumber() || !Park(player, lease))
        return;
    Pet* pet = player->CreateTamedPetFrom(entry);
    if (!pet)
    {
        Restore(player, lease.Previous);
        return;
    }
    lease.Number = pet->GetCharmInfo()->GetPetNumber();
    lease.Build = context.Build;
    lease.Revision = context.Revision;
    lease.Busy = true;
    lease.Restored = true;
    pet->GetMap()->AddToMap(pet->ToCreature(), true);
    player->SetMinion(pet, true);
    pet->InitTalentForLevel();
    player->PetSpellInitialize();
    auto transaction = CharacterDatabase.BeginTransaction();
    auto* insert = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_DRAFT_PET);
    insert->SetData(0, player->GetGUID().GetCounter());
    insert->SetData(1, lease.Number);
    insert->SetData(2, lease.Previous);
    insert->SetData(3, lease.Build);
    insert->SetData(4, lease.Revision);
    transaction->Append(insert);
    pet->SavePetToDB(PET_SAVE_AS_CURRENT, transaction);
    auto* session = player->GetSession();
    auto const guid = player->GetGUID();
    auto const number = lease.Number;
    session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(transaction)).AfterComplete(
        [session, guid, number](bool success)
        {
            auto* owner = session->GetPlayer();
            auto* current = owner && owner->GetGUID() == guid ? owner->CustomData.Get<Lease>(Key) : nullptr;
            if (!current || current->Number != number)
                return;
            if (!success)
            {
                if (auto* pet = owner->GetPet(); pet && pet->GetCharmInfo()->GetPetNumber() == number)
                    owner->RemovePet(pet, PET_SAVE_AS_DELETED);
                EraseStable(owner->GetOrInitPetStable(), number);
                auto const previous = current->Previous;
                *current = Lease{};
                current->Loaded = true;
                Restore(owner, previous);
            }
            current->Busy = false;
            if (success)
                current->Loaded = false;
        });
}
}

bool Area52DraftPetTracked(Pet const* pet)
{
    auto* owner = pet && pet->GetOwner() ? pet->GetOwner()->ToPlayer() : nullptr;
    auto const* lease = owner ? owner->CustomData.Get<Area52DraftPets::Lease>(Area52DraftPets::Key) : nullptr;
    return lease && !lease->Permanent && lease->Number == const_cast<Pet*>(pet)->GetCharmInfo()->GetPetNumber();
}

bool Area52WardrobePetReserved(Player const* player, std::uint32_t number)
{
    auto const* lease = player->CustomData.Get<Area52DraftPets::Lease>(Area52DraftPets::Key);
    return !lease || !lease->Loaded || lease->Busy ||
        (!lease->Permanent && lease->Number && (lease->Number == number || lease->Previous == number));
}

bool Area52DraftPetBusy(Player const* player)
{
    auto const* lease = player ? player->CustomData.Get<Area52DraftPets::Lease>(Area52DraftPets::Key) : nullptr;
    return lease && lease->Busy;
}

bool Area52DraftPetAuthorized(Player const* player, std::uint32_t number)
{
    auto const* lease = player ? player->CustomData.Get<Area52DraftPets::Lease>(Area52DraftPets::Key) : nullptr;
    if (!lease || !lease->Loaded || lease->Busy || lease->Missing || lease->Number != number)
        return false;
    if (lease->Permanent)
        return true;
    auto const context = Area52Build::GetDraftVanityContext(const_cast<Player*>(player));
    return context.Ready && context.Build == lease->Build && context.Revision == lease->Revision;
}

bool Area52DraftPetSavePending(Pet const* pet)
{
    auto* owner = pet && pet->GetOwner() ? pet->GetOwner()->ToPlayer() : nullptr;
    auto const* lease = owner ? owner->CustomData.Get<Area52DraftPets::Lease>(Area52DraftPets::Key) : nullptr;
    return Area52DraftPetTracked(pet) && lease->Busy;
}

void UpdateArea52DraftPet(Player* player)
{
    if (!AscensionFreePick::Applies(player) || player->GetSession()->IsBot())
        return;
    auto& lease = *player->CustomData.GetDefault<Area52DraftPets::Lease>(Area52DraftPets::Key);
    auto const now = std::chrono::steady_clock::now();
    if (lease.Busy || now < lease.Retry)
        return;
    lease.Retry = now + std::chrono::seconds(2);
    if (!lease.Loaded)
    {
        Area52DraftPets::Load(player, lease);
        return;
    }
    auto const context = Area52Build::GetDraftVanityContext(player);
    if (!context.Ready || !player->FindMap() || !player->IsAlive() || player->IsInCombat()
        || player->GetCharmGUID() || player->IsBeingTeleported() || player->InBattleground() || player->InArena())
        return;
    if (lease.Permanent)
    {
        if (lease.Number && !lease.Missing && !lease.Restored)
            lease.Restored = Area52DraftPets::Restore(player, lease.Number);
        return;
    }
    if (lease.Number)
    {
        if (lease.Missing || context.Build != lease.Build || context.Revision != lease.Revision)
            Area52DraftPets::Revoke(player, lease);
        else if (!lease.Restored)
        {
            lease.Restored = Area52DraftPets::Restore(player, lease.Number);
        }
        return;
    }
    if (lease.Revoked)
    {
        if (lease.Previous)
        {
            Area52DraftPets::Restore(player, lease.Previous);
            auto const& stable = player->GetOrInitPetStable();
            if (!stable.CurrentPet || stable.CurrentPet->PetNumber != lease.Previous)
                return;
        }
        auto* clear = CharacterDatabase.GetPreparedStatement(CHAR_DEL_A52_DRAFT_PET);
        clear->SetData(0, player->GetGUID().GetCounter());
        clear->SetData(1, 0);
        CharacterDatabase.Execute(clear);
        lease.Loaded = false;
        return;
    }
    if (auto const entry = Area52Build::FeaturedHunterPet(context.Pet, context.Archetype))
        Area52DraftPets::Grant(player, lease, context, entry);
}

bool Area52DraftPetRewardReady(Player* player, std::uint64_t build, std::uint32_t revision)
{
    auto const context = Area52Build::GetDraftVanityContext(player);
    if (!context.Ready || context.Build != build || context.Revision != revision)
        return false;
    if (!Area52Build::FeaturedHunterPet(context.Pet, context.Archetype))
        return true;
    auto const* lease = player->CustomData.Get<Area52DraftPets::Lease>(Area52DraftPets::Key);
    return lease && lease->Loaded && !lease->Busy && !lease->Missing && lease->Number && lease->Build == build
        && lease->Revision == revision;
}

void ConfirmArea52DraftPetReward(Player* player, std::uint64_t build, std::uint32_t revision)
{
    auto* lease = player->CustomData.Get<Area52DraftPets::Lease>(Area52DraftPets::Key);
    if (lease && lease->Build == build && lease->Revision == revision)
        lease->Permanent = true;
}
