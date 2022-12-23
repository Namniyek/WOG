// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeOfDay.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/Timespan.h"

// Sets default values
ATimeOfDay::ATimeOfDay()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);
	StartingTime = 1060;
	UpdateFrequency = 0.5f;

}

void ATimeOfDay::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATimeOfDay, RepCurrentTime);
}

// Called when the game starts or when spawned
void ATimeOfDay::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		CurrentTime = StartingTime;
		FTimerHandle UpdateTimeHandle;
		GetWorldTimerManager().SetTimer(UpdateTimeHandle, this, &ThisClass::UpdateTime, UpdateFrequency, true);
	}
}

//void ATimeOfDay::OnRep_CurrentTime()
//{
//	ConvertTimeFormat(RepCurrentTime);
//
//	TimeUpdated(RepCurrentTime, CurrentHour, CurrentMinute);
//}

void ATimeOfDay::UpdateTime()
{
	AGameStateBase* GameState = GetWorld()->GetGameState<AGameStateBase>();
	if (!GameState)
	{
		return;
	}

	//float ServerTime =  GameState->GetServerWorldTimeSeconds()/2;
	CurrentTime +=(1);
	
	RepCurrentTime = CurrentTime; 

	if (RepCurrentTime >= 1440)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, FString("NewDay"));
		RepCurrentTime = 0;
		CurrentTime = 0;
	}

	Multicast_SyncCurrentTime(RepCurrentTime);
}

void ATimeOfDay::ConvertTimeFormat(int32 MinutesToConvert)
{
	CurrentHour = FMath::Floor(MinutesToConvert / 60);
	CurrentMinute = MinutesToConvert % 60;
	return;
}

// Called every frame
void ATimeOfDay::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATimeOfDay::Multicast_SyncCurrentTime_Implementation(int32 TimeToSync)
{
	ConvertTimeFormat(RepCurrentTime);

	TimeUpdated(RepCurrentTime, CurrentHour, CurrentMinute);
	GEngine->AddOnScreenDebugMessage(1, 1, FColor::Green, FString("MC called"));
}

