// Fill out your copyright notice in the Description page of Project Settings.

#include "Consumables/WOGBaseConsumable.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "WOG/PlayerCharacter/BasePlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Data/AGRLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags.h"
#include "PlayerController/WOGPlayerController.h"
#include "ActorComponents/WOGUIManagerComponent.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "Types/WOGGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"


AWOGBaseConsumable::AWOGBaseConsumable()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bNetLoadOnClient = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->InitSphereRadius(96.f);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->bHiddenInGame = false;

	ItemComponent = CreateDefaultSubobject <UAGR_ItemComponent>(TEXT("ItemComponent"));
	ItemComponent->bStackable = true;
	ItemComponent->MaxStack = 100;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetIsReplicated(true);
}

void AWOGBaseConsumable::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitConsumableData();
}

void AWOGBaseConsumable::InitConsumableData()
{
	OwnerCharacter = GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;

	const FString ConsumableTablePath{ TEXT("Engine.DataTable'/Game/Data/Consumables/DT_Consumables.DT_Consumables'") };
	UDataTable* ConsumableTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConsumableTablePath));

	if (!ConsumableTableObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Consumable DataTable"));
		return;
	}

	TArray<FName> ConsumableNamesArray = ConsumableTableObject->GetRowNames();
	FConsumableDataTable* ConsumableDataRow = nullptr;

	for (auto ConsumableRowName : ConsumableNamesArray)
	{
		if (ConsumableRowName == ConsumableName)
		{
			ConsumableDataRow = ConsumableTableObject->FindRow<FConsumableDataTable>(ConsumableName, TEXT(""));
			break;
		}
	}
	if (ConsumableDataRow)
	{
		Mesh->SetStaticMesh(ConsumableDataRow->MeshMain);
		ItemComponent->CurrentStack = ConsumableDataRow->VendorItemData.ItemAmount;

		ConsumableDataRow->VendorItemData.ItemIcon = ConsumableDataRow->AbilityIcon;
		ConsumableDataRow->VendorItemData.ItemTag = ConsumableDataRow->ConsumableTag;
		ConsumableDataRow->VendorItemData.bIsAttacker = ConsumableDataRow->bIsAttacker;
		ConsumableDataRow->VendorItemData.BaseConsumableValue = ConsumableDataRow->Value;
		ConsumableDataRow->VendorItemData.ConsumableDuration = ConsumableDataRow->Duration;

		ConsumableData = *ConsumableDataRow;
	}
}

void AWOGBaseConsumable::UpdateVendorData(FConsumableDataTable* Row)
{
	if (Row)
	{
		ConsumableData.VendorItemData.ItemIcon = Row->VendorItemData.ItemIcon;
		ConsumableData.VendorItemData.ItemTag = Row->VendorItemData.ItemTag;
		ConsumableData.VendorItemData.BaseConsumableValue = Row->VendorItemData.BaseConsumableValue;
		ConsumableData.VendorItemData.ConsumableDuration = Row->VendorItemData.ConsumableDuration;
		ConsumableData.VendorItemData.bIsAttacker = Row->VendorItemData.bIsAttacker;
	}
}

void AWOGBaseConsumable::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (SphereComponent && HasAuthority())
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnConsumableOverlap);
	}

	if (ItemComponent && HasAuthority())
	{
		ItemComponent->ItemName = ConsumableName;
		ItemComponent->ItemTagSlotType = ConsumableData.ConsumableTag;

		ItemComponent->OnPickup.AddDynamic(this, &ThisClass::OnConsumablePickedUp);
		ItemComponent->OnItemUsed.AddDynamic(this, &ThisClass::OnConsumableUsed);
		ItemComponent->OnDestroy.AddDynamic(this, &ThisClass::OnConsumableDestroyed);
		ItemComponent->OnItemDropped.AddDynamic(this, &ThisClass::OnConsumableDestroyed);
		
	}
}

void AWOGBaseConsumable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseConsumable, OwnerCharacter);
	DOREPLIFETIME(AWOGBaseConsumable, AbilityKey);
}

void AWOGBaseConsumable::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = OwnerCharacter != nullptr ? OwnerCharacter : GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;

}

void AWOGBaseConsumable::OnConsumableOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !ItemComponent || !OtherActor) return;

	bool bIsActorAttacker = UWOGBlueprintLibrary::GetCharacterData(OtherActor).bIsAttacker;
	if (ConsumableData.bIsAttacker != bIsActorAttacker)
	{
		UE_LOG(WOGLogInventory, Error, TEXT("Consumable->bIsAttacker != OtherActor->bIsAttacker"));
		return;
	}

	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(OtherActor);
	UAGR_InventoryManager* Inventory = UAGRLibrary::GetInventory(OtherActor);

	if (!Equipment || !Inventory)
	{
		UE_LOG(WOGLogInventory, Error, TEXT("Equipment or Inventory not valid"));
		return;
	}

	TArray<AActor*> OutItems = {};
	int32 Amount = 0;
	Inventory->GetAllItemsOfTagSlotType(TAG_Inventory_Consumable, OutItems, Amount);
	if (!OutItems.IsEmpty())
	{
		UE_LOG(WOGLogInventory, Error, TEXT("Consumable slot already filled"));
		return;
	}

	ABasePlayerCharacter* NewOwnerCharacter = Cast<ABasePlayerCharacter>(OtherActor);
	if (NewOwnerCharacter)
	{
		SetOwnerCharacter(NewOwnerCharacter);
	}
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetGenerateOverlapEvents(false);
	ItemComponent->PickUpItem(Inventory);
}

void AWOGBaseConsumable::OnConsumablePickedUp(UAGR_InventoryManager* Inventory)
{
	ABasePlayerCharacter* NewOwnerCharacter = Cast<ABasePlayerCharacter>(Inventory->GetOwner());
	if (NewOwnerCharacter)
	{
		SetOwnerCharacter(NewOwnerCharacter);
	}

	if (OwnerCharacter)
	{
		AddAbilityWidget(4);
		GrantAbilities();
	}
}

void AWOGBaseConsumable::OnConsumableUsed(AActor* User, FGameplayTag GameplayTag)
{
	if (!ItemComponent || !OwnerCharacter) return;
	TObjectPtr<UAGR_InventoryManager> Inventory = UAGRLibrary::GetInventory(OwnerCharacter);
	if (!Inventory) return;

	//if (ItemComponent->CurrentStack - 1 <= 0)
	//{
		//ItemComponent->DestroyItem();
	//}

	FText OutNote;
	Inventory->RemoveItemsOfClass(this->StaticClass(), 1, OutNote);

	UE_LOG(WOGLogInventory, Display, TEXT("%s, Current stack: %d"), *OutNote.ToString(), ItemComponent->CurrentStack);
}

void AWOGBaseConsumable::OnConsumableDestroyed()
{
	UE_LOG(WOGLogInventory, Display, TEXT("Consumable destroyed"));
	if (!OwnerCharacter || !OwnerCharacter->GetOwnerPC() || !OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()) return;
	RemoveGrantedAbilities(OwnerCharacter);
	OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()->Client_RemoveAbilityWidget(4);
}

bool AWOGBaseConsumable::GrantAbilities()
{
	if (!HasAuthority() || ConsumableData.AbilitiesToGrant.IsEmpty()) return false;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
	if (!ASC) return false;
	for (auto Ability : ConsumableData.AbilitiesToGrant)
	{
		if (!Ability) continue;
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), OwnerCharacter);
		FGameplayAbilitySpecHandle GrantedAbility = ASC->GiveAbility(AbilitySpec);
		UE_LOG(LogTemp, Display, TEXT("Ability granted: %s on %s"), *Ability.GetDefaultObject()->GetName(), *UEnum::GetValueAsString(OwnerCharacter->GetLocalRole()));
	}
	return true;
}

bool AWOGBaseConsumable::RemoveGrantedAbilities(AActor* User)
{
	if (!HasAuthority())
	{
		UE_LOG(WOGLogInventory, Error, TEXT("No authority on %s"), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()));
		return false;
	}	

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(User);
	if (!ASC) return false;
	for (auto Class : ConsumableData.AbilitiesToGrant)
	{
		FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(Class);
		if (!AbilitySpec) continue;
		FGameplayAbilitySpecHandle Handle = AbilitySpec->Handle;
		ASC->ClearAbility(Handle);
		UE_LOG(LogTemp, Display, TEXT("Ability cleared: %s"), *Handle.ToString())
	}

	return true;
}

void AWOGBaseConsumable::AddAbilityWidget(const int32& Key)
{
	if (!OwnerCharacter || !OwnerCharacter->GetOwnerPC() || !OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()) return;
	if (Key != 4) return;

	AbilityKey = Key;

	OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()->Client_AddAbilityWidget(AbilityKey, ConsumableData.AbilityWidgetClass, ConsumableData.AbilityIcon, ConsumableData.Cooldown, ConsumableData.CooldownTag);
}

void AWOGBaseConsumable::SetOwnerCharacter(ABasePlayerCharacter* NewOwner)
{
	if (NewOwner)
	{
		OwnerCharacter = NewOwner;
	}
}

