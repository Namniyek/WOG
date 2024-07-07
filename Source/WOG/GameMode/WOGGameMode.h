// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WOGGameMode.generated.h"

class ABasePlayerCharacter;
/**
 * 
 */
UCLASS()
class WOG_API AWOGGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	#pragma region Debug
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDebugMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	int32 LastDefenderIndex = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bHandleDropIn = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug | Character")
	int32 DebugCharacterIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug | Character")
	bool bIsDebugCharacterMale = true;
	#pragma endregion 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AWOGDefender> DefenderCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AWOGAttacker> AttackerCharacter;

	void PlayerEliminated(class AWOGBaseCharacter* ElimmedCharacter, class AWOGPlayerController* VictimController, AWOGPlayerController* AttackerController);
	void PreLogout(APlayerController* InPlayerController);

private:
	void HandleDropIn(APlayerController* NewPlayer);
	void CreateRandomCharacter(APlayerController* NewPlayer);

	UPROPERTY()
	class UWOGGameInstance* GameInstance;

public:

	void HandleStartingPlayer(APlayerController* NewPlayer);
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	FTransform GetPlayerStart(FString StartIndex);

	UFUNCTION(BlueprintCallable)
	void RestartMatch();

	UFUNCTION()
	void RequestRespawn(ABasePlayerCharacter* ElimmedCharacter, APlayerController* ElimmedController);
};
