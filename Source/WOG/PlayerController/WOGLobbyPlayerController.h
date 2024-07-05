// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WOG/Data/PlayerProfileSaveGame.h"
#include "WOGLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API AWOGLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWOGLobbyPlayerController();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_IsAttacker, BlueprintReadOnly, VisibleAnywhere)
	bool bIsAttacker;

	UPROPERTY(BlueprintReadWrite)
	bool bInEditMode = false;
	
	void OnNetCleanup(UNetConnection* Connection) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	bool bIsPlayerReady;

private:
	UPROPERTY(Replicated)
	class AWOGLobbyAvatar* LobbyAvatar;
	UPROPERTY(Replicated, VisibleAnywhere)
	class AWOGLobbyPlayerSpot* LobbyPlayerSpot;

	FPlayerData PlayerProfile;

	void SaveGame();

	UFUNCTION()
	void OnRep_IsAttacker();
	void SetIsAttacker(bool NewIsAttacker);

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetLobbyAvatar(AWOGLobbyAvatar* NewLobbyAvatar) { LobbyAvatar = NewLobbyAvatar; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGLobbyAvatar* GetLobbyAvatar() { return LobbyAvatar; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetLobbyPlayerSpot(AWOGLobbyPlayerSpot* NewLobbyPlayerSpot) { LobbyPlayerSpot = NewLobbyPlayerSpot; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE AWOGLobbyPlayerSpot* GetLobbyPlayerSpot() { return LobbyPlayerSpot; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsPlayerReady() { return bIsPlayerReady; }

	FORCEINLINE void SetPlayerReady(bool bNewReady) { bIsPlayerReady = bNewReady; }

	UFUNCTION(BlueprintCallable)
	void SetViewTargetCamera(AActor* NewViewTarget);


	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SaveGame();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SetPlayerReady(bool bPlayerReady);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_ServerTravel(const FString& Address);

	UFUNCTION(Server, Reliable)
	void Server_SetIsAttacker(bool NewIsAttacker);
};
