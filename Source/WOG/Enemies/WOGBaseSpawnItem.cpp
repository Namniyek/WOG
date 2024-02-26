// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/WOGBaseSpawnItem.h"
#include "Data/AGRLibrary.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"

void AWOGBaseSpawnItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ItemComponent && HasAuthority())
	{
		ItemComponent->ItemTagSlotType = SpawnData.ItemTag;
	}
}

void AWOGBaseSpawnItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseSpawnItem, SpawnData);
}

FSpawnables AWOGBaseSpawnItem::ReturnSpawnData_Implementation()
{
	return SpawnData;
}

void AWOGBaseSpawnItem::InitData()
{
	if (!ItemDataTable)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid Spawn DataTable"));
		return;
	}

	if (ItemLevel >= ItemNames.Num())
	{
		return;
	}

	TArray<FName> SpawnNamesArray = ItemDataTable->GetRowNames();
	FSpawnables* SpawnDataRow = nullptr;

	for (auto SpawnRowName : SpawnNamesArray)
	{
		if (SpawnRowName == ItemNames[ItemLevel])
		{
			SpawnDataRow = ItemDataTable->FindRow<FSpawnables>(ItemNames[ItemLevel], TEXT(""));
			break;
		}
	}
	if (SpawnDataRow)
	{
		ItemComponent->CurrentStack = SpawnDataRow->VendorItemData.ItemAmount;
		ItemComponent->ItemTagSlotType = SpawnDataRow->ItemTag;

		//SpawnDataRow->VendorItemData.ItemIcon = SpawnDataRow->Icon;
		SpawnDataRow->VendorItemData.ItemTag = SpawnDataRow->ItemTag;
		SpawnDataRow->VendorItemData.AmountUnits = SpawnDataRow->AmountUnits;

		SpawnData = *SpawnDataRow;
		UE_LOG(WOGLogSpawn, Display, TEXT("SpawnData Updated"));
	}
	else
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid SpawnDataRow"));
	}
}

