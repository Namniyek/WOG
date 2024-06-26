// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/WOGBlueprintLibrary.h"
#include "WOG.h"
#include "Weapons/WOGBaseWeapon.h"
#include "Magic/WOGBaseMagic.h"
#include "Data/AGRLibrary.h"
#include "Characters/WOGBaseCharacter.h"
#include "Data/WOGGameplayTags.h"
#include "Consumables/WOGBaseConsumable.h"
#include "PlayerController/WOGPlayerController.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemBlueprintLibrary.h"

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
	UAGR_EquipmentManager* EquipmentManager = UAGRLibrary::GetEquipment(Owner);
	if (!EquipmentManager) return nullptr;

	AActor* OutItem;
	EquipmentManager->GetItemInSlot(NAME_ConsumableSlot_Consumable, OutItem);
	if (!OutItem) return nullptr;

	TObjectPtr<AWOGBaseConsumable> EquippedConsumable = Cast<AWOGBaseConsumable>(OutItem);
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

bool UWOGBlueprintLibrary::TryActivateAbilityByTagWithData(AActor* Target, const FGameplayTag& ActivationTag, FGameplayEventData AbilityData)
{
	if (!Target) return false;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!ASC) return false;

	FGameplayTagContainer ActivationTags = FGameplayTagContainer(ActivationTag);
	TArray<FGameplayAbilitySpec*> OutArray;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(ActivationTags, OutArray);

	if (OutArray.IsEmpty()) return false;

	return ASC->TryActivateAbility(OutArray[0]->Handle);
}
