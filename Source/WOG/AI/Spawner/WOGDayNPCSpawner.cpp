// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Spawner/WOGDayNPCSpawner.h"
#include "Subsystems/WOGWorldSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "WOG.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/WOGBaseDayNPC.h"

AWOGDayNPCSpawner::AWOGDayNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsSpawnerActive = true;

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

void AWOGDayNPCSpawner::SetSpawnerActive_Implementation(bool NewActive)
{
	if (!HasAuthority()) return;
	bIsSpawnerActive = NewActive;
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

void AWOGDayNPCSpawner::HandleTODChange()
{
	switch (CurrentTOD)
	{
	case ETimeOfDay::TOD_Dawn1:
		StartSpawn();
		break;
	case ETimeOfDay::TOD_Dawn2:
		StartSpawn();
		break;
	case ETimeOfDay::TOD_Dawn3:
		StartSpawn();
		break;
	default:
		break;
	}
}

bool AWOGDayNPCSpawner::StartSpawn()
{
	if(!bIsSpawnerActive)
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("%s spawner not active anymore"), *GetNameSafe(this));
		return false;
	}

	if (SpawnMap.IsEmpty())
	{
		UE_LOG(WOGLogSpawn, Error, TEXT("%s spawner SpawnMap is empty. Cannot StartSpawn()"), *GetNameSafe(this));
		return false;
	}

	int32 CurrentAvailableLocationIndex = SpawnLocations.Num()-1;

	for (auto SpawnPair : SpawnMap)
	{
		if (!UKismetSystemLibrary::IsValidClass(SpawnPair.Key))
		{
			UE_LOG(WOGLogSpawn, Error, TEXT("Attempted to access invalid class from SpawnMap"), *GetNameSafe(this));
			continue;
		}

		for (int32 i = SpawnPair.Value; i > 0; i--)
		{
			FVector NextLocation = SpawnLocations[CurrentAvailableLocationIndex].Vector;
			FVector NextWorldLocation = NextLocation + GetActorLocation();
			SpawnNPC(SpawnPair.Key, NextWorldLocation);

			CurrentAvailableLocationIndex--;
		}
	}
	
	return true;
}

void AWOGDayNPCSpawner::SpawnNPC(const TSubclassOf<AActor>& ClassToSpawn, const FVector& Location)
{
	FTimerDelegate TimerDel;
	FTimerHandle TimerHandle;
	//Setup random spawn delay time
	float TimerDelay = FMath::RandRange(0.1f, 3.f);

	//Binding the function with specific values
	TimerDel.BindUFunction(this, FName("DelayedSpawnNPC"), ClassToSpawn, Location);
	//Calling DelayedSpawnNPC() after the TimerDelay without looping
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, TimerDelay, false);

}

void AWOGDayNPCSpawner::DelayedSpawnNPC(const TSubclassOf<AActor>& ClassToSpawn, const FVector& Location)
{
	UE_LOG(WOGLogSpawn, Display, TEXT("Attempted to DelayedSpawn %s at %s"), *GetNameSafe(ClassToSpawn), *Location.ToString());
	if (!HasAuthority()) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	FVector NewLocation = Location;
	NewLocation.Z = NewLocation.Z + 90.f;

	GetWorld()->SpawnActor<AActor>(ClassToSpawn, NewLocation, FRotator(), SpawnParams);
}

#if WITH_EDITOR
void AWOGDayNPCSpawner::PostEditChangeProperty(FPropertyChangedEvent& PostEditChangeProperty)
{
	Super::PostEditChangeProperty(PostEditChangeProperty);

	if (PostEditChangeProperty.GetPropertyName() == FName("SpawnMap"))
	{
		int32 CurrentLocationsAmount = SpawnLocations.Num();
		int32 CurrentSpawnsAmount = 0;

		if (SpawnMap.IsEmpty())
		{
			UE_LOG(WOGLogSpawn, Error, TEXT("%s spawner SpawnMap is empty."), *GetNameSafe(this));
			SpawnLocations.Empty();
			return;
		}

		for (auto SpawnPair : SpawnMap)
		{
			if (!UKismetSystemLibrary::IsValidClass(SpawnPair.Key))
			{
				UE_LOG(WOGLogSpawn, Error, TEXT("Attempted to access invalid class from SpawnMap"), *GetNameSafe(this));
				continue;
			}

			for (int32 i = SpawnPair.Value; i > 0; i--)
			{
				CurrentSpawnsAmount++;
			}
		}

		UE_LOG(WOGLogSpawn, Display, TEXT("CurrentSpawnsAmount: %d"), CurrentSpawnsAmount);

		//No valid spawns, clear locations array
		if (CurrentSpawnsAmount == 0)
		{
			SpawnLocations.Empty();
		}

		//Amounts are equal, nothing to do here
		if (CurrentLocationsAmount == CurrentSpawnsAmount) return;

		//if more spawns than locations, add new locations, not touching the old ones
		if (CurrentSpawnsAmount > CurrentLocationsAmount)
		{
			for (int32 i = 0; i <= CurrentSpawnsAmount; i++)
			{
				if (i > CurrentLocationsAmount)
				{
					SpawnLocations.Add(FVector3DWithWidget());
					CurrentLocationsAmount = SpawnLocations.Num();
				}
			}
			return;
		}

		//if less spawns than location, reverse for loop to remove the extra locations
		if (CurrentSpawnsAmount < CurrentLocationsAmount)
		{
			UE_LOG(WOGLogSpawn, Display, TEXT("Something happened"));
			for (int32 i = CurrentLocationsAmount; i > CurrentSpawnsAmount; i--)
			{
				SpawnLocations.RemoveAt(i - 1);
				CurrentLocationsAmount = SpawnLocations.Num();
			}
			return;
		}
	}
}
#endif


