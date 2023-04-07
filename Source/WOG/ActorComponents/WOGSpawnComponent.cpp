// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGSpawnComponent.h"
#include "WOG/PlayerCharacter/WOGAttacker.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Camera/CameraComponent.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "WOG/ActorComponents/WOGCombatComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "WOG/Enemies/WOGBaseEnemy.h"
#include "WOG/Interfaces/AttributesInterface.h"

UWOGSpawnComponent::UWOGSpawnComponent()
{
	bCanSpawn = false;
	bIsSpawnModeOn = false;
	SpawnID = 0;
	SpawnTransform = FTransform();

	SpawnGhost = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuidGhost"));
	SpawnGhost->SetStaticMesh(nullptr);
}

void UWOGSpawnComponent::BeginPlay()
{
	Super::BeginPlay();
	AttackerCharacter = OwnerCharacter == nullptr ? Cast<AWOGAttacker>(GetOwner()) : Cast<AWOGAttacker>(OwnerCharacter);

	if (AttackerCharacter)
	{
		Camera = AttackerCharacter->GetFollowCamera();
	}

	if (SpawnablesDataTable)
	{
		{
			SpawnablesDataTable->GetAllRows<FSpawnables>(TEXT("Spawnables"), Spawnables);
			LastIndexDataTable = Spawnables.Num() - 1;
		}
	}
}

void UWOGSpawnComponent::LaunchSpawnMode()
{
	if (bIsSpawnModeOn)
	{
		StopSpawnMode();
	}
	else
	{
		bIsSpawnModeOn = true;
		SpawnCycle();
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
	TObjectPtr<UWOGCombatComponent> Combat = AttackerCharacter->GetCombatComponent();
	ActorsToIgnore.AddUnique(Combat->GetMainWeapon());
	ActorsToIgnore.AddUnique(Combat->GetSecondaryWeapon());

	Params.AddIgnoredActors(ActorsToIgnore);
	Params.bTraceComplex = false;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, Channel, Params);

	SpawnTransform.SetLocation((bHit == true ? HitResult.ImpactPoint : HitResult.TraceEnd));

	if (bHit)
	{
		CurrentHitActor = HitResult.GetActor();

		if (SpawnGhost->GetStaticMesh())
		{
			bool bIsAllowed = !CheckForOverlap();

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
		SpawnGhost->SetStaticMesh(Spawnables[SpawnID]->Mesh);
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

void UWOGSpawnComponent::ChangeMesh()
{
	if (!SpawnGhost) return;

	SpawnGhost->SetStaticMesh(Spawnables[SpawnID]->Mesh);
}

bool UWOGSpawnComponent::CheckForOverlap()
{
	if (!SpawnGhost) return true;

	FVector Origin = FVector();
	FVector _BoxExtents = FVector();
	float _SphereRadius = 0.f;
	UKismetSystemLibrary::GetComponentBounds(SpawnGhost, Origin, _BoxExtents, _SphereRadius);

	FBoxSphereBounds Bounds = Spawnables[SpawnID]->Mesh->GetBounds();
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
	Spawn(Transform, ID);
}

void UWOGSpawnComponent::Spawn(FTransform Transform, int32 ID)
{

	TArray<FVector> Spawns = GetSpawnLocations(Transform.GetLocation(), 100, Spawnables[ID]->AmountUnits);

	for (auto Spawn : Spawns)
	{
		Transform.SetLocation(Spawn + FVector(0.f, 0.f, Spawnables[ID]->HeightOffset));

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		TObjectPtr<AActor> SpawnedActor = GetWorld()->SpawnActor<AActor>(Spawnables[ID]->Actor, Transform, Params);
		UE_LOG(LogTemp, Warning, TEXT("Spawned at: %s"), *Spawn.ToString());

		/*if (SpawnedActor->GetClass()->ImplementsInterface(UAttributesInterface::StaticClass()))
		{
			IAttributesInterface::Execute_SetMaxHealth(SpawnedActor,Spawnables[ID]->MaxHealth);
			UE_LOG(LogTemp, Error, TEXT("SetMaxHealth() from SpawnCOmponent"));
		}*/
	}
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

void UWOGSpawnComponent::PlaceSpawn()
{
	if (bCanSpawn && bIsSpawnModeOn)
	{
		Server_Spawn(SpawnTransform, SpawnID);
	}
}

void UWOGSpawnComponent::HandleSpawnRotation(bool bRotateLeft)
{
	if (!bIsSpawnModeOn || !SpawnGhost->GetStaticMesh()) return;

	FRotator NewSpawnRotation = FRotator();

	if (bRotateLeft)
	{
		NewSpawnRotation = SpawnTransform.GetRotation().Rotator() - FRotator(0.f, 5.f, 0.f);
	}
	else
	{
		NewSpawnRotation = SpawnTransform.GetRotation().Rotator() + FRotator(0.f, 5.f, 0.f);
	}

	SpawnTransform.SetRotation(FQuat::MakeFromRotator(NewSpawnRotation));
}
