// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/WOGBaseInventoryItem.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "Data/AGRLibrary.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "Subsystems/WOGWorldSubsystem.h"

AWOGBaseInventoryItem::AWOGBaseInventoryItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bNetLoadOnClient = false;

	ItemComponent = CreateDefaultSubobject <UAGR_ItemComponent>(TEXT("ItemComponent"));
	ItemComponent->bStackable = false;
	ItemComponent->MaxStack = 1;
	ItemComponent->ItemTagSlotType = FGameplayTag();

	ItemLevel = 0;
	ItemDataTable = nullptr;
}

void AWOGBaseInventoryItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitData();
}

void AWOGBaseInventoryItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ItemComponent && HasAuthority())
	{
		ItemComponent->OnPickup.AddDynamic(this, &ThisClass::OnItemPickedUp);
		ItemComponent->OnItemUsed.AddDynamic(this, &ThisClass::OnItemUsed);
		ItemComponent->OnDestroy.AddDynamic(this, &ThisClass::OnItemDestroyed);
		ItemComponent->OnItemDropped.AddDynamic(this, &ThisClass::OnItemDestroyed);
		ItemComponent->OnEquip.AddDynamic(this, &ThisClass::OnItemEquipped);
		ItemComponent->OnUnEquip.AddDynamic(this, &ThisClass::OnItemUnequipped);
	}
}

void AWOGBaseInventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseInventoryItem, ItemLevel);
	DOREPLIFETIME(AWOGBaseInventoryItem, OwnerCharacter);
}

void AWOGBaseInventoryItem::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = OwnerCharacter != nullptr ? OwnerCharacter : GetOwner() ? (TObjectPtr<ABasePlayerCharacter>) Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;

	TObjectPtr<UWOGWorldSubsystem> WorldSubsystem = GetWorld()->GetSubsystem<UWOGWorldSubsystem>();
	if (WorldSubsystem && HasAuthority())
	{
		WorldSubsystem->OnItemUpgradedDelegate.AddDynamic(this, &ThisClass::SetItemLevel);
	}
}

void AWOGBaseInventoryItem::SetOwnerCharacter(ABasePlayerCharacter* NewOwner)
{
	if (NewOwner)
	{
		OwnerCharacter = NewOwner;
	}
}

void AWOGBaseInventoryItem::SetItemLevel(FGameplayTag ItemTag, int32 NewLevel)
{
	if (!HasAuthority()) return;
	if(!ItemTag.MatchesTagExact(ItemComponent->ItemTagSlotType)) return;

	ItemLevel = NewLevel>ItemLevel ? NewLevel : ItemLevel;
	UE_LOG(WOGLogInventory, Display, TEXT("Item: %s upgraded to level: %d"), *GetNameSafe(this), ItemLevel);
	InitData();
}

