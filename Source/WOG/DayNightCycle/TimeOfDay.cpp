// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeOfDay.h"
#include "WOG.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameState.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/HUD.h"

// Sets default values
ATimeOfDay::ATimeOfDay()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	StartingTime = 1060;
	UpdateFrequency = 0.5f;
	TimeOfDay = ETimeOfDay::TOD_Start;

}

void ATimeOfDay::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATimeOfDay, RepCurrentTime);
	DOREPLIFETIME(ATimeOfDay, TimeOfDay);
	DOREPLIFETIME(ATimeOfDay, CurrentDay);
}

// Called when the game starts or when spawned
void ATimeOfDay::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		CurrentTime = StartingTime;
		UpdateTimeOfDay(CurrentTime);
		GetWorldTimerManager().SetTimer(UpdateTimeHandle, this, &ThisClass::UpdateTime, UpdateFrequency, true);
	}
}

void ATimeOfDay::UpdateTime()
{
	AGameStateBase* GameState = GetWorld()->GetGameState<AGameStateBase>();
	if (!GameState)
	{
		return;
	}

	CurrentTime += 1;

	RepCurrentTime = CurrentTime;

	if (RepCurrentTime >= 1440)
	{
		RepCurrentTime = 0;
		CurrentTime = 0;
	}

	if (RepCurrentTime == 350)
	{
		OnKeyTimeHit.Broadcast(RepCurrentTime);
	}

	if (RepCurrentTime == 360)
	{
		++CurrentDay;
		DayChanged.Broadcast(CurrentDay);
		UpdateTimeOfDay(RepCurrentTime);
		TimeOfDayChanged.Broadcast(TimeOfDay);
	}

	if (RepCurrentTime == 420)
	{
		OnKeyTimeHit.Broadcast(RepCurrentTime);
	}

	if (RepCurrentTime == 1020)
	{
		OnKeyTimeHit.Broadcast(RepCurrentTime);
	}

	if (RepCurrentTime == 1070)
	{
		OnKeyTimeHit.Broadcast(RepCurrentTime);
	}

	if (RepCurrentTime == 1080)
	{
		UpdateTimeOfDay(RepCurrentTime);
		TimeOfDayChanged.Broadcast(TimeOfDay);
	}

	ConvertTimeFormat(RepCurrentTime);
	TimeUpdated(RepCurrentTime, CurrentHour, CurrentMinute);
}

void ATimeOfDay::OnRep_CurrentTime()
{
	ConvertTimeFormat(RepCurrentTime);
	DayChanged.Broadcast(CurrentDay);
	TimeUpdated(RepCurrentTime, CurrentHour, CurrentMinute);
}

void ATimeOfDay::ConvertTimeFormat(int32 MinutesToConvert)
{
	CurrentHour = FMath::Floor(MinutesToConvert / 60);
	CurrentMinute = MinutesToConvert % 60;
	return;
}

void ATimeOfDay::UpdateTimeOfDay(int32 Time)
{
	switch (Time)
	{
		case 360:
			if (CurrentDay == 1) TimeOfDay = ETimeOfDay::TOD_Dawn1;
			if (CurrentDay == 2) TimeOfDay = ETimeOfDay::TOD_Dawn2;
			if (CurrentDay == 3) TimeOfDay = ETimeOfDay::TOD_Dawn3;
			if (CurrentDay == 4) TimeOfDay = ETimeOfDay::TOD_Dawn4;
			break;

		case 1080:
			if (CurrentDay == 1) TimeOfDay = ETimeOfDay::TOD_Dusk1;
			if (CurrentDay == 2) TimeOfDay = ETimeOfDay::TOD_Dusk2;
			if (CurrentDay == 3) TimeOfDay = ETimeOfDay::TOD_Dusk3;
			break;
	}


}

void ATimeOfDay::StopCycle()
{
	GetWorldTimerManager().ClearTimer(UpdateTimeHandle);
}

