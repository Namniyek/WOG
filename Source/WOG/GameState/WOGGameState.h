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
	void HandleTODAnnouncement(ETimeOfDay TOD);
	void HandleEndGame();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AActor> TimeOfDayClass;

	UPROPERTY(EditDefaultsOnly)
	ETimeOfDay FinishMatchTOD = ETimeOfDay::TOD_Dawn4;

protected:
	virtual void HandleMatchHasStarted() override;

private:
	void SetupTOD();
	int32 FinishMatchDayNumber = 4;

	UFUNCTION()
	void TimeOfDayChanged(ETimeOfDay TOD);

	UFUNCTION()
	void DayChanged(int32 DayNumber);

	class ATimeOfDay* TODActor;

	bool bAttackersWon = false;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsWinnerAttacker() { return bAttackersWon; }

};
