// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGSpawnComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "TargetSystemComponent.h"
#include "TimerManager.h"
#include "WOG.h"
#include "ActorComponents/WOGEnemyOrderComponent.h"
#include "AI/Combat/WOGBaseSquad.h"
#include "Camera/CameraComponent.h"
#include "Components/AGR_EquipmentManager.h"
#include "Data/AGRLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Resources/WOGCommonInventory.h"
#include "WOG/Enemies/WOGBaseEnemy.h"
#include "WOG/PlayerCharacter/WOGAttacker.h"

UWOGSpawnComponent::UWOGSpawnComponent()
{
	bCanSpawn = false;
	bIsSpawnModeOn = false;
	SpawnID = 0;
	SpawnTransform = FTransform();

	SpawnGhost = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuidGhost"));
	SpawnGhost->SetStaticMesh(nullptr);
}

void UWOGSpawnComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWOGSpawnComponent, Spawnables);
	DOREPLIFETIME(UWOGSpawnComponent, SpawnID);
}

void UWOGSpawnComponent::BeginPlay()
{
	Super::BeginPlay();
	AttackerCharacter = OwnerCharacter == nullptr ? Cast<AWOGAttacker>(GetOwner()) : Cast<AWOGAttacker>(OwnerCharacter);

	if (AttackerCharacter)
	{
		Camera = AttackerCharacter->GetFollowCamera();
	}
}

void UWOGSpawnComponent::LaunchSpawnMode()
{
	if (!AttackerCharacter) return;

	if (bIsSpawnModeOn)
	{
		StopSpawnMode();
		if (APlayerController* PlayerController = Cast<APlayerController>(AttackerCharacter->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(AttackerCharacter->MainMappingContext, 0);
			}
		}
	}
	else
	{
		if (!SetSpawnables())
		{
			StopSpawnMode();
			if (APlayerController* PlayerController = Cast<APlayerController>(AttackerCharacter->GetController()))
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
				{
					Subsystem->ClearAllMappings();
					Subsystem->AddMappingContext(AttackerCharacter->MainMappingContext, 0);
					UE_LOG(LogTemp, Warning, TEXT("DefaultModeMC"));
				}
			}
			return;
		}

		AttackerCharacter->GetTargetComponent()->TargetLockOff();

		bIsSpawnModeOn = true;
		SpawnCycle();
		if (APlayerController* PlayerController = Cast<APlayerController>(AttackerCharacter->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(AttackerCharacter->SpawnModeMappingContext, 0);
			}
		}
	}
}

void UWOGSpawnComponent::StopSpawnMode()
{
	bIsSpawnModeOn = false;
	bCanSpawn = false;
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	if (SpawnGhost->GetStaticMesh())
	{
		SpawnGhost->SetStaticMesh(nullptr);
	}
}

void UWOGSpawnComponent::SpawnDelay()
{
	if (bIsSpawnModeOn)
	{
		float SpawnDelayTime = 0.05f;
		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::SpawnCycle, SpawnDelayTime);
	}
	else
	{
		StopSpawnMode();
	}
}

void UWOGSpawnComponent::SpawnCycle()
{
	if (!AttackerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackerCharacter invalid"));
		return;
	}

	FHitResult HitResult;
	FVector ForwardVector = Camera->GetForwardVector();
	FVector Start = (Camera->GetComponentLocation()) + (ForwardVector * 350);
	FVector End = (Camera->GetComponentLocation()) + (ForwardVector * 1500);

	ECollisionChannel Channel = ECollisionChannel::ECC_Visibility;
	FCollisionQueryParams Params;
	
	TArray<AActor*> ActorsToIgnore;
	AActor* WeaponOne;
	AActor* WeaponTwo;
	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(AttackerCharacter);

	if (Equipment)
	{
		if (Equipment->GetWeaponShortcutReference(FName("1"), WeaponOne) && WeaponOne)
		{
			ActorsToIgnore.AddUnique(WeaponOne);
		}
		if (Equipment->GetWeaponShortcutReference(FName("2"), WeaponTwo) && WeaponTwo)
		{
			ActorsToIgnore.AddUnique(WeaponTwo);
		}
	}

	Params.AddIgnoredActors(ActorsToIgnore);
	Params.bTraceComplex = false;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, Channel, Params);

	SpawnTransform.SetLocation((bHit == true ? HitResult.ImpactPoint : HitResult.TraceEnd));

	if (bHit)
	{
		CurrentHitActor = HitResult.GetActor();

		if (SpawnGhost->GetStaticMesh())
		{
			UWOGEnemyOrderComponent* OrderComp = AttackerCharacter->GetEnemyOrderComponent();

			bool bIsAllowed = !CheckForOverlap() && CheckCost() && (OrderComp && OrderComp->GetCurrentSquads().Num() < OrderComp->MaxAmountSquads);

			GiveSpawnColor(bIsAllowed);
			SpawnDelay();
		}
		else
		{
			SpawnSpawnGhost();
			SpawnDelay();
		}
	}

	if (!bHit)
	{
		if (SpawnGhost->GetStaticMesh())
		{
			GiveSpawnColor(false);
			SpawnDelay();
		}
		else
		{
			SpawnSpawnGhost();
			SpawnDelay();
		}
	}

}

void UWOGSpawnComponent::SpawnSpawnGhost()
{
	if (SpawnGhost)
	{
		SpawnGhost->SetStaticMesh(Spawnables[SpawnID].Mesh);
		SpawnGhost->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UWOGSpawnComponent::GiveSpawnColor(bool IsAllowed)
{
	if (!AllowedGhostMaterial || !ForbiddenGhostMaterial || !SpawnGhost) return;

	bCanSpawn = IsAllowed;

	for (int32 i = 0; i < SpawnGhost->GetNumMaterials(); i++)
	{
		SpawnGhost->SetMaterial(i, IsAllowed == true ? AllowedGhostMaterial : ForbiddenGhostMaterial); // Determine what material to use based on the bCnaSpawn bool.
	}

	SpawnGhost->SetWorldTransform(SpawnTransform);
}

void UWOGSpawnComponent::ChangeMesh(int32 ID)
{
	if (!SpawnGhost) return;

	SpawnGhost->SetStaticMesh(Spawnables[ID].Mesh);
}

bool UWOGSpawnComponent::CheckForOverlap()
{
	if (!SpawnGhost) return true;

	FVector Origin = FVector();
	FVector _BoxExtents = FVector();
	float _SphereRadius = 0.f;
	UKismetSystemLibrary::GetComponentBounds(SpawnGhost, Origin, _BoxExtents, _SphereRadius);

	FBoxSphereBounds Bounds = Spawnables[SpawnID].Mesh->GetBounds();
	TArray<AActor*> ActorsToIgnore = {};
	FHitResult HitResult;

	bool bHit = UKismetSystemLibrary::BoxTraceSingle(this,
		Origin,
		Origin,
		Bounds.BoxExtent / 2,
		SpawnTransform.GetRotation().Rotator(),
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), //Custom trace channel SpawningOverlapTrace
		false,
		ActorsToIgnore,
		EDrawDebugTrace::Type::None,
		HitResult,
		true
	);

	return bHit;
}

void UWOGSpawnComponent::Server_Spawn_Implementation(FTransform Transform, int32 ID)
{
	SpawnID = ID;
	Spawn(Transform, ID);
}

void UWOGSpawnComponent::Spawn(FTransform Transform, int32 ID)
{
	if (!AttackerCharacter)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Attacker character invalid"));
		return;
	}
	UWOGEnemyOrderComponent* OrderComp = AttackerCharacter->GetEnemyOrderComponent();
	if (!OrderComp)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Order component invalid"));
		return;
	}
	if (Spawnables[ID].MinionArray.IsEmpty())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("MinionArray empty"));
		return;
	}

	/*
	*Check how many active squads the attacker has
	*/
	if (OrderComp->GetCurrentSquads().Num() > OrderComp->MaxAmountSquads - 1)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("too many currently active squads"));
		return;
	}

	/*
	*Start by spawning the WOGSquadActor
	*/
	FActorSpawnParameters SquadParams;
	SquadParams.Owner = AttackerCharacter ? AttackerCharacter : nullptr;
	SquadParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector SquadLocation = Transform.GetLocation() + FVector(0.f, 0.f, Spawnables[ID].HeightOffset);
	TObjectPtr<AActor> SpawnedSquadActor = GetWorld()->SpawnActor<AActor>(SquadClass, SquadLocation, Transform.GetRotation().Rotator(), SquadParams);
	TObjectPtr<AWOGBaseSquad> SpawnedSquad = Cast<AWOGBaseSquad>(SpawnedSquadActor);
	int32 CurrentSlotIndex = 0;
	if (!SpawnedSquad)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("WOGSquad Not Spawned"));
		return;
	}

	//Register the spawned squad
	OrderComp->HandleCurrentSquads(SpawnedSquad, true);
	OrderComp->SetCurrentlySelectedSquad(SpawnedSquad);

	/*
	*Handle the actual spawn of the enemies
	*/

	for (int32 i = 0; i<Spawnables[ID].AmountUnits; i++)
	{
		Transform.SetLocation(Transform.GetLocation() + FVector((Spawnables[ID].AmountUnits*150), 0.f, Spawnables[ID].HeightOffset));

		FActorSpawnParameters Params;
		Params.Owner = AttackerCharacter ? AttackerCharacter : GetOwner();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		int32 Index = FMath::RandRange(0, Spawnables[ID].MinionArray.Num() - 1);
		TSubclassOf<AWOGBaseEnemy> MinionClass = Spawnables[ID].MinionArray[Index];

		TObjectPtr<AWOGBaseEnemy> SpawnedEnemy = GetWorld()->SpawnActor<AWOGBaseEnemy>(MinionClass, Transform, Params);

		if (SpawnedEnemy)
		{
			//Handle the squad assign logic and init
			FEnemyCombatSlot Slot;
			Slot.SlotIndex = CurrentSlotIndex;
			Slot.Location = SpawnedSquad->SlotComponentsArray[CurrentSlotIndex];
			Slot.CurrentEnemy = SpawnedEnemy;

			SpawnedSquad->SquadSlots.Add(Slot);

			CurrentSlotIndex++;

			//Handle the spawned enemy logic and init
			SpawnedEnemy->SetActorTransform(Slot.Location->GetComponentTransform());
			SpawnedEnemy->SetDefaultAbilitiesAndEffects(Spawnables[ID].DefaultAbilitiesAndEffects);
			SpawnedEnemy->GiveDefaultAbilities();
			SpawnedEnemy->ApplyDefaultEffects();
			SpawnedEnemy->SetOwnerAttacker(AttackerCharacter ? AttackerCharacter : nullptr);
			SpawnedEnemy->SetOwnerSquad(SpawnedSquad);
			SpawnedEnemy->SetSquadUnitIndex(Slot.SlotIndex);
			SpawnedEnemy->SetBaseDamage(Spawnables[ID].BaseDamage);
			SpawnedEnemy->SetAttackRange(Spawnables[ID].AttackRange);
			SpawnedEnemy->SetDefendRange(Spawnables[ID].DefendRange);
			SpawnedEnemy->SetDamageEffect(Spawnables[ID].DamageEffect);
			SpawnedEnemy->SetSecondaryDamageEffect(Spawnables[ID].SecondaryDamageEffect);
			SpawnedEnemy->SetCosmeticsDataAsset(Spawnables[ID].CosmeticsDataAsset);
			SpawnedEnemy->SetCharacterData(Spawnables[ID].CharacterData);
		}
	}

	SpawnedSquad->SetSquadType(Spawnables[ID].SquadType);
	SpawnedSquad->SquadIcon = Spawnables[ID].Icon;
	SpawnedSquad->SquadName = Spawnables[ID].Name;
	SpawnedSquad->SendOrder(EEnemyOrder::EEO_Hold, SpawnedSquad->GetTransform());

	DeductCost();
}

TArray<FVector> UWOGSpawnComponent::GetSpawnLocations(const FVector& MiddleLocation, float GridSize, int32 Amount)
{
	TArray<FVector> SpawnLocations;

	// Define the grid size and Offset
	float Offset = (Amount % 2 == 0) ? (GridSize / 2.0f) : 0.0f;

	// Calculate the number of points in each direction
	int32 AmountPointsPerDirection = floor(sqrt(Amount));

	// Generate the points
	for (int32 x = 0; x < AmountPointsPerDirection; x++)
	{
		for (int32 y = 0; y < AmountPointsPerDirection; y++)
		{
			// Calculate the x and y offsets based on the loop indices
			float X_Offset = (x - (AmountPointsPerDirection / 2.0f)) * GridSize + Offset;
			float Y_Offset = (y - (AmountPointsPerDirection / 2.0f)) * GridSize + Offset;

			// Generate the point around the origin
			FVector SpawnLocation = FVector(MiddleLocation.X + X_Offset,
				MiddleLocation.Y + Y_Offset,
				MiddleLocation.Z);

			// Do something with the new point (e.g. add it to an array, spawn an actor at the location, etc.)
			SpawnLocations.Add(SpawnLocation);
		}
	}
	return SpawnLocations;
}

void UWOGSpawnComponent::Server_SetSpawnables_Implementation(const TArray<FSpawnables>& InSpawnables)
{
	Spawnables = InSpawnables;
}

bool UWOGSpawnComponent::SetSpawnables()
{
	Spawnables.Empty();
	SpawnID = 0;

	if (!GetOwner()) return false;

	UAGR_InventoryManager* Inventory = UAGRLibrary::GetInventory(GetOwner());
	if (!Inventory) return false;

	TArray<AActor*> OutItems;
	int32 Amount = 0;
	Inventory->GetAllItemsOfTagSlotType(TAG_Inventory_Spawnable, OutItems, Amount);

	if (OutItems.IsEmpty())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("No Spawn items in the inventory"));
		return false;
	}

	FSpawnables TempSpawnable;

	for (auto Item : OutItems)
	{
		if (!IsValid(Item) || !Item->Implements<USpawnInterface>())
		{
			UE_LOG(WOGLogSpawn, Error, TEXT("Item doesn't implement SpawnInterface"));
			continue;
		}

		UE_LOG(WOGLogSpawn, Display, TEXT("Item name: %s"), *GetNameSafe(Item));
		Spawnables.Add(ISpawnInterface::Execute_ReturnSpawnData(Item));

	}

	if (Spawnables.IsEmpty())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("Inventory loop done, but no results"));
		return false;
	}

	UE_LOG(WOGLogSpawn, Display, TEXT("Spawnables amount: %d"), Spawnables.Num());

	for (auto Spawnable : Spawnables)
	{
		UE_LOG(WOGLogSpawn, Display, TEXT("Spawnable name: %s"), *Spawnable.VendorItemData.DisplayName.ToString());
	}

	Server_SetSpawnables(Spawnables);
	return true;
}

bool UWOGSpawnComponent::CheckCost()
{
	if (!AttackerCharacter || !AttackerCharacter->GetCommonInventory())
	{
		UE_LOG(LogTemp, Error, TEXT("Attacker or CommonInventory actor invalid"));
		return false;
	}

	TObjectPtr<UAGR_InventoryManager> Inventory = UAGRLibrary::GetInventory(AttackerCharacter->GetCommonInventory());
	if (!Inventory)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get Inventory component from CommonInventory actor"));
		return false;
	}

	FText Note;
	bool bCanAfford = Inventory->HasEnoughItemsWithTagSlotType(Spawnables[SpawnID].CostTag, Spawnables[SpawnID].CostAmount, Note);
	return bCanAfford;
}

void UWOGSpawnComponent::DeductCost()
{
	if (!AttackerCharacter || !AttackerCharacter->GetCommonInventory())
	{
		UE_LOG(LogTemp, Error, TEXT("Attacker or CommonInventory actor invalid"));
		return;
	}

	TObjectPtr<UAGR_InventoryManager> Inventory = UAGRLibrary::GetInventory(AttackerCharacter->GetCommonInventory());
	if (!Inventory)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get Inventory component from CommonInventory actor"));
		return;
	}

	FText Note;
	Inventory->RemoveItemsWithTagSlotType(Spawnables[SpawnID].CostTag, Spawnables[SpawnID].CostAmount, Note);
	UE_LOG(LogTemp, Display, TEXT("%s"), *Note.ToString());
}

void UWOGSpawnComponent::PlaceSpawn() const
{
	if (bCanSpawn && bIsSpawnModeOn)
	{
		const FGameplayEventData Payload;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), TAG_Event_Summon, Payload);
	}
}

void UWOGSpawnComponent::HandleSpawnRotation(const bool bRotateLeft)
{
	if (!bIsSpawnModeOn || !SpawnGhost->GetStaticMesh()) return;

	FRotator NewSpawnRotation = FRotator();

	if (bRotateLeft)
	{
		NewSpawnRotation = SpawnTransform.GetRotation().Rotator() - FRotator(0.f, 45.f, 0.f);
	}
	else
	{
		NewSpawnRotation = SpawnTransform.GetRotation().Rotator() + FRotator(0.f, 45.f, 0.f);
	}

	SpawnTransform.SetRotation(FQuat::MakeFromRotator(NewSpawnRotation));
}
