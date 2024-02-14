// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/WOGBaseSpawnItem.h"
#include "Data/AGRLibrary.h"
#include "WOG.h"

AWOGBaseSpawnItem::AWOGBaseSpawnItem()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
	bNetLoadOnClient = false;

	ItemComponent = CreateDefaultSubobject <UAGR_ItemComponent>(TEXT("ItemComponent"));
	ItemComponent->bStackable = false;
	ItemComponent->MaxStack = 1;
}

void AWOGBaseSpawnItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitSpawnableData();
}

void AWOGBaseSpawnItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ItemComponent && HasAuthority())
	{
		ItemComponent->ItemName = SpawnName;
		ItemComponent->ItemTagSlotType = SpawnData.ItemTag;

		ItemComponent->OnPickup.AddDynamic(this, &ThisClass::OnSpawnablePickedUp);
		ItemComponent->OnItemUsed.AddDynamic(this, &ThisClass::OnSpawnableUsed);
		ItemComponent->OnDestroy.AddDynamic(this, &ThisClass::OnSpawnableDestroyed);
		ItemComponent->OnItemDropped.AddDynamic(this, &ThisClass::OnSpawnableDestroyed);
		ItemComponent->OnEquip.AddDynamic(this, &ThisClass::OnSpawnableEquipped);
		ItemComponent->OnUnEquip.AddDynamic(this, &ThisClass::OnSpawnableUnequipped);
	}
}

void AWOGBaseSpawnItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWOGBaseSpawnItem::OnSpawnablePickedUp(UAGR_InventoryManager* Inventory)
{
}

void AWOGBaseSpawnItem::OnSpawnableEquipped(AActor* User, FName SlotName)
{
}

void AWOGBaseSpawnItem::OnSpawnableUnequipped(AActor* User, FName SlotName)
{
}

void AWOGBaseSpawnItem::OnSpawnableUsed(AActor* User, FGameplayTag GameplayTag)
{
}

void AWOGBaseSpawnItem::OnSpawnableDestroyed()
{
}

FSpawnables AWOGBaseSpawnItem::ReturnSpawnData_Implementation()
{
	return SpawnData;
}

void AWOGBaseSpawnItem::InitSpawnableData()
{
	const FString SpawnTablePath{ TEXT("Engine.DataTable'/Game/Data/Spawnables/DT_Spawnables.DT_Spawnables'") };
	UDataTable* SpawnTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *SpawnTablePath));

	if (!SpawnTableObject)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Invalid Spawn DataTable"));
		return;
	}

	TArray<FName> SpawnNamesArray = SpawnTableObject->GetRowNames();
	FSpawnables* SpawnDataRow = nullptr;

	for (auto SpawnRowName : SpawnNamesArray)
	{
		if (SpawnRowName == SpawnName)
		{
			SpawnDataRow = SpawnTableObject->FindRow<FSpawnables>(SpawnName, TEXT(""));
			break;
		}
	}
	if (SpawnDataRow)
	{
		ItemComponent->CurrentStack = SpawnDataRow->VendorItemData.ItemAmount;
		ItemComponent->ItemTagSlotType = SpawnDataRow->ItemTag;

		SpawnDataRow->VendorItemData.ItemIcon = SpawnDataRow->Icon;
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

