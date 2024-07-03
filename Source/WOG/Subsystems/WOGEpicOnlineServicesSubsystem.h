// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WOGEpicOnlineServicesSubsystem.generated.h"

/**
 * 
 */
 DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginProcessCompleteDelegate, bool, bWasSuccessful);
 
UCLASS()
class WOG_API UWOGEpicOnlineServicesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable)
	FOnLoginProcessCompleteDelegate OnLoginCompleteDelegate;

protected:
	UFUNCTION(BlueprintCallable)
	void Login();

	UFUNCTION(BlueprintCallable)
	void CreateLobby();

	UFUNCTION(BlueprintCallable)
	void GetLobbyMembers();

private:
	
	#pragma region Callback functions
	void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error);
	#pragma endregion

	#pragma region Delegate handles
	FDelegateHandle LoginDelegateHandle;
	#pragma endregion

	#pragma region Created Lobby
	FString LobbyIdString;
	#pragma endregion 

public:

	UFUNCTION(BlueprintPure)
	bool IsLocalUserLoggedIn() const;
};
