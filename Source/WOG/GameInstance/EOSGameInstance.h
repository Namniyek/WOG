// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSGameInstance.generated.h"


USTRUCT(BlueprintType)
struct FServerItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FString HostName;
	UPROPERTY(BlueprintReadOnly)
		FString CurrentPlayers;
	UPROPERTY()
		int32 NumCurrentPlayers;
	UPROPERTY()
		int32 NumMaxPlayers;
	UPROPERTY(BlueprintReadOnly)
		int32 ServerArrayIndex;

	void SetCurrentPlayers()
	{
		CurrentPlayers = FString(FString::FromInt(NumCurrentPlayers) + " / " + FString::FromInt(NumMaxPlayers));
	}

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerFoundDelegate, FServerItem, ServerInfo);

/**
 * 
 */
UCLASS()
class WOG_API UEOSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UEOSGameInstance();

	virtual void Init() override;

	void Login();
	void OnLoginComplete(int ControllerIndex, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorString);

	UFUNCTION(BlueprintCallable)
	void CreateSession();
	void OnCreateSessionComplete(FName SessionName, bool bSuccessful);
	
	UFUNCTION(BlueprintCallable)
	void DestroySession();
	void OnDestroySessionComplete(FName SessionName, bool bSuccessful);


	UFUNCTION(BlueprintCallable)
	void GetAllFriends();
	void OnGetAllFriendsComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
	
	UFUNCTION(BlueprintCallable)
	void ShowInviteUI();

	UFUNCTION(BlueprintCallable)
	void ShowFriendsUI();

	TSharedPtr <class FOnlineSessionSearch> SearchSettings;

	UFUNCTION(BlueprintCallable)
	void FindSessions();
	void OnFindSessionsComplete(bool bWasSuccessful);

	UFUNCTION(BlueprintCallable)
	void JoinServer(int32 ServerIndex);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

protected:
	class IOnlineSubsystem* OnlineSubsystem;

	bool bIsLoggedIn;

	UPROPERTY(BlueprintAssignable)
	FServerFoundDelegate ServerFoundDelegate;
	
};
