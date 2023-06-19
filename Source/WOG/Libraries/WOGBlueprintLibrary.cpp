// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/WOGBlueprintLibrary.h"
#include "WOG.h"
#include "Weapons/WOGBaseWeapon.h"
#include "Data/AGRLibrary.h"

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