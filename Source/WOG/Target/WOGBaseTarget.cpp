// Fill out your copyright notice in the Description page of Project Settings.


#include "Target/WOGBaseTarget.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Net/UnrealNetwork.h"
#include "WOG.h"
#include "AI/Spawner/WOGDayNPCSpawner.h"
#include "Subsystems/WOGWorldSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameState/WOGGameState.h"

AWOGBaseTarget::AWOGBaseTarget()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GeometryCollectionComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComp"));
	SetRootComponent(GeometryCollectionComp);
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	RootMesh->SetupAttachment(GetRootComponent());
	RootMesh->SetIsReplicated(true);

	MaxHealth = 100.f;
	Health = MaxHealth;

	DestroyDelay = 3.f;

	TargetScore = 5;
}

void AWOGBaseTarget::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWOGBaseTarget, Health)
}

void AWOGBaseTarget::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		TObjectPtr<UWOGWorldSubsystem> WorldSubsystem = GetWorld()->GetSubsystem<UWOGWorldSubsystem>();
		if (WorldSubsystem)
		{
			WorldSubsystem->TimeOfDayChangedDelegate.AddDynamic(this, &ThisClass::TimeOfDayChanged);
			WorldSubsystem->OnKeyTimeHitDelegate.AddDynamic(this, &ThisClass::KeyTimeHit);
		}
	}
}

void AWOGBaseTarget::UpdateRestCollection(const UGeometryCollection* RestCollectionIn)
{
	GeometryCollectionComp->SetRestCollection(RestCollectionIn);
}

void AWOGBaseTarget::DealDamage_Implementation(const float& Damage)
{
	HandleDamage(Damage);
}

void AWOGBaseTarget::HandleDamage(const float& Damage)
{
	if (!HasAuthority()) return;

	if (Health - Damage <= 0)
	{
		Health = 0;
		HandleDestruction();
		HandleHealthBar(false);
	}
	else
	{
		Health = Health - Damage;
		HandleHealthBar(true);
	}
}

void AWOGBaseTarget::HandleHealthBar(bool NewVisible)
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

void AWOGBaseTarget::OnRep_Health()
{
	if (Health <= 0)
	{
		HandleHealthBar(false);
		HandleDestruction();
		HandleChaosDestruction();
	}
	else
	{
		HandleHealthBar(true);
	}
}

void AWOGBaseTarget::HandleDestruction()
{
	if (!HasAuthority()) return;
	if (!RootMesh) return;

	TArray<USceneComponent*> OutChildren;
	RootMesh->GetChildrenComponents(true, OutChildren);

	if (!OutChildren.IsEmpty())
	{
		for (auto Child : OutChildren)
		{
			if (!Child) continue;

			Child->DestroyComponent();
		}
	}

	if (RootMesh)
	{
		RootMesh->DestroyComponent();
	}

	if (ConnectedSpawner)
	{
		ConnectedSpawner->SetIsSpawnerActive(false);
	}

	HandleChaosDestruction();
	BroadcastDestructionToGameState();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::DestroyTarget, DestroyDelay);
}

void AWOGBaseTarget::DestroyTarget()
{
	Destroy();
}

void AWOGBaseTarget::TimeOfDayChanged(ETimeOfDay TOD)
{
	switch (TOD)
	{
	case ETimeOfDay::TOD_Dawn1:
		Multicast_HandleDoorOpening(true);
		break;
	case ETimeOfDay::TOD_Dusk1:
		Multicast_HandleDoorOpening(false);
		break;
	case ETimeOfDay::TOD_Dawn2:
		Multicast_HandleDoorOpening(true);
		break;
	case ETimeOfDay::TOD_Dusk2:
		Multicast_HandleDoorOpening(false);
		break;
	case ETimeOfDay::TOD_Dawn3:
		Multicast_HandleDoorOpening(true);
		break;
	case ETimeOfDay::TOD_Dusk3:
		Multicast_HandleDoorOpening(false);
		break;
	}
}

void AWOGBaseTarget::KeyTimeHit(int32 CurrentTime)
{
	switch (CurrentTime)
	{
	case 420:
		Multicast_HandleDoorOpening(false);
		break;
	case 1020:
		Multicast_HandleDoorOpening(true);
		break;
	}
}

void AWOGBaseTarget::BroadcastDestructionToGameState()
{
	if (!HasAuthority()) return;

	TObjectPtr<AWOGGameState> GameState = Cast<AWOGGameState>(UGameplayStatics::GetGameState(this));
	if (!GameState)
	{
		UE_LOG(WOGLogWorld, Error, TEXT("Invalid GameState from destruction of %s"), *GetNameSafe(this));
		return;
	}

	GameState->SubtractFromCurrentTargetScore(TargetScore);
}

void AWOGBaseTarget::Multicast_HandleDoorOpening_Implementation(bool bIsOpen)
{
	HandleDoorOpening(bIsOpen);
}

