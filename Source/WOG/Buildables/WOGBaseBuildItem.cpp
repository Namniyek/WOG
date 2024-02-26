// Fill out your copyright notice in the Description page of Project Settings.


#include "Buildables/WOGBaseBuildItem.h"
#include "Data/AGRLibrary.h"
#include "WOG.h"

void AWOGBaseBuildItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ItemComponent && HasAuthority())
	{
		ItemComponent->ItemTagSlotType = BuildData.ItemTag;
	}
}

FBuildables AWOGBaseBuildItem::ReturnBuildData_Implementation()
{
	return BuildData;
}

void AWOGBaseBuildItem::InitData()
{
	if (!ItemDataTable)
	{
		UE_LOG(WOGLogBuild, Error, TEXT("Invalid Build DataTable"));
		return;
	}

	TArray<FName> BuildNamesArray = ItemDataTable->GetRowNames();
	FBuildables* BuildDataRow = nullptr;

	for (auto BuildRowName : BuildNamesArray)
	{
		if (BuildRowName == ItemNames[ItemLevel])
		{
			BuildDataRow = ItemDataTable->FindRow<FBuildables>(ItemNames[ItemLevel], TEXT(""));
			break;
		}
	}
	if (BuildDataRow)
	{
		ItemComponent->CurrentStack = BuildDataRow->VendorItemData.ItemAmount;
		ItemComponent->ItemTagSlotType = BuildDataRow->ItemTag;

		BuildDataRow->VendorItemData.ItemIcon = BuildDataRow->Icon;
		BuildDataRow->VendorItemData.ItemTag = BuildDataRow->ItemTag;
		BuildDataRow->VendorItemData.BuildHealth = BuildDataRow->Health;

		BuildData = *BuildDataRow;
		UE_LOG(WOGLogBuild, Display, TEXT("BuildData Updated"));
	}
	else
	{
		UE_LOG(WOGLogBuild, Error, TEXT("Invalid BuildDataRow"));
	}
}
