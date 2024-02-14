// Fill out your copyright notice in the Description page of Project Settings.


#include "Buildables/WOGBaseBuildItem.h"
#include "Data/AGRLibrary.h"
#include "WOG.h"

AWOGBaseBuildItem::AWOGBaseBuildItem()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
	bNetLoadOnClient = false;

	ItemComponent = CreateDefaultSubobject <UAGR_ItemComponent>(TEXT("ItemComponent"));
	ItemComponent->bStackable = false;
	ItemComponent->MaxStack = 1;
}

void AWOGBaseBuildItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitBuildableData();
}

void AWOGBaseBuildItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ItemComponent && HasAuthority())
	{
		ItemComponent->ItemName = BuildName;
		ItemComponent->ItemTagSlotType = BuildData.ItemTag;

		ItemComponent->OnPickup.AddDynamic(this, &ThisClass::OnBuildablePickedUp);
		ItemComponent->OnItemUsed.AddDynamic(this, &ThisClass::OnBuildableUsed);
		ItemComponent->OnDestroy.AddDynamic(this, &ThisClass::OnBuildableDestroyed);
		ItemComponent->OnItemDropped.AddDynamic(this, &ThisClass::OnBuildableDestroyed);
		ItemComponent->OnEquip.AddDynamic(this, &ThisClass::OnBuildableEquipped);
		ItemComponent->OnUnEquip.AddDynamic(this, &ThisClass::OnBuildableUnequipped);
	}
}

void AWOGBaseBuildItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWOGBaseBuildItem::OnBuildablePickedUp(UAGR_InventoryManager* Inventory)
{
}

void AWOGBaseBuildItem::OnBuildableEquipped(AActor* User, FName SlotName)
{
}

void AWOGBaseBuildItem::OnBuildableUnequipped(AActor* User, FName SlotName)
{
}

void AWOGBaseBuildItem::OnBuildableUsed(AActor* User, FGameplayTag GameplayTag)
{
}

void AWOGBaseBuildItem::OnBuildableDestroyed()
{
}

FBuildables AWOGBaseBuildItem::ReturnBuildData_Implementation()
{
	return BuildData;
}

void AWOGBaseBuildItem::InitBuildableData()
{
	const FString BuildTablePath{ TEXT("Engine.DataTable'/Game/Data/Buildables/DT_Buildables.DT_Buildables'") };
	UDataTable* BuildTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *BuildTablePath));

	if (!BuildTableObject)
	{
		UE_LOG(WOGLogBuild, Error, TEXT("Invalid Build DataTable"));
		return;
	}

	TArray<FName> BuildNamesArray = BuildTableObject->GetRowNames();
	FBuildables* BuildDataRow = nullptr;

	for (auto BuildRowName : BuildNamesArray)
	{
		if (BuildRowName == BuildName)
		{
			BuildDataRow = BuildTableObject->FindRow<FBuildables>(BuildName, TEXT(""));
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
