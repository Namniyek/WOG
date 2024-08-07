// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "WOG/DayNightCycle/TimeOfDay.h"
#include "WOGGameState.generated.h"

class AWOGPlayerState;
class ABasePlayerCharacter;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurrentTargetScoreChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerInitCompleteDelegate, const AWOGPlayerState*, InitPlayerState, const ABasePlayerCharacter*, InitPlayerCharacter);

UCLASS()
class WOG_API AWOGGameState : public AGameState
{
	GENERATED_BODY()

public:
	AWOGGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void HandleTODAnnouncement(ETimeOfDay TOD);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_HandleEndGame();

	UPROPERTY(EditDefaultsOnly, Category = Setup)
	TSubclassOf<class AActor> TimeOfDayClass;

	UPROPERTY(EditDefaultsOnly, Category = Setup)
	ETimeOfDay FinishMatchTOD = ETimeOfDay::TOD_Dawn4;

	UPROPERTY(EditDefaultsOnly, Category = Setup)
	float EndGameDelay;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerInitCompleteDelegate OnPlayerInitComplete;

protected:
	virtual void HandleMatchHasStarted() override;

	UFUNCTION()
	void TimeOfDayChanged(ETimeOfDay TOD);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 TotalTargetScore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 CurrentTargetScore;

	UPROPERTY(BlueprintAssignable)
	FOnCurrentTargetScoreChanged OnCurrentTargetScoreChangedDelegate;

private:
	void SetupTOD();
	int32 FinishMatchDayNumber = 4;

	void InitTargetScores();

	UFUNCTION()
	void DayChanged(int32 DayNumber);

	UPROPERTY()
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

	void SubtractFromCurrentTargetScore(const int32& ScoreToSubtract);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_UpdateCurrentTargetScore();

	UFUNCTION(BlueprintPure)
	float GetTargetScorePercentage();

};
