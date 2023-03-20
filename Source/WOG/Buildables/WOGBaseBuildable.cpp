// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseBuildable.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"

AWOGBaseBuildable::AWOGBaseBuildable()
{
	bIsDead = false;
	BuildMaxHeightOffset = 0.f;
	BuildHealth = 0.f;
	DestroyDelay = 5.f;

	bReplicates = true;
	SetReplicateMovement(true);
}

void AWOGBaseBuildable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseBuildable, bIsDead);
	DOREPLIFETIME(AWOGBaseBuildable, BuildHealth);
	DOREPLIFETIME(AWOGBaseBuildable, BuildMaxHeightOffset);
	DOREPLIFETIME(AWOGBaseBuildable, BuildChildren);
}

void AWOGBaseBuildable::BeginPlay()
{
	Super::BeginPlay();
}

TArray<UBoxComponent*> AWOGBaseBuildable::ReturnCollisionBoxes_Implementation()
{
	TArray<TObjectPtr<UBoxComponent>> BoxArray;
	GetComponents(BoxArray);
	return BoxArray;
}

void AWOGBaseBuildable::SetProperties_Implementation(UStaticMesh* Mesh, UStaticMesh* ExtensionMesh, const float& Health, const float& MaxHeightOffset)
{
	BuildHealth = Health;
	BuildMaxHeightOffset = MaxHeightOffset;
	GetStaticMeshComponent()->SetStaticMesh(Mesh);
	BuildExtensionMesh = ExtensionMesh;
}

void AWOGBaseBuildable::AddBuildChild_Implementation(AActor* Actor)
{
	BuildChildren.AddUnique(Actor);
}

void AWOGBaseBuildable::DealDamage_Implementation(const float& Damage)
{
	if (bIsDead) return;

	BuildHealth -= Damage;

	if (BuildHealth <= 0)
	{
		bIsDead = true;
		Multicast_DestroyBuild();
	}
}

void AWOGBaseBuildable::Multicast_DestroyBuild_Implementation()
{
	GetStaticMeshComponent()->SetVisibility(false, true);
	GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	for (auto Extension : BuildExtensionsMeshes)
	{
		if (Extension)
		{
			Extension->DestroyComponent();
		}
	}

	for (auto Child : BuildChildren)
	{
		if (Child)
		{
			if (Child->GetClass()->ImplementsInterface(UBuildingInterface::StaticClass()))
			{
				IBuildingInterface::Execute_DealDamage(Child, 9999999.f);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Interface call SetProperties() failed"));
			}
		}
	}

	HandleChaosDestruction();

	TArray<TObjectPtr<UStaticMeshComponent>> ComponentsToDestroy;
	GetComponents(ComponentsToDestroy);

	for (auto Component : ComponentsToDestroy)
	{
		if (Component)
		{
			Component->DestroyComponent();
		}
	}

	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &ThisClass::DestroyBuild, DestroyDelay);
}

void AWOGBaseBuildable::DestroyBuild()
{
	Destroy();
}

bool AWOGBaseBuildable::Trace(const TObjectPtr<UPrimitiveComponent> Component, float& OutDistance)
{
	FHitResult HitResult;
	FVector Start = Component->GetComponentLocation();
	FVector End = Start;
	End.Z = End.Z - (BuildMaxHeightOffset + 200);
	FCollisionQueryParams Params;
	Params.bDebugQuery = true;

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, true);

	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility
	);

	OutDistance = HitResult.Distance;
	return HitResult.bBlockingHit;
}

void AWOGBaseBuildable::BuildExtensions()
{
	TArray<TObjectPtr<UArrowComponent>> Arrows;
	GetComponents(Arrows);

	if (Arrows.Num() == 0 || !BuildExtensionMesh) return;
	
	float Distance = 0.f;

	for (auto Arrow : Arrows)
	{
		float LocalDistance = 0.f;
		bool bHit = Trace(Arrow, LocalDistance);
		if (bHit)
		{
			Distance = LocalDistance > Distance ? LocalDistance : Distance;
		}
	}

	if (Distance < 25.f)
	{
		return;
	}
	int32 NumberOfExtensions = FMath::CeilToInt32(Distance / 200);

	FTransform RelativeTransform = FTransform();

	for (int32 i = 1; i <= NumberOfExtensions; i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("Extension for loop : %d"), i);

		FVector RelativeLocation = FVector(0, 0, (-i * 200));
		RelativeTransform.SetLocation(RelativeLocation);

		TObjectPtr<UStaticMeshComponent> Extension =
			Cast<UStaticMeshComponent>(AddComponentByClass(UStaticMeshComponent::StaticClass(), false, RelativeTransform, false));
		if (Extension)
		{
			BuildExtensionsMeshes.AddUnique(Extension);
			Extension->SetStaticMesh(BuildExtensionMesh);
			UE_LOG(LogTemp, Warning, TEXT("ExtensionBuilt"));
		}
	}
}
