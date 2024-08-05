// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RedpointInterfaces/OnlineLobbyInterface.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "VoiceChat.h"
#include "WOGEpicOnlineServicesSubsystem.generated.h"


/**
 * 
 */
/*Voice chat channel section

UENUM(BlueprintType)
enum class EWOGResultCodes :uint8
{
	Success = 0,
	Failed = 1
};

struct FWOGChannelCredentials : public FJsonSerializable
{
	FString OverrideUserId;
	FString ClientBaseUrl;
	FString ParticipantToken;

	BEGIN_JSON_SERIALIZER
	JSON_SERIALIZE("override_userid", OverrideUserId);
	JSON_SERIALIZE("client_base_url", ClientBaseUrl);
	JSON_SERIALIZE("participant_token", ParticipantToken);
	END_JSON_SERIALIZER
};*/
//DECLARE_DYNAMIC_DELEGATE_TwoParams(FWOGResultDelegate, bool, bWasSuccess, EWOGResultCodes, Result);
//DECLARE_DYNAMIC_DELEGATE_ThreeParams(FWOGChannelTokenResultDelegate, bool, bWasSuccess, FString, ChannelCredentials, APlayerController*, PlayerController);
//DECLARE_DELEGATE_FourParams(FWOGChannelTokenResultCppDelegate, bool, /*bWasSuccess,*/ const FString&, /*ChannelCredentials*/ const FString&, /*ChannelName*/ APlayerController* /*PlayerController*/);
//DECLARE_DELEGATE_TwoParams(FWOGResultCppDelegate, bool, /*bWasSuccess*/ EWOGResultCodes /*Result*/);

USTRUCT(BlueprintType)
struct FWOGVoiceChatDeviceData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString DeviceName = FString("");

	UPROPERTY(BlueprintReadOnly)
	FString DeviceID = FString("");
};

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FReconnectSessionFoundDelegate, const bool, ReconnectSessionFound, const FString&, HostName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLobbyMemberFoundDelegate, const FLobbyMemberData&, LobbyMemberData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVoiceChatPlayerIsTalkingUpdated, const FString&, ChannelName, const FString&, PlayerName, bool, bIsTalking);

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

	UPROPERTY(BlueprintAssignable)
	FReconnectSessionFoundDelegate ReconnectSessionFoundDelegate;

	UFUNCTION(BlueprintCallable)
	void UnregisterFromSessionUsingPlayerController(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable)
	void DisconnectFromLobby();

protected:

	#pragma region User
	
	UFUNCTION(BlueprintCallable)
	void Login();
	
	void UpdatePresence(const FString& NewPresenceStatus = FString()) const;
	#pragma endregion
	
	#pragma region Sessions
	
	UFUNCTION(BlueprintCallable)
	void CreateSession();

	UFUNCTION(BlueprintCallable)
	void FindSession();
	
	UFUNCTION(BlueprintCallable)
	void DestroySession();

	UFUNCTION(BlueprintCallable)
	void JoinSession(int32 SessionIndex);

	UFUNCTION(BlueprintCallable)
	void AttemptReconnectSession();

	//true if the call succeeds, false otherwise
	UFUNCTION(BlueprintCallable)
	bool StartSession();

	//true if the call succeeds, false otherwise
	UFUNCTION(BlueprintCallable)
	bool EndSession();

	UFUNCTION(BlueprintCallable)
	void SearchReconnectSession();

	void JoinFriendServer(const FOnlineSessionSearchResult& InviteResult);
	
	#pragma endregion

	#pragma region Lobbies
	UFUNCTION(BlueprintCallable)
	void CreateLobby(bool bIsPublic, bool bVoiceChat, const FString& LobbyMapName, const FString& MatchMapPath);

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
	#pragma endregion

	#pragma region VoiceChat	
	void VoiceChatLogin();
	void VoiceChatLogout();

	UFUNCTION(BlueprintCallable)
	TArray<FWOGVoiceChatDeviceData> GetAvailableInputDeviceInfos();

	UFUNCTION(BlueprintCallable)
	TArray<FWOGVoiceChatDeviceData> GetAvailableOutputDeviceInfos();

	UFUNCTION(BlueprintCallable)
	FWOGVoiceChatDeviceData GetCurrentInputDeviceInfo();
	
	UFUNCTION(BlueprintCallable)
	FWOGVoiceChatDeviceData GetCurrentOutputDeviceInfo();
	
	UFUNCTION(BlueprintCallable)
	void SetCurrentInputDevice(const FWOGVoiceChatDeviceData& NewDevice);

	UFUNCTION(BlueprintCallable)
	void SetCurrentOutputDevice(const FWOGVoiceChatDeviceData& NewDevice);

	//Value between 0.0 and 200.0
	UFUNCTION(BlueprintCallable)
	void SetVoiceChatInputVolume(float NewVolume);

	//Value between 0.0 and 200.0
	UFUNCTION(BlueprintCallable)
	void SetVoiceChatOutputVolume(float NewVolume);

	UFUNCTION(BlueprintPure)
	float GetVoiceChatInputVolume() const;

	UFUNCTION(BlueprintPure)
	float GetVoiceChatOutputVolume() const;

	UFUNCTION(BlueprintCallable)
	void SetVoiceChatInputDeviceMuted(bool bIsMuted);

	UFUNCTION(BlueprintCallable)
	void SetVoiceChatOutputDeviceMuted(bool bIsMuted);

	UFUNCTION(BlueprintPure)
	bool GetVoiceChatInputDeviceMuted() const;

	UFUNCTION(BlueprintPure)
	bool GetVoiceChatOutputDeviceMuted() const;
	
	#pragma region VoiceChat channels - NOT WORKING
	/*
	void OnResultDelegateFired(bool bWasSuccess,  EWOGResultCodes Result);
	void ConnectToEOSVoiceChat(const FWOGResultCppDelegate& ResultDelegate) const;
	void LoginToEOSVoiceChat(const FWOGResultCppDelegate& Result);

	void CredentialsComplete(const FOnlineError& OnlineError, const FUniqueNetId& UniqueNetId, const TArray<FVoiceAdminChannelCredentials>& VoiceAdminChannelCredentialses) const;
	void HandleVoiceChannelsCredentials(APlayerController* NewPlayer);
	void OnChannelTokenRetrieved(bool bWasSuccessful, const FString& ChannelCredentials, const FString& ChannelName, APlayerController* PlayerController) const;
	
	//Requests a channel token for the specified voice channel.
	void GetWOGChannelToken(
		APlayerController* PlayerController,
		const FString& VoiceRoomName,
		const FString& PlayerName,
		const FString& ClientIP,
		const FWOGChannelTokenResultCppDelegate& Result);
	
	//Joins the specified voice channel.
	void JoinWOGVoiceChannel(APlayerController* PlayerController, const FString& VoiceChannelName, const FString& ChannelCredentials, const FWOGResultCppDelegate& Result);
	*/
	#pragma endregion 

	#pragma endregion
	
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

	void HandleFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult> &Results);

	//This function handles user being logged in for the Voice Chat
	void OnVoiceChatLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result);
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
	// Delegate to bind callback event for when find friend session is complete.
	FDelegateHandle FindFriendSessionDelegateHandle;
	#pragma endregion

	#pragma region Cached Variables
	FString LobbyIdString = FString();

	TArray<FOnlineSessionSearchResult> CachedSessionSearchResults = {};

	FString CachedLobbyMapName = FString();
	FString CachedMatchMapPath = FString();

	FOnlineSessionSearchResult CachedReconnectSession = FOnlineSessionSearchResult();
	
	IVoiceChatUser* VoiceChatUser;

private:
#pragma endregion 

public:

	UFUNCTION(BlueprintPure)
	bool IsLocalUserLoggedIn() const;

	bool IsUserIdLobbyMember(const TSharedPtr<const FUniqueNetId>& UserID) const;
	
	bool UnregisterAllSessionMembers();

	TArray<TSharedPtr<const FUniqueNetId>> CachedSessionMemberIds;
	
	FORCEINLINE IVoiceChatUser* GetVoiceChatUser() const { return VoiceChatUser; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FString GetCachedMatchMapPath() const { return CachedMatchMapPath; }
};
