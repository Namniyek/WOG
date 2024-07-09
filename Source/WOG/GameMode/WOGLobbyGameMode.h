// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WOGLobbyGameMode.generated.h"

/**
 * 
 */
class AWOGLobbyPlayerSpot;
class AWOGLobbyPlayerController;
class AWOGLobbyAvatar;

UCLASS()
class WOG_API AWOGLobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	AWOGLobbyGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	UFUNCTION()
	void RegisterPlayer(APlayerController* NewPlayer);

	virtual void Logout(AController* Exiting) override;
	void PreLogout(APlayerController *InPlayerController);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray <AWOGLobbyPlayerController*> AllLobbyPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray <AWOGLobbyPlayerSpot*> DefenderLobbySpots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray <AWOGLobbyPlayerSpot*> AttackerLobbySpots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 PlayersReady;

private:
	bool bAllExistingPlayersRegistered;

	TArray <AWOGLobbyPlayerSpot*> AllLobbySpots;

	void SetupLobbyCharacter(int32 PlayerIndex);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AWOGLobbyAvatar> AvatarBP;


	
public:

	bool GetAttackerPlayerSpot(AWOGLobbyPlayerSpot* &OutPlayerSpot);
	bool GetDefenderPlayerSpot(AWOGLobbyPlayerSpot* &OutPlayerSpot);
	bool GetNextPlayerSpot(AWOGLobbyPlayerSpot* &OutPlayerSpot, int32 &OutTeamIndex);

	//Implemented in BP
	UFUNCTION(BlueprintImplementableEvent)
	void IncreasePlayerReady();
	UFUNCTION(BlueprintImplementableEvent)
	void DecreasePlayerReady();

	void RegisterExistingPlayers();

};
