// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WOGBaseDayNPC.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/WOGWorldSubsystem.h"
#include "WOG.h"

AWOGBaseDayNPC::AWOGBaseDayNPC()
{
	CurrentState = EDayNPCState::EDNS_MAX;
	PreviousSlot = nullptr;
	SpawnLocation = FVector();
	CurrentSlotIndex = 0;
}

void AWOGBaseDayNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWOGBaseDayNPC, CurrentState);
}

void AWOGBaseDayNPC::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<UWOGWorldSubsystem> WorldSubsystem = GetWorld()->GetSubsystem<UWOGWorldSubsystem>();
	if (WorldSubsystem && HasAuthority())
	{
		WorldSubsystem->OnKeyTimeHitDelegate.AddDynamic(this, &ThisClass::KeyTimeHit);
		UE_LOG(WOGLogSpawn, Display, TEXT("OnKeyTimeHit bound for villager NPC"));
	}

	GiveDefaultAbilities();

	InitCurrentState();
}

void AWOGBaseDayNPC::InitCurrentState()
{
	if (!HasAuthority()) return;

	switch (CurrentTOD)
	{
	case ETimeOfDay::TOD_Dawn1:
		SetCurrentState(EDayNPCState::EDNS_Working);
		break;
	case ETimeOfDay::TOD_Dawn2:
		SetCurrentState(EDayNPCState::EDNS_Working);
		break;
	case ETimeOfDay::TOD_Dawn3:
		SetCurrentState(EDayNPCState::EDNS_Working);
		break;
	default:
		break;
	}

	SpawnLocation = GetActorLocation();
}

void AWOGBaseDayNPC::HandleTODChange()
{
	if (!HasAuthority()) return;

	switch (CurrentTOD)
	{
	case ETimeOfDay::TOD_Dawn1:
		SetCurrentState(EDayNPCState::EDNS_Working);
		break;
	case ETimeOfDay::TOD_Dusk1:
		Destroy();
		break;
	case ETimeOfDay::TOD_Dawn2:
		SetCurrentState(EDayNPCState::EDNS_Working);
		break;
	case ETimeOfDay::TOD_Dusk2:
		Destroy();
		break;
	case ETimeOfDay::TOD_Dawn3:
		SetCurrentState(EDayNPCState::EDNS_Working);
		break;
	case ETimeOfDay::TOD_Dusk3:
		Destroy();
		break;
	default:
		break;
	}
}

void AWOGBaseDayNPC::KeyTimeHit(int32 CurrentTime)
{
	if (!HasAuthority()) return;

	if (CurrentTime == 1020)
	{
		SetCurrentState(EDayNPCState::EDNS_Return);
		UE_LOG(WOGLogSpawn, Display, TEXT("Current time == 17:00 -> Time to go home"));
	}
}

void AWOGBaseDayNPC::SetCurrentState(EDayNPCState NewState)
{
	CurrentState = NewState;
	HandleStateChange();
}

FGameplayTag AWOGBaseDayNPC::GetNextActivitySlotTag()
{
	if (ActivitySlotsTags.IsEmpty()) return FGameplayTag();

	TArray<FGameplayTag> Array = ActivitySlotsTags.GetGameplayTagArray();
	
	//Shuffles array
	if (Array.Num() > 0)
	{
		const int32 LastIndex = Array.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			const int32 Index = FMath::RandRange(i, LastIndex);
			if (i != Index)
			{
				Array.Swap(i, Index);
			}
		}
	}

	return Array[CurrentSlotIndex];
}

void AWOGBaseDayNPC::IncrementCurrentSlotIndex()
{
	if (CurrentSlotIndex + 1 > ActivitySlotsTags.Num() - 1)
	{
		CurrentSlotIndex = 0;
	}
	else
	{
		CurrentSlotIndex++;
	}
}
