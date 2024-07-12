// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RedpointInterfaces/OnlineLobbyInterface.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "WOGEpicOnlineServicesSubsystem.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FServerItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString HostName = FString("");
	UPROPERTY(BlueprintReadOnly)
	FString CurrentPlayers = FString("");
	UPROPERTY()
	int32 NumCurrentPlayers = 0;
	UPROPERTY()
	int32 NumMaxPlayers = 0;
	UPROPERTY(BlueprintReadOnly)
	int32 ServerArrayIndex = 0;

	void SetCurrentPlayers()
	{
		CurrentPlayers = FString(FString::FromInt(NumCurrentPlayers) + " / " + FString::FromInt(NumMaxPlayers));
	}

};

USTRUCT(BlueprintType)
struct FLobbyMemberData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString MemberDisplayName = FString("");

	UPROPERTY(BlueprintReadOnly)
	FString MemberUserIDString = FString("");
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginProcessCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyCreatedDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerFoundDelegate, FServerItem, ServerInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLobbyMemberFoundDelegate, const FLobbyMemberData&, LobbyMemberData);

UCLASS()
class WOG_API UWOGEpicOnlineServicesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable)
	FOnLoginProcessCompleteDelegate OnLoginCompleteDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnLobbyCreatedDelegate OnLobbyCreatedDelegate;

	UPROPERTY(BlueprintAssignable)
	FServerFoundDelegate ServerFoundDelegate;

	UPROPERTY(BlueprintAssignable)
	FLobbyMemberFoundDelegate LobbyMemberFoundDelegate;

	UFUNCTION(BlueprintCallable)
	void UnregisterPlayerFromSession(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable)
	void DisconnectFromLobby();

protected:
	UFUNCTION(BlueprintCallable)
	void Login();

	UFUNCTION(BlueprintCallable)
	void CreateLobby(bool bIsPublic, bool bVoiceChat, const FString& MapName);

	UFUNCTION(BlueprintCallable)
	void GetLobbyMembers();

	UFUNCTION(BlueprintCallable)
	void DestroyLobby();

	UFUNCTION(BlueprintCallable)
	void SearchLobbies();

	UFUNCTION(BlueprintCallable)
	void JoinLobby(const FString& DesiredLobbyIdString);

	UFUNCTION(BlueprintCallable)
	void KickFromLobby(const FString& LobbyMemberIDString);

	UFUNCTION(BlueprintCallable)
	void CreateSession();

	UFUNCTION(BlueprintCallable)
	void FindSession();
	
	UFUNCTION(BlueprintCallable)
	void DestroySession();

	UFUNCTION(BlueprintCallable)
	void JoinSession(int32 SessionIndex);

	//true if the call succeeds, false otherwise
	UFUNCTION(BlueprintCallable)
	bool StartSession();

	//true if the call succeeds, false otherwise
	UFUNCTION(BlueprintCallable)
	bool EndSession();

	void JoinFriendServer(const FOnlineSessionSearchResult& InviteResult);

	void UpdatePresence(const FString& NewPresenceStatus = FString()) const;
private:
	
	#pragma region Callback functions
	//This function will run when a login is complete.
	void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error);

	//This function will run when a lobby is created via RedpointAPI.
	void HandleCreateLobbyCompleted(const FOnlineError& Error, const FUniqueNetId& UserId, const TSharedPtr<FOnlineLobby>& CreatedLobby);

	//This function will run when session is created
	void HandleCreateSessionComplete(FName SessionName,	bool bWasSuccessful);

	//This function will run when session search is complete
	void HandleFindSessionsComplete(bool bWasSuccessful, TSharedRef<FOnlineSessionSearch> Search);

	//This function will run when session is destroyed
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	//This function will run when session is joined
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

	//This function handles User invites
	void OnSessionUserInviteAccepted(bool bWasSuccessful, int ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);

	//This function handles user being disconnected from lobby
	void OnLobbyMemberDisconnected(const FUniqueNetId & LocalUserId, const FOnlineLobbyId & LobbyId, const FUniqueNetId & MemberId,	bool bWasKicked);
	#pragma endregion

	#pragma region Delegate handles
	// Delegate to bind callback event for when login is complete.
	FDelegateHandle LoginDelegateHandle;
	// Delegate to bind callback event for when sessions are found.
	FDelegateHandle FindSessionsDelegateHandle;
	// Delegate to bind callback event for when lobby is created.
	FDelegateHandle CreateLobbyDelegateHandle;
	// Delegate to bind callback event for when session is created.
	FDelegateHandle CreateSessionDelegateHandle;
	// Delegate to bind callback event for when session is destroyed.
	FDelegateHandle DestroySessionDelegateHandle;
	// Delegate to bind callback event for when session is joined.
	FDelegateHandle JoinSessionDelegateHandle;

	
	#pragma endregion

	#pragma region Cached Variables
	FString LobbyIdString = FString();

	TArray<FOnlineSessionSearchResult> CachedSessionSearchResults = {};

	TArray<TSharedPtr<const FUniqueNetId>> CachedMemberIds;

	FString CachedMapName = FString();
	#pragma endregion 

public:

	UFUNCTION(BlueprintPure)
	bool IsLocalUserLoggedIn() const;

	bool IsUserIdLobbyMember(const TSharedPtr<const FUniqueNetId>& UserID) const;
};
