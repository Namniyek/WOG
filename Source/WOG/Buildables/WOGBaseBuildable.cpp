// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGBaseBuildable.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"
#include "TargetSystemComponent.h"
#include "Libraries/WOGBlueprintLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "WOG.h"
#include "Data/DataAssets/WOGSpawnCosmetics.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"

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

	CosmeticsDataAsset = nullptr;
}

void AWOGBaseBuildable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseBuildable, bIsDead);
	DOREPLIFETIME(AWOGBaseBuildable, BuildHealth);
	DOREPLIFETIME(AWOGBaseBuildable, BuildMaxHealth);
	DOREPLIFETIME(AWOGBaseBuildable, BuildMaxHeightOffset);
	DOREPLIFETIME(AWOGBaseBuildable, CurrentMeleeSquad);
	DOREPLIFETIME(AWOGBaseBuildable, CurrentRangedSquad);
	DOREPLIFETIME(AWOGBaseBuildable, CurrentEpicSquad);
	DOREPLIFETIME(AWOGBaseBuildable, CosmeticsDataAsset);
}

void AWOGBaseBuildable::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle CosmeticTimer;
	GetWorldTimerManager().SetTimer(CosmeticTimer, this, &ThisClass::HandleSpawnCosmetics, 0.1f);
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

	HandleDestroyCosmetics();

	GetStaticMeshComponent()->SetVisibility(false, true);
	GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	for (auto Extension : BuildExtensionsMeshes)
	{
		if (Extension)
		{
			Extension->DestroyComponent();
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

void AWOGBaseBuildable::HandleSpawnCosmetics()
{
	if (CosmeticsDataAsset && CosmeticsDataAsset->SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CosmeticsDataAsset->SpawnSound, GetActorLocation());
	}
	if (CosmeticsDataAsset && CosmeticsDataAsset->SpawnParticleSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, CosmeticsDataAsset->SpawnParticleSystem, GetActorLocation(), GetActorRotation());
	}
}

void AWOGBaseBuildable::HandleDestroyCosmetics()
{
	if (CosmeticsDataAsset->DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CosmeticsDataAsset->DestroySound, GetActorLocation());
	}
	if (CosmeticsDataAsset->DestroyParticleSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, CosmeticsDataAsset->DestroyParticleSystem, GetActorLocation(), GetActorRotation());
	}
}

void AWOGBaseBuildable::ExecuteGameplayCueWithCosmeticsDataAsset(const FGameplayTag& CueTag)
{
	if (!HasAuthority() || !GetOwner())
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (ASC)
	{
		FGameplayCueParameters CueParams;
		CueParams.SourceObject = this;
		ASC->ExecuteGameplayCue(CueTag, CueParams);
	}
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

void AWOGBaseBuildable::SetCurrentEpicSquad(AWOGBaseSquad* NewSquad)
{
	if (HasAuthority())
	{
		CurrentEpicSquad = NewSquad;
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
		DestroyChildren();
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
			Extension->SetCollisionResponseToChannel(ECC_DamageTrace, ECollisionResponse::ECR_Block);
			UE_LOG(LogTemp, Warning, TEXT("ExtensionBuilt"));
		}
	}
}

void AWOGBaseBuildable::DestroyChildren()
{
	if (ChildrenBuilds.IsEmpty()) return;
	if (!HasAuthority()) return;

	TArray<AActor*> OutItems;
	ChildrenBuilds.GetKeys(OutItems);
	if (OutItems.IsEmpty()) return;

	for (auto Child : OutItems)
	{
		if (!Child) continue;

		if (Child->Implements<UBuildingInterface>())
		{
			IBuildingInterface::Execute_DealDamage(Child, 9999999.f, nullptr);
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

AWOGBaseSquad* AWOGBaseBuildable::GetCurrentRangedSquadSlot_Implementation() const
{
	return CurrentRangedSquad;
}

AWOGBaseSquad* AWOGBaseBuildable::GetCurrentMeleeSquadSlot_Implementation() const
{
	return CurrentMeleeSquad;
}

void AWOGBaseBuildable::SetCurrentEpicSquadSlot_Implementation(AWOGBaseSquad* NewSquad)
{
	SetCurrentEpicSquad(NewSquad);
}

void AWOGBaseBuildable::FreeCurrentEpicSquadSlot_Implementation()
{
	SetCurrentEpicSquad(nullptr);
}

bool AWOGBaseBuildable::IsCurrentEpicSquadSlotAvailable_Implementation() const
{
	return CurrentEpicSquad == nullptr;
}

AWOGBaseSquad* AWOGBaseBuildable::GetCurrentEpicSquadSlot_Implementation() const
{
	return CurrentEpicSquad;
}
