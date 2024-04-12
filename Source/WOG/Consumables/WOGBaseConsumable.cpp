// Fill out your copyright notice in the Description page of Project Settings.

#include "Consumables/WOGBaseConsumable.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Data/AGRLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Abilities/WOGGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "PlayerController/WOGPlayerController.h"
#include "ActorComponents/WOGUIManagerComponent.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "Data/WOGGameplayTags.h"


AWOGBaseConsumable::AWOGBaseConsumable()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->InitSphereRadius(96.f);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->bHiddenInGame = false;

	ItemComponent->bStackable = true;
	ItemComponent->MaxStack = 100;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetIsReplicated(true);
}

void AWOGBaseConsumable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseConsumable, AbilityKey);
}

void AWOGBaseConsumable::InitData()
{
	OwnerCharacter = GetOwner() ? Cast<ABasePlayerCharacter>(GetOwner()) : nullptr;
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Consumable DataTable"));
		return;
	}

	TArray<FName> ConsumableNamesArray = ItemDataTable->GetRowNames();
	FConsumableDataTable* ConsumableDataRow = nullptr;

	for (auto ConsumableRowName : ConsumableNamesArray)
	{
		if (ConsumableRowName == ItemNames[ItemLevel])
		{
			ConsumableDataRow = ItemDataTable->FindRow<FConsumableDataTable>(ItemNames[ItemLevel], TEXT(""));
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

void AWOGBaseConsumable::UpdateVendorData(const FConsumableDataTable* Row)
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
		ItemComponent->ItemTagSlotType = ConsumableData.ConsumableTag;
	}
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

void AWOGBaseConsumable::OnItemEquipped(AActor* User, FName SlotName)
{
	ABasePlayerCharacter* NewOwnerCharacter = Cast<ABasePlayerCharacter>(User);
	if (NewOwnerCharacter)
	{
		SetOwnerCharacter(NewOwnerCharacter);
	}

	if (OwnerCharacter)
	{
		AddAbilityWidget(4);
		GrantAbilities(nullptr);

		AttachToActor(OwnerCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		SetActorHiddenInGame(true);

		TObjectPtr<AWOGPlayerController> OwnerPC = Cast<AWOGPlayerController>(OwnerCharacter->GetController());
		if (OwnerPC && ItemComponent)
		{
			ItemComponent->PreviousOwnerIndex = OwnerPC->UserIndex;
			UE_LOG(WOGLogInventory, Display, TEXT("New PREVIOUS_USER_INDEX for consumable: %d"), ItemComponent->PreviousOwnerIndex);
		}
	}
}

void AWOGBaseConsumable::OnItemUnequipped(AActor* User, FName SlotName)
{
	if (!OwnerCharacter || !OwnerCharacter->GetOwnerPC() || !OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()) return;
	RemoveGrantedAbilities(OwnerCharacter);
	OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()->Client_RemoveAbilityWidget(4);
}

void AWOGBaseConsumable::OnItemUsed(AActor* User, FGameplayTag GameplayTag)
{
	if (!ItemComponent || !HasAuthority() || !OwnerCharacter) return;

	if (ItemComponent->CurrentStack - 1 <= 0)
	{
		RemoveGrantedAbilities(OwnerCharacter);
		OwnerCharacter->GetOwnerPC()->GetUIManagerComponent()->Client_RemoveAbilityWidget(4);
		ItemComponent->DestroyItem();
	}
	else
	{
		ItemComponent->CurrentStack--;
	}

	UE_LOG(WOGLogInventory, Display, TEXT("Current stack: %d"), ItemComponent->CurrentStack);
}

bool AWOGBaseConsumable::GrantAbilities(AActor* User)
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

