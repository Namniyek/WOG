// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "WOG/DayNightCycle/TimeOfDay.h"
#include "WOGGameState.generated.h"


/**
 * 
 */
UCLASS()
class WOG_API AWOGGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void HandleTODAnnouncement(ETimeOfDay TOD);
	UFUNCTION(Server, Reliable)
	void Server_HandleEndGame();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AActor> TimeOfDayClass;

	UPROPERTY(EditDefaultsOnly)
	ETimeOfDay FinishMatchTOD = ETimeOfDay::TOD_Dawn4;

protected:
	virtual void HandleMatchHasStarted() override;

	UFUNCTION()
	void TimeOfDayChanged(ETimeOfDay TOD);

private:
	void SetupTOD();
	int32 FinishMatchDayNumber = 4;

	UFUNCTION()
	void DayChanged(int32 DayNumber);

	class ATimeOfDay* TODActor;

	UPROPERTY(Replicated)
	bool bAttackersWon = false;

	UPROPERTY(Replicated)
	FString MostElimmedPlayer = FString();
	UPROPERTY(Replicated)
	FString PlayerWithMostElimms = FString();
	int32 MostElimms = 0;
	int32 MostElimmed = 0;

	FTransform GetPlayerStartTransform(APlayerController* PlayerController);

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsWinnerAttacker() { return bAttackersWon; }

	UFUNCTION(Server, Reliable)
	void Server_SetEndgamePlayerStats();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetEndgamePlayerStats();
	void SetEndgamePlayerStats();

};
