// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "WOGGameInstance.generated.h"

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

USTRUCT(BlueprintType)
struct FFriendItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FString FriendName;
	UPROPERTY(BlueprintReadOnly)
		int32 bIsOnline;
	UPROPERTY(BlueprintReadOnly)
		int32 bIsPlayingThisGame;
	UPROPERTY(BlueprintReadOnly)
		FString UniqueID;
	UPROPERTY(BlueprintReadOnly)
		int32 ServerArrayIndex;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerFoundDelegate, FServerItem, ServerInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFriendFoundDelegate, FFriendItem, FriendInfo);

/**
 *
 */
UCLASS()
class WOG_API UWOGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UWOGGameInstance();

	virtual void Init() override;

	void Login();
	void OnLoginComplete(int ControllerIndex, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorString);

	TSharedPtr <class FOnlineSessionSearch> SearchSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int, FString> PlayersMap;


protected:
	class IOnlineSubsystem* OnlineSubsystem;

	bool bIsLoggedIn;

	UPROPERTY(BlueprintAssignable)
	FServerFoundDelegate ServerFoundDelegate;

	UPROPERTY(BlueprintAssignable)
	FFriendFoundDelegate FriendFoundDelegate;

public:
	UFUNCTION(BlueprintCallable)
	void CreateSession();
	void OnCreateSessionComplete(FName SessionName, bool bSuccessful);

	UFUNCTION(BlueprintCallable)
	void DestroySession();
	void OnDestroySessionComplete(FName SessionName, bool bSuccessful);


	UFUNCTION(BlueprintCallable)
	void GetFriends();
	void OnGetFriendsComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	UFUNCTION(BlueprintCallable)
	void FindSessions();
	void OnFindSessionsComplete(bool bWasSuccessful);

	UFUNCTION(BlueprintCallable)
	bool SendInvite(FString UniqueID);

	//void OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
	void OnSessionInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	UFUNCTION(BlueprintCallable)
	void JoinServer(int32 ServerIndex);
	void JoinFriendServer(const FOnlineSessionSearchResult& InviteResult);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(BlueprintCallable)
	void BPServerTravel(FString Address);


};