// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeOfDay.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameState.h"
#include "Misc/Timespan.h"

// Sets default values
ATimeOfDay::ATimeOfDay()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);


}

void ATimeOfDay::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATimeOfDay, CurrentTime);
}

// Called when the game starts or when spawned
void ATimeOfDay::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		FTimerHandle UpdateTimeHandle;
		GetWorldTimerManager().SetTimer(UpdateTimeHandle, this, &ThisClass::UpdateTime, 0.5f, true);
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString("Event Bound"));
	}
}

void ATimeOfDay::OnRep_CurrentTime()
{
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString::FromInt(CurrentTime));
	int32 Minute = CurrentTime % 60;
	int32 Hour = (CurrentTime/60) % 24;
	FString CurrentTimeStr = FString::FromInt(Hour) + FString(" : ") + FString::FromInt(Minute);
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString(CurrentTimeStr));

}

void ATimeOfDay::UpdateTime()
{
	AGameStateBase* GameState = GetWorld()->GetGameState<AGameStateBase>();
	if (!GameState)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString("NoGameState"));
		return;
	}
	float ServerTime =  GameState->GetServerWorldTimeSeconds();
	CurrentTime = StartingTime + (ServerTime * 2.f);
	if (CurrentTime >= 2400) CurrentTime = 0;

}

// Called every frame
void ATimeOfDay::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

