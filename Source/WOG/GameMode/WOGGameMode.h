// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WOGGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDebugMode = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHandleDropIn = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AWOGDefender> DefenderCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AWOGAttacker> AttackerCharacter;

	void PlayerEliminated(class AWOGBaseCharacter* ElimmedCharacter, class AWOGPlayerController* VictimController, AWOGPlayerController* AttackerController);

protected:


private:
	void HandleDropIn(APlayerController* NewPlayer);
	void CreateRandomCharacter(APlayerController* NewPlayer);

	class UWOGGameInstance* GameInstance;

	FTransform GetPlayerStart(FString StartIndex);

public:

	void HandleStartingPlayer(APlayerController* NewPlayer);
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable)
	void RestartMatch();

	UFUNCTION()
	void RequestRespawn(ABasePlayerCharacter* ElimmedCharacter, APlayerController* ElimmedController);
};
