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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHandleDropIn = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class ABasePlayerCharacter> DefenderCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ABasePlayerCharacter> AttackerCharacter;

protected:

	virtual void BeginPlay() override;

private:
	void HandleStartingPlayer(APlayerController* NewPlayer);
	void HandleDropIn(APlayerController* NewPlayer);
	void CreateRandomCharacter(APlayerController* NewPlayer);

	class UWOGGameInstance* GameInstance;

	FTransform GetPlayerStart(FString StartIndex);


public:

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};
