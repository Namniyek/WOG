// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBuildComponent.h"
#include "WOG/PlayerCharacter/WOGDefender.h"
#include "Engine/World.h"
#include "WOG/Interfaces/BuildingInterface.h"
#include "Engine/EngineTypes.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "WOG/Weapons/WOGBaseWeapon.h"
#include "EnhancedInputSubsystems.h"
#include "Data/AGRLibrary.h"
#include "Components/AGR_EquipmentManager.h"

UWOGBuildComponent::UWOGBuildComponent()
{
	bCanBuild = false;
	bIsBuildModeOn = false;
	BuildID = 0;
	BuildTransform = FTransform();
	HeightOffset = FVector();

	BuildGhost = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuidGhost"));
	BuildGhost->SetStaticMesh(nullptr);
}

void UWOGBuildComponent::BeginPlay()
{
	Super::BeginPlay();
	DefenderCharacter = OwnerCharacter == nullptr ? Cast<AWOGDefender>(GetOwner()) : Cast<AWOGDefender>(OwnerCharacter);

	if (DefenderCharacter)
	{
		Camera = DefenderCharacter->GetFollowCamera();
	}

	if (BuildablesDataTable)
	{
		BuildablesDataTable->GetAllRows<FBuildables>(TEXT("Buildables"), Buildables);
		LastIndexDataTable = Buildables.Num() - 1;
	}
}

void UWOGBuildComponent::LaunchBuildMode()
{
	if (bIsBuildModeOn)
	{
		StopBuildMode();
		if (APlayerController* PlayerController = Cast<APlayerController>(DefenderCharacter->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(DefenderCharacter->MatchMappingContext, 0);
			}
		}
	}
	else
	{
		bIsBuildModeOn = true;
		HeightOffset = FVector();
		BuildCycle();
		if (APlayerController* PlayerController = Cast<APlayerController>(DefenderCharacter->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(DefenderCharacter->SpawnModeMappingContext, 0);
			}
		}
	}
}

void UWOGBuildComponent::StopBuildMode()
{
	bIsBuildModeOn = false;
	bCanBuild = false;
	GetWorld()->GetTimerManager().ClearTimer(BuildTimerHandle);
	if (BuildGhost->GetStaticMesh())
	{
		BuildGhost->SetStaticMesh(nullptr);
	}
}

void UWOGBuildComponent::BuildDelay()
{
	if (bIsBuildModeOn)
	{
		float BuildDelayTime = 0.05f;
		GetWorld()->GetTimerManager().SetTimer(BuildTimerHandle, this, &ThisClass::BuildCycle, BuildDelayTime);
	}
	else
	{
		StopBuildMode();
	}
}

void UWOGBuildComponent::SpawnBuildGhost()
{
	if (BuildGhost)
	{
		BuildGhost->SetStaticMesh(Buildables[BuildID]->Mesh);
		BuildGhost->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UWOGBuildComponent::GiveBuildColor(bool IsAllowed)
{
	if (!AllowedGhostMaterial || !ForbiddenGhostMaterial || !BuildGhost) return;

	bCanBuild = IsAllowed;

	for (int32 i = 0; i < BuildGhost->GetNumMaterials(); i++)
	{
		BuildGhost->SetMaterial(i, IsAllowed == true ? AllowedGhostMaterial : ForbiddenGhostMaterial); // Determine what material to use based on the bCnaBuild bool.
	}

	BuildGhost->SetWorldTransform(BuildTransform);
}

void UWOGBuildComponent::ChangeMesh()
{
	if (!BuildGhost) return;

	BuildGhost->SetStaticMesh(Buildables[BuildID]->Mesh);
	HeightOffset = FVector();
}

void UWOGBuildComponent::BuildCycle()
{
	if (!DefenderCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("DefenderCharacter invalid"));
		return;
	}

	FHitResult HitResult;
	FVector ForwardVector = Camera->GetForwardVector();
	FVector Start = (Camera->GetComponentLocation()) + (ForwardVector*350);
	FVector End = (Camera->GetComponentLocation()) + (ForwardVector * 1500);

	ECollisionChannel Channel = UEngineTypes::ConvertToCollisionChannel(Buildables[BuildID]->TraceChannel);
	FCollisionQueryParams Params;

	TArray<AActor*> ActorsToIgnore;
	AActor* WeaponOne;
	AActor* WeaponTwo;
	UAGR_EquipmentManager* Equipment = UAGRLibrary::GetEquipment(DefenderCharacter);

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

	BuildTransform.SetLocation((bHit == true ? HitResult.ImpactPoint : HitResult.TraceEnd) + HeightOffset);

	if (bHit)
	{
		CurrentHitComponent = HitResult.GetComponent();
		CurrentHitActor = HitResult.GetActor();

		if (BuildGhost->GetStaticMesh())
		{
			bool bFound = false;
			FTransform Transform;
			DetectBuildBoxes(bFound, Transform);

			if (bFound)
			{
				BuildTransform.SetLocation(Transform.GetLocation() + HeightOffset);
				BuildTransform.SetRotation(Transform.GetRotation());
				BuildTransform.SetScale3D(Transform.GetScale3D());
			}
			else
			{
				CurrentHitActor = nullptr;
			}

			bool bIsAllowed = !CheckForOverlap() && IsBuildFloating();
			
			GiveBuildColor(bIsAllowed);
			BuildDelay();
		}
		else 
		{
			SpawnBuildGhost();
			BuildDelay();
		}
	}

	if(!bHit)
	{
		if (BuildGhost->GetStaticMesh())
		{
			GiveBuildColor(false);
			BuildDelay();
		}
		else
		{
			SpawnBuildGhost();
			BuildDelay();
		}
	}
}

void UWOGBuildComponent::DetectBuildBoxes(bool& OutFound, FTransform& OutTransform)
{
	bool bLocalFound = false;

	if (CurrentHitActor->GetClass()->ImplementsInterface(UBuildingInterface::StaticClass()))
	{
		TArray<UBoxComponent*> CollisionBoxArray = IBuildingInterface::Execute_ReturnCollisionBoxes(CurrentHitActor);

		for (auto Box : CollisionBoxArray)
		{
			if (Box == Cast<UBoxComponent>(CurrentHitComponent))
			{
				bLocalFound = true;
				break;
			}
		}
		OutFound = bLocalFound;
		OutTransform = CurrentHitComponent->GetComponentTransform();
	}
}

bool UWOGBuildComponent::CheckForOverlap()
{
	if (!BuildGhost) return true;

	FVector Origin = FVector();
	FVector _BoxExtents = FVector();
	float _SphereRadius = 0.f;
	UKismetSystemLibrary::GetComponentBounds(BuildGhost, Origin, _BoxExtents, _SphereRadius);

	FBoxSphereBounds Bounds = Buildables[BuildID]->Mesh->GetBounds();
	TArray<AActor*> ActorsToIgnore = {};
	FHitResult HitResult;

	bool bHit = UKismetSystemLibrary::BoxTraceSingle(this,
		Origin,
		Origin,
		Bounds.BoxExtent / 2,
		BuildTransform.GetRotation().Rotator(),
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), //Custom trace channel BuildingOverlapTrace
		false,
		ActorsToIgnore,
		EDrawDebugTrace::Type::None,
		HitResult,
		true
	);

	return bHit;
}

bool UWOGBuildComponent::IsBuildFloating()
{
	if (!BuildGhost) return true;

	FVector Start = FVector();
	FVector _BoxExtents = FVector();
	float _SphereRadius = 0.f;
	UKismetSystemLibrary::GetComponentBounds(BuildGhost, Start, _BoxExtents, _SphereRadius);

	FBoxSphereBounds Bounds = Buildables[BuildID]->Mesh->GetBounds();
	FVector End = Start;
	End.Z = End.Z - Buildables[BuildID]->MaxHeightOffset;
	TArray<AActor*> ActorsToIgnore = {};
	FHitResult HitResult;

	bool bHit = UKismetSystemLibrary::BoxTraceSingle(this,
		Start,
		End,
		Bounds.BoxExtent / 1.2,
		BuildTransform.GetRotation().Rotator(),
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::Type::None,
		HitResult,
		true
	);

	return bHit;
}

void UWOGBuildComponent::PlaceBuildable()
{
	if (bCanBuild && bIsBuildModeOn)
	{
		Server_SpawnBuild(BuildTransform, BuildID, CurrentHitActor, CurrentHitComponent);
	}
}

void UWOGBuildComponent::Server_SpawnBuild_Implementation(FTransform Transform, int32 ID, AActor* Hit, UPrimitiveComponent* HitComponent)
{
	SpawnBuild(Transform, ID, Hit, HitComponent);
	HeightOffset = FVector();
	UE_LOG(LogTemp, Warning, TEXT("SpawnBuild() called"));
}

void UWOGBuildComponent::SpawnBuild(FTransform Transform, int32 ID, AActor* Hit, UPrimitiveComponent* HitComponent)
{
	TObjectPtr<AActor> SpawnedBuild =  GetWorld()->SpawnActor<AActor>(Buildables[ID]->Actor, Transform);
	HeightOffset = FVector();

	if (!SpawnedBuild)
	{
		UE_LOG(LogTemp, Error, TEXT("Nothing Spawned"));
		return;
	}

	if (SpawnedBuild->GetClass()->ImplementsInterface(UBuildingInterface::StaticClass()))
	{
		IBuildingInterface::Execute_SetProperties(SpawnedBuild, Buildables[ID]->Mesh, Buildables[ID]->ExtensionMesh, Buildables[ID]->Health, Buildables[ID]->MaxHeightOffset);
	}

	if (Hit && HitComponent && Hit->GetClass()->ImplementsInterface(UBuildingInterface::StaticClass()))
	{
		IBuildingInterface::Execute_HandleBuildWalls(Hit, HitComponent->GetName(), SpawnedBuild);

		if (!Buildables[ID]->AvoidAddingAsChild)
		{
			IBuildingInterface::Execute_AddBuildChild(Hit, SpawnedBuild);
		}
	}
}

void UWOGBuildComponent::HandleBuildHeight(bool bShouldRise)
{
	FVector Increment = FVector();

	if (bShouldRise)
	{
		Increment = HeightOffset + FVector(0.f, 0.f, 100.f);
	}
	else
	{
		Increment = HeightOffset - FVector(0.f, 0.f, 100.f);
	}

	Increment.Z = FMath::Clamp(Increment.Z, -600.f, 600.f);
	HeightOffset = FVector(0.f, 0.f, Increment.Z);
}

void UWOGBuildComponent::HandleBuildRotation(bool bRotateLeft)
{
	if (!bIsBuildModeOn || !BuildGhost->GetStaticMesh()) return;

	FRotator NewBuildRotation = FRotator();

	if (bRotateLeft)
	{
		NewBuildRotation = BuildTransform.GetRotation().Rotator() - FRotator(0.f, 5.f, 0.f);
	}
	else
	{
		NewBuildRotation = BuildTransform.GetRotation().Rotator() + FRotator(0.f, 5.f, 0.f);
	}

	BuildTransform.SetRotation(FQuat::MakeFromRotator(NewBuildRotation));
}

void UWOGBuildComponent::Server_InteractWithBuild_Implementation(UObject* HitActor)
{
	IBuildingInterface* BuildInterface = Cast<IBuildingInterface>(HitActor);
	if (BuildInterface)
	{
		BuildInterface->Execute_InteractWithBuild(HitActor);
	}
}






