// Copyright Adam Grodzki All Rights Reserved.

#include "Components/AGR_EquipmentManager.h"

#include "BlueprintGameplayTagLibrary.h"
#include "Components/AGR_ItemComponent.h"
#include "Data/AGRLibrary.h"
#include "Data/AGRTypes.h"
#include "Net/UnrealNetwork.h"

bool operator==(const FShortcutItemReference& lhs, const FShortcutItemReference& rhs)
{
    return lhs.ItemShortcut == rhs.ItemShortcut && lhs.Key == rhs.Key;
}

UAGR_EquipmentManager::UAGR_EquipmentManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    SetIsReplicatedByDefault(true);
    UAGR_EquipmentManager::SetAutoActivate(true);
}

void UAGR_EquipmentManager::BeginPlay()
{
    Super::BeginPlay();
}

void UAGR_EquipmentManager::TickComponent(
    const float DeltaTime,
    const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAGR_EquipmentManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, EquipmentList);
    DOREPLIFETIME(ThisClass, WeaponShortcutReferences);
    DOREPLIFETIME(ThisClass, MagicShortcutReferences);
}

bool UAGR_EquipmentManager::GetAllItems(TArray<AActor*>& OutItems)
{
    OutItems.Reset(EquipmentList.Num());

    for(const FEquipment& EquipmentItem : EquipmentList)
    {
        if(IsValid(EquipmentItem.ItemActor))
        {
            OutItems.Add(EquipmentItem.ItemActor);
        }
    }

    return OutItems.Num() > 0;
}

bool UAGR_EquipmentManager::EquipItemInSlot(
    const FName Slot,
    AActor* ItemActor,
    AActor*& OutPreviousItem,
    AActor*& OutNewItem)
{
    if(!IsValid(GetOwner()) || !GetOwner()->HasAuthority() || !IsValid(ItemActor))
    {
        return false;
    }

    AActor* ItemInSlot;
    if(GetItemInSlot(Slot, ItemInSlot))
    {
        if(ItemInSlot == ItemActor)
        {
            // Same item is already equipped
            return false;
        }
    }

    for(int32 i = 0; i < EquipmentList.Num(); ++i)
    {
        const FEquipment& EquipmentElement = EquipmentList[i];

        if(Slot != EquipmentElement.Id)
        {
            continue;
        }

        if(!ItemActor->ActorHasTag(UAGR_ItemComponent::TAG_ITEM))
        {
            continue;
        }

        const UAGR_ItemComponent* ItemComponent = UAGRLibrary::GetItemComponent(ItemActor);
        if(!IsValid(ItemComponent))
        {
            continue;
        }

        const bool bMatches = ItemComponent->ItemTagSlotType.MatchesAny(EquipmentElement.AcceptableSlots);
        if(!bMatches)
        {
            continue;
        }

        // All conditions are met -> Start to equip item

        // Skip UnEquip if item is NOT valid (empty slot!)
        if(IsValid(EquipmentElement.ItemActor))
        {
            OutPreviousItem = EquipmentElement.ItemActor;
            const UAGR_ItemComponent* PreviousItemComponent = UAGRLibrary::GetItemComponent(OutPreviousItem);
            if(IsValid(PreviousItemComponent))
            {
                PreviousItemComponent->UnEquipInternal(Slot);
            }
        }

        EquipmentList[i].ItemActor = ItemActor;
        OutNewItem = ItemActor;

        ItemComponent->EquipInternal(Slot);

        // Item equipped successfully
        return true;
    }

    // Failed to equip item
    return false;
}

void UAGR_EquipmentManager::SetupDefineSlots(const TArray<FEquipment> InEquipmentList)
{
    if(!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
    {
        return;
    }

    EquipmentList = InEquipmentList;
}

bool UAGR_EquipmentManager::UnEquipItemFromSlot(const FName Slot, AActor*& OutItemUnequipped)
{
    if(!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
    {
        return false;
    }

    for(int32 i = 0; i < EquipmentList.Num(); ++i)
    {
        FEquipment& EquipmentElement = EquipmentList[i];

        if(Slot != EquipmentElement.Id)
        {
            continue;
        }

        if(!IsValid(EquipmentElement.ItemActor))
        {
            continue;
        }

        /* slot exists + item is an item + tag is acceptable in this slot */

        OutItemUnequipped = EquipmentElement.ItemActor;
        EquipmentElement.ItemActor = nullptr;

        const UAGR_ItemComponent* UnequippedItemComponent = UAGRLibrary::GetItemComponent(OutItemUnequipped);
        if(IsValid(UnequippedItemComponent))
        {
            UnequippedItemComponent->UnEquipInternal(Slot);
        }

        // Item unequipped successfully
        return true;
    }

    // Failed to UnEquip item
    return false;
}

bool UAGR_EquipmentManager::UnEquipByReference(AActor* ItemActor, FText& OutNote)
{
    if(!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
    {
        return false;
    }

    if(!IsValid(ItemActor))
    {
        // TODO OutNote should be an enum!
        OutNote = FText::FromString("Not a valid actor");
        return false;
    }

    /* Remove from array */

    for(int32 i = 0; i < EquipmentList.Num(); ++i)
    {
        FEquipment& EquipmentElement = EquipmentList[i];

        if(EquipmentElement.ItemActor != ItemActor)
        {
            continue;
        }

        EquipmentElement.ItemActor = nullptr;

        const UAGR_ItemComponent* UnequippedItemComponent = UAGRLibrary::GetItemComponent(ItemActor);
        if(IsValid(UnequippedItemComponent))
        {
            UnequippedItemComponent->UnEquipInternal(EquipmentElement.Id);
        }

        // Unequipped successfully
        OutNote = FText::FromString("Successfully UnEquipped");
        return true;
    }

    // Failed to UnEquip
    // TODO OutNote should be an enum!
    OutNote = FText::FromString("Not in equipment");
    return false;
}

bool UAGR_EquipmentManager::GetItemInSlot(const FName Slot, AActor*& OutItem)
{
    for(const FEquipment& EquipmentElement : EquipmentList)
    {
        if(EquipmentElement.Id != Slot)
        {
            continue;
        }

        // Found matching slot ID

        if(!IsValid(EquipmentElement.ItemActor))
        {
            // Failed to find item that is also valid
            return false;
        }

        OutItem = EquipmentElement.ItemActor;

        // Valid item found
        return true;
    }

    // Failed to find item
    return false;
}

void UAGR_EquipmentManager::SaveShortcutReference(const FName Key, AActor* Item)
{
    References.Add(Key, Item);
}

bool UAGR_EquipmentManager::SaveWeaponShortcutReference(FShortcutItemReference InItem)
{
    if (WeaponShortcutReferences.IsEmpty())
    {
        WeaponShortcutReferences.Add(InItem);
        UE_LOG(LogTemp, Warning, TEXT("Added!"));
        return true;
    }

    for (auto WeaponRef : WeaponShortcutReferences)
    {
        if (WeaponRef.Key == InItem.Key)
        {
            UE_LOG(LogTemp, Error, TEXT("Looped but NOT added"));
            continue;
        }
        else
        {
            WeaponShortcutReferences.Add(InItem);
            UE_LOG(LogTemp, Warning, TEXT("Looped but Added!"));
            return true;
        }
    }
    UE_LOG(LogTemp, Error, TEXT("Return false"));
    return false;
}

bool UAGR_EquipmentManager::GetWeaponShortcutReference(const FName Key, AActor*& OutItem)
{
    if (WeaponShortcutReferences.IsEmpty())
    {
        OutItem = nullptr;
        return false;
    }

    for (auto WeaponRef : WeaponShortcutReferences)
    {
        if (WeaponRef.Key == Key)
        {
            OutItem = WeaponRef.ItemShortcut;
            return true;
        }
        else
        {
            continue;
        }
    }
    return false;
}

bool UAGR_EquipmentManager::RemoveWeaponShortcutReference(const AActor* WeaponToRemove)
{
    if (!WeaponToRemove) return false;

    FShortcutItemReference RefToRemove = FShortcutItemReference();

    for (FShortcutItemReference Weapon : WeaponShortcutReferences)
    {
        if (Weapon.ItemShortcut == WeaponToRemove)
        {
            RefToRemove = Weapon;
            break;
        }
    }

    return WeaponShortcutReferences.Remove(RefToRemove) > 0;
}

bool UAGR_EquipmentManager::SaveMagicShortcutReference(FShortcutItemReference InItem)
{
    if (MagicShortcutReferences.IsEmpty())
    {
        MagicShortcutReferences.Add(InItem);
        UE_LOG(LogTemp, Warning, TEXT("Magic Added!"));
        return true;
    }

    for (auto MagicRef : MagicShortcutReferences)
    {
        if (MagicRef.Key == InItem.Key)
        {
            UE_LOG(LogTemp, Error, TEXT("Magic Looped but NOT added"));
            continue;
        }
        else
        {
            MagicShortcutReferences.Add(InItem);
            UE_LOG(LogTemp, Warning, TEXT("Magic Looped but Added!"));
            return true;
        }
    }
    return false;
}

bool UAGR_EquipmentManager::GetMagicShortcutReference(const FName Key, AActor*& OutItem)
{
    if (MagicShortcutReferences.IsEmpty())
    {
        OutItem = nullptr;
        return false;
    }

    for (auto MagicRef : MagicShortcutReferences)
    {
        if (MagicRef.Key == Key)
        {
            OutItem = MagicRef.ItemShortcut;
            return true;
        }
        else
        {
            continue;
        }
    }
    return false;
}

bool UAGR_EquipmentManager::RemoveMagicShortcutReference(const AActor* MagicToRemove)
{
    if (!MagicToRemove)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid MAGIC to remove"));
        return false;
    }

    FShortcutItemReference RefToRemove = FShortcutItemReference();

    for (FShortcutItemReference Magic : MagicShortcutReferences)
    {
        if (Magic.ItemShortcut == MagicToRemove)
        {
            RefToRemove = Magic;
            break;
        }
    }

    return MagicShortcutReferences.Remove(RefToRemove) > 0;
}

bool UAGR_EquipmentManager::GetShortcutReference(const FName Key, AActor*& OutActor)
{
    AActor** FoundActor = References.Find(Key);
    if(FoundActor == nullptr)
    {
        // Failed to get the reference: Key not found
        return false;
    }

    if(!IsValid(*FoundActor))
    {
        // Failed to get the reference: Key found but value invalid
        return false;
    }

    OutActor = *FoundActor;

    // Found actor reference
    return true;
}
