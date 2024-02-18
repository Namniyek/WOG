// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseBuildable.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"
#include "TargetSystemComponent.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "WOG.h"

AWOGBaseBuildable::AWOGBaseBuildable()
{
	bIsDead = false;
	BuildMaxHeightOffset = 0.f;
	BuildHealth = 0.f;
	DestroyDelay = 5.f;

	bReplicates = true;
	SetReplicateMovement(true);

	TargetWidgetLocation = CreateDefaultSubobject<USceneComponent>(TEXT("TargetWidgetLocation"));
	TargetWidgetLocation->SetupAttachment(GetRootComponent());
}

void AWOGBaseBuildable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseBuildable, bIsDead);
	DOREPLIFETIME(AWOGBaseBuildable, BuildHealth);
	DOREPLIFETIME(AWOGBaseBuildable, BuildMaxHealth);
	DOREPLIFETIME(AWOGBaseBuildable, BuildMaxHeightOffset);
	DOREPLIFETIME(AWOGBaseBuildable, BuildChildren);
	DOREPLIFETIME(AWOGBaseBuildable, CurrentMeleeSquad);
	DOREPLIFETIME(AWOGBaseBuildable, CurrentRangedSquad);

}

void AWOGBaseBuildable::BeginPlay()
{
	Super::BeginPlay();
}

void AWOGBaseBuildable::OnRep_BuildHealth()
{
	if (BuildHealth <= 0)
	{
		HandleHealthBar(false);
	}
	else
	{
		HandleHealthBar(true);
	}
}

void AWOGBaseBuildable::HandleHealthBar(bool NewVisible)
{
	GetWorldTimerManager().ClearTimer(HealthBarTimerHandle);

	if (NewVisible)
	{
		ShowHealthBar();
		GetWorldTimerManager().SetTimer(HealthBarTimerHandle, this, &ThisClass::HideHealthBar, 3.f);
	}
	else
	{
		HideHealthBar();
	}
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
	BuildMaxHealth = Health;
	BuildMaxHeightOffset = MaxHeightOffset;
	GetStaticMeshComponent()->SetStaticMesh(Mesh);
	BuildExtensionMesh = ExtensionMesh;
}

void AWOGBaseBuildable::AddBuildChild_Implementation(AActor* Actor)
{
	BuildChildren.AddUnique(Actor);
}

void AWOGBaseBuildable::DealDamage_Implementation(const float& Damage, const AActor* Agressor)
{
	HandleDamage(Damage, Agressor);
}

void AWOGBaseBuildable::ReturnBuildHealth_Implementation(float& OutBuildHealth, float& OutMaxBuildHealth)
{
	OutBuildHealth = BuildHealth;
	OutMaxBuildHealth = BuildMaxHealth;
}

void AWOGBaseBuildable::Multicast_DestroyBuild_Implementation(const AActor* Agressor)
{
	if (Agressor)
	{
		UTargetSystemComponent* TargetComp = Agressor->GetComponentByClass<UTargetSystemComponent>();
		if (TargetComp)
		{
			TargetComp->TargetLockOff();
		}
	}

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
				IBuildingInterface::Execute_DealDamage(Child, 9999999.f, nullptr);
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
		if (Component && Component != GetRootComponent())
		{
			Component->DestroyComponent();
		}
	}
}

void AWOGBaseBuildable::DestroyBuild()
{
	Destroy();
}

void AWOGBaseBuildable::SetCurrentRangedSquad(AWOGBaseSquad* NewSquad)
{
	if (HasAuthority())
	{
		CurrentRangedSquad = NewSquad;
	}
}

void AWOGBaseBuildable::SetCurrentMeleeSquad(AWOGBaseSquad* NewSquad)
{
	if (HasAuthority())
	{
		CurrentMeleeSquad = NewSquad;
	}
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

void AWOGBaseBuildable::HandleDamage(const float& Damage, const AActor* Agressor)
{
	if (bIsDead || !HasAuthority()) return;

	if (BuildHealth - Damage <= 0)
	{
		BuildHealth = 0;
		bIsDead = true;
		HandleHealthBar(false);
		OnTargetDestroyedDelegate.Broadcast(nullptr);

		FTimerHandle DestroyTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &ThisClass::DestroyBuild, DestroyDelay);

		Multicast_DestroyBuild(Agressor);
	}
	else
	{
		BuildHealth = BuildHealth - Damage;
		HandleHealthBar(true);
	}
}

void AWOGBaseBuildable::Server_BuildExtensions_Implementation()
{
	BuildExtensions();
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
			Extension->SetIsReplicated(true);
			UE_LOG(LogTemp, Warning, TEXT("ExtensionBuilt"));
		}
	}
}

bool AWOGBaseBuildable::IsTargetable_Implementation(AActor* TargeterActor) const
{
	//For targets, check if the targeter is attacker and then return true if that is the case.
	//We only want attackers to be able to target WOGBaseBuildables
	bool bIsTargeterAttacker = UWOGBlueprintLibrary::GetCharacterData(TargeterActor).bIsAttacker;
	return bIsTargeterAttacker;
}

void AWOGBaseBuildable::GetTargetWidgetAttachmentParent_Implementation(USceneComponent*& OutParentComponent, FName& OutSocketName) const
{
	OutParentComponent = TargetWidgetLocation;
	OutSocketName = FName("");
}

FVector AWOGBaseBuildable::GetMeleeAttackSlot_Implementation(const int32& SlotIndex) const
{
	if (SlotIndex < MeleeSlots.Num())
	{
		return MeleeSlots[SlotIndex].Vector;
	}
	else
	{
		UE_LOG(WOGLogCombat, Error, TEXT("No corresponding MeleeAttackSlot on %s that corresponds to the provided index %d"), *GetNameSafe(this), SlotIndex);
		return FVector();
	}
}

FVector AWOGBaseBuildable::GetRangedAttackSlot_Implementation(const int32& SlotIndex) const
{
	if (SlotIndex < RangedSlots.Num())
	{
		return RangedSlots[SlotIndex].Vector;
	}
	else
	{
		UE_LOG(WOGLogCombat, Error, TEXT("No corresponding RangedAttackSlot on %s that corresponds to the provided index %d"), *GetNameSafe(this), SlotIndex);
		return FVector();
	}
}

bool AWOGBaseBuildable::IsCurrentMeleeSquadSlotAvailable_Implementation() const
{
	return CurrentMeleeSquad == nullptr;
}

bool AWOGBaseBuildable::IsCurrentRangedSquadSlotAvailable_Implementation() const
{
	return CurrentRangedSquad == nullptr;
}

void AWOGBaseBuildable::FreeCurrentRangedSquadSlot_Implementation()
{
	SetCurrentRangedSquad(nullptr);
}

void AWOGBaseBuildable::FreeCurrentMeleeSquadSlot_Implementation()
{
	SetCurrentMeleeSquad(nullptr);
}

void AWOGBaseBuildable::SetCurrentRangedSquadSlot_Implementation(AWOGBaseSquad* NewSquad)
{
	SetCurrentRangedSquad(NewSquad);
}

void AWOGBaseBuildable::SetCurrentMeleeSquadSlot_Implementation(AWOGBaseSquad* NewSquad)
{
	SetCurrentMeleeSquad(NewSquad);
}
