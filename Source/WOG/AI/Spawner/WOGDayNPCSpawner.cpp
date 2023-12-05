// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Spawner/WOGDayNPCSpawner.h"
#include "Subsystems/WOGWorldSubsystem.h"
#include "Net/UnrealNetwork.h"

AWOGDayNPCSpawner::AWOGDayNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AWOGDayNPCSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGDayNPCSpawner, CurrentTOD);
}

void AWOGDayNPCSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TObjectPtr<UWOGWorldSubsystem> WorldSubsystem = GetWorld()->GetSubsystem<UWOGWorldSubsystem>();
		if (WorldSubsystem)
		{
			CurrentTOD = WorldSubsystem->CurrentTOD;
			WorldSubsystem->TimeOfDayChangedDelegate.AddDynamic(this, &ThisClass::TimeOfDayChanged);
			WorldSubsystem->OnKeyTimeHitDelegate.AddDynamic(this, &ThisClass::KeyTimeHit);
		}
	}
}

void AWOGDayNPCSpawner::TimeOfDayChanged(ETimeOfDay TOD)
{
	CurrentTOD = TOD;
	HandleTODChange();
}

void AWOGDayNPCSpawner::KeyTimeHit(int32 CurrentTime)
{
}

void AWOGDayNPCSpawner::OnRep_CurrentTOD()
{
	HandleTODChange();
}


