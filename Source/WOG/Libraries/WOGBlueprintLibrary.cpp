// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/WOGBlueprintLibrary.h"
#include "WOG.h"
#include "Weapons/WOGBaseWeapon.h"
#include "Magic/WOGBaseMagic.h"
#include "Data/AGRLibrary.h"
#include "Characters/WOGBaseCharacter.h"
#include "Types/WOGGameplayTags.h"
#include "Consumables/WOGBaseConsumable.h"
#include "PlayerController/WOGPlayerController.h"

AWOGBaseWeapon* UWOGBlueprintLibrary::GetEquippedWeapon(const AActor* Owner)
{
	if (!Owner) return nullptr;

	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(Owner);
	if (!Equipment) return nullptr;

	AActor* OutEquippedActor;
	if (Equipment->GetItemInSlot(NAME_WeaponSlot_Primary, OutEquippedActor))
	{
		AWOGBaseWeapon* EquippedWeapon = Cast<AWOGBaseWeapon>(OutEquippedActor);
		return EquippedWeapon ? EquippedWeapon : nullptr;
	}

	return nullptr;
}

AWOGBaseMagic* UWOGBlueprintLibrary::GetEquippedMagic(const AActor* Owner)
{
	if (!Owner) return nullptr;

	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(Owner);
	if (!Equipment) return nullptr;

	AActor* OutEquippedActor;
	if (Equipment->GetItemInSlot(NAME_MagicSlot_MagicPrimary, OutEquippedActor))
	{
		AWOGBaseMagic* EquippedMagic = Cast<AWOGBaseMagic>(OutEquippedActor);
		return EquippedMagic ? EquippedMagic : nullptr;
	}

	return nullptr;
}

AWOGBaseConsumable* UWOGBlueprintLibrary::GetEquippedConsumable(const AActor* Owner)
{
	if (!Owner) return nullptr;
	UAGR_InventoryManager* Inventory = UAGRLibrary::GetInventory(Owner);
	if (!Inventory) return nullptr;


	TArray<AActor*> EquippedConsumables;
	int32 Amount = 0;
	Inventory->GetAllItemsOfTagSlotType(TAG_Inventory_Consumable, EquippedConsumables, Amount);
	if (EquippedConsumables.IsEmpty()) return nullptr;

	TObjectPtr<AWOGBaseConsumable> EquippedConsumable = Cast<AWOGBaseConsumable>(EquippedConsumables[0]);

	return EquippedConsumable;
}

FCharacterData UWOGBlueprintLibrary::GetCharacterData(AActor* Owner)
{
	if (!Owner) return FCharacterData();
	
	TObjectPtr<AWOGBaseCharacter> OwnerCharacter = Cast<AWOGBaseCharacter>(Owner);

	if (OwnerCharacter)
	{
		return OwnerCharacter->GetCharacterData();
	}

	return FCharacterData();
}

UWOGUIManagerComponent* UWOGBlueprintLibrary::GetUIManagerComponent(AController* Owner)
{
	if (!Owner) return nullptr;

	TObjectPtr<AWOGPlayerController> OwnerPC = Cast<AWOGPlayerController>(Owner);
	if(!OwnerPC) return nullptr;

	return OwnerPC->GetUIManagerComponent();
}
