// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/WOGEpicOnlineServicesSubsystem.h"
#include "WOG.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameMode/WOGLobbyGameMode.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"
#include "VoiceChat.h"

void UWOGEpicOnlineServicesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	const IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	TSharedPtr<IOnlineLobby> Lobby = Online::GetLobbyInterface(Subsystem);
	
	Session->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &ThisClass::OnSessionUserInviteAccepted);
	Lobby->OnMemberDisconnectDelegates.AddUObject(this, &ThisClass::OnLobbyMemberDisconnected);
}

void UWOGEpicOnlineServicesSubsystem::Login()
{
	const IOnlineSubsystem *Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

	if(IsLocalUserLoggedIn())
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Orange, FString("Already Logged in"));
		return;
	}

	LoginDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(
		0, FOnLoginComplete::FDelegate::CreateUObject(this, &ThisClass::HandleLoginComplete));

	if(!Identity->AutoLogin(0))
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to AutoLogin"));
		Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginDelegateHandle);
		LoginDelegateHandle.Reset();
	}
}

void UWOGEpicOnlineServicesSubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if(bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Login successful"));
		OnLoginCompleteDelegate.Broadcast(bWasSuccessful);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("AutoLogin succeeded, but failed to Login"));
		OnLoginCompleteDelegate.Broadcast(bWasSuccessful);
	}

	// Deregister the event handler.
	const IOnlineSubsystem *Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginDelegateHandle);
	this->LoginDelegateHandle.Reset();
}

bool UWOGEpicOnlineServicesSubsystem::IsLocalUserLoggedIn() const
{
	const IOnlineSubsystem *Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	
	return Identity && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn; 
}

void UWOGEpicOnlineServicesSubsystem::CreateLobby(bool bIsPublic, bool bVoiceChat, const FString& MapName)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	TSharedPtr<IOnlineLobby> LobbyInterface = Online::GetLobbyInterface(Subsystem);
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

	/*
	 *Create the LobbyTransaction with the relevant data for the Lobby
	 */
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);
	TSharedPtr<FOnlineLobbyTransaction> LobbyTransaction = LobbyInterface->MakeCreateLobbyTransaction(*LocalUserId.Get());

	// TODO - To enable voice chat on a lobby, set the special "EOSVoiceChat_Enabled" metadata value.
	LobbyTransaction->SetMetadata.Add(TEXT("EOSVoiceChat_Enabled"), bVoiceChat);

	//Debug echo setting for the lobby
	LobbyTransaction->SetMetadata.Add(TEXT("EOSVoiceChat_Echo"), true);

	// To allow clients connecting to the listen server to join the lobby based on just the ID, we need
	// to set it to public.
	LobbyTransaction->Public = bIsPublic;

	// Here you can adjust the capacity of the lobby. 
	LobbyTransaction->Capacity = 5;

	// Setting a lobby as locked prevents players from joining it.
	LobbyTransaction->Locked = false;

	//Setting the custom parameter to filter the correct MatchTypes
	LobbyTransaction->SetMetadata.Add(TEXT("MatchType"), FVariantData(TEXT("WOG_default")));

	//Setting the custom parameter to check the Host name
	FString HostName = Identity->GetUserAccount(*LocalUserId)->GetDisplayName();
	LobbyTransaction->SetMetadata.Add(TEXT("HostName"), FVariantData(HostName));

	//Clear previous cached map name
	CachedMapName = FString();
	//Cache the desired map name
	CachedMapName = MapName;

	/*
	 * Create the Lobby
	 */
	if (!LobbyInterface->CreateLobby(*LocalUserId.Get(), *LobbyTransaction,
	FOnLobbyCreateOrConnectComplete::CreateLambda([&](
		const FOnlineError & Error,
		const FUniqueNetId & UserId,
		const TSharedPtr<FOnlineLobby> & CreatedLobby)
	{
		//Create Lobby call successful
		HandleCreateLobbyCompleted(Error, UserId, CreatedLobby);
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Create Lobby call successful"));	})))
	{
		// Call failed to start.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Call to Create Lobby failed to start"));
		OnLobbyCreatedDelegate.Broadcast(false);
	}
}

void UWOGEpicOnlineServicesSubsystem::HandleCreateLobbyCompleted(const FOnlineError& Error, const FUniqueNetId& UserId,
	const TSharedPtr<FOnlineLobby>& CreatedLobby)
{
	if (Error.WasSuccessful())
	{
		// The lobby was created successfully and is now in CreatedLobby.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Lobby Created"));
		LobbyIdString = CreatedLobby->Id->ToString();
		OnLobbyCreatedDelegate.Broadcast(true);
	
		UGameplayStatics::OpenLevel(this, *CachedMapName, true, FString("listen"));
		// You'll need to store IdStr somewhere, as that is what needs to be sent to connecting clients.

		VoiceChatLogin();
	}
	else
	{
		// Lobby could not be created.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to Create Lobby"));
		OnLobbyCreatedDelegate.Broadcast(false);
	}
}

void UWOGEpicOnlineServicesSubsystem::GetLobbyMembers()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);
	
	for (auto It = this->GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (PlayerController->IsLocalPlayerController())
		{
			//We skip the host here. Might add the code here if needed
			continue;
		}

		FUniqueNetIdRepl UniqueNetIdRepl;
		UNetConnection *RemoteNetConnection = Cast<UNetConnection>(PlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;

		// Get the unique player ID.
		TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
		check(UniqueNetId != nullptr);

		if(!IsUserIdLobbyMember(UniqueNetId)) continue;
		
		FLobbyMemberData LobbyMemberData = FLobbyMemberData();
		LobbyMemberData.MemberDisplayName = PlayerController->PlayerState->GetPlayerName();
		LobbyMemberData.MemberUserIDString = UniqueNetId->ToString();

		LobbyMemberFoundDelegate.Broadcast(LobbyMemberData);
		}
	}

void UWOGEpicOnlineServicesSubsystem::DestroyLobby()
{
	if(LobbyIdString==FString()) return;
	
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if(!Subsystem) return;
	
	TSharedPtr<IOnlineLobby> Lobby = Online::GetLobbyInterface(Subsystem);
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);
	TSharedPtr<FOnlineLobbyId> CurrentLobbyID = Lobby->ParseSerializedLobbyId(LobbyIdString);

	if (!Lobby->DeleteLobby(
	*LocalUserId,
	*CurrentLobbyID,
	FOnLobbyOperationComplete::CreateLambda([&](
		const FOnlineError & Error,
		const FUniqueNetId & UserId)
	{
		if (Error.WasSuccessful())
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Lobby deleted successfully"));
			LobbyIdString = FString();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Lobby not deleted"));
		}
	})))
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("DeleteLobby function call failed"));
	}
}

void UWOGEpicOnlineServicesSubsystem::SearchLobbies()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	TSharedPtr<IOnlineLobby> Lobby = Online::GetLobbyInterface(Subsystem);
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);

	TSharedRef<FOnlineLobbySearchQuery> Search = MakeShared<FOnlineLobbySearchQuery>();

	// To search by settings, add a LobbySetting and SettingValue to search for:
	Search->Filters.Add(
		FOnlineLobbySearchQueryFilter(
		FString(TEXT("MatchType")),
		FString(TEXT("WOG_default")),
		EOnlineLobbySearchQueryFilterComparator::Equal));

	if (!Lobby->Search(
	*LocalUserId.Get(),
	*Search,
	FOnLobbySearchComplete::CreateLambda([&, Lobby, LocalUserId](const FOnlineError &Error,
	                                                             const FUniqueNetId &UserId,
	                                                             const TArray<TSharedRef<const FOnlineLobbyId>> &Lobbies)
	{
		if (Error.WasSuccessful())
		{
			// The search was successful, access the results
			// via the "Lobbies" parameter.
			if(Lobbies.IsEmpty()) return;

			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Lobby search complete. Lobbies found: ") + FString::FromInt(Lobbies.Num()));

			int8 ArrayIndex = -1;
			for(auto FoundLobby : Lobbies)
			{
				ArrayIndex++;
				FServerItem ServerInfo;
				
				FVariantData HostName;
				FString HostNameString = FString();
				Lobby->GetLobbyMetadataValue(*LocalUserId, *FoundLobby, FString("HostName"), HostName);
				HostName.GetValue(HostNameString);				
				ServerInfo.HostName = HostNameString;
				
				ServerInfo.NumMaxPlayers = 5;
				int32 MemberCount = 0;
				Lobby->GetMemberCount(*LocalUserId.Get(), *FoundLobby, MemberCount);
				ServerInfo.NumCurrentPlayers = MemberCount;
				
				ServerInfo.ServerArrayIndex = ArrayIndex;
				ServerInfo.SetCurrentPlayers();

				ServerFoundDelegate.Broadcast(ServerInfo);
			}
		}
		else
		{
			// The search failed, refer to the "Error" parameter
			// for more detail on the error.
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Lobby search failed"));
		}
	})))
	{
		// Call failed to start.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Lobby search call failed to start"));
	}
}

void UWOGEpicOnlineServicesSubsystem::JoinLobby(const FString& DesiredLobbyIdString)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	TSharedPtr<IOnlineLobby> Lobby = Online::GetLobbyInterface(Subsystem);
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);
	TSharedPtr<FOnlineLobbyId> DesiredLobbyID = Lobby->ParseSerializedLobbyId(DesiredLobbyIdString);
	
	if (!Lobby->ConnectLobby(
	*LocalUserId,
	*DesiredLobbyID,
	FOnLobbyCreateOrConnectComplete::CreateLambda([this](
		const FOnlineError & Error,
		const FUniqueNetId & UserId,
		const TSharedPtr<class FOnlineLobby> & CreatedLobby)
	{
		if (Error.WasSuccessful())
		{
			// The lobby was joined successfully.
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Lobby joined successfully"));
			VoiceChatLogin();
		}
		else
		{
			// Lobby could not be joined.
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Lobby join failed"));
		}
	})))
	{
		// Call failed to start.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Join lobby call failed to start"));
	}
}

void UWOGEpicOnlineServicesSubsystem::KickFromLobby(const FString& LobbyMemberIDString)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	TSharedPtr<IOnlineLobby> Lobby = Online::GetLobbyInterface(Subsystem);
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);
	TSharedPtr<FOnlineLobbyId> DesiredLobbyID = Lobby->ParseSerializedLobbyId(LobbyIdString);

	TSharedPtr<const FUniqueNetId> MemberUserIdToKick = Identity->CreateUniquePlayerId(LobbyMemberIDString);
	
	if(!MemberUserIdToKick.IsValid() || !IsUserIdLobbyMember(MemberUserIdToKick))
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Invalid UserID for kicking OR UserID is not a valid Lobby member"));
		return;
	}
	
	if (!Lobby->KickMember(
	*LocalUserId.Get(),
	*DesiredLobbyID,
	*MemberUserIdToKick,
	FOnLobbyOperationComplete::CreateLambda([](
		const FOnlineError & Error,
		const FUniqueNetId & UserId)
	{
		if (Error.WasSuccessful())
		{
			// The member was kicked successfully.
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Member Kicked successfully"));
		}
		else
		{
			// Member could not be kicked.
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Member could not be kicked"));
		}
	})))
	{
		// Call failed to kick.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Kick Call failed"));
	}
}

bool UWOGEpicOnlineServicesSubsystem::IsUserIdLobbyMember(const TSharedPtr<const FUniqueNetId>& UserID) const
{
	bool bIsValidMember = false;
	
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	TSharedPtr<IOnlineLobby> Lobby = Online::GetLobbyInterface(Subsystem);
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);
	TSharedPtr<FOnlineLobbyId> CurrentLobbyID = Lobby->ParseSerializedLobbyId(LobbyIdString);

	int32 MemberCount = 0;
	Lobby->GetMemberCount(*LocalUserId.Get(), *CurrentLobbyID.Get(), MemberCount);

	for (int32 i = 0; i < MemberCount; i++) 
	{
		TSharedPtr<const FUniqueNetId> MemberId;
		Lobby->GetMemberUserId(*LocalUserId, *CurrentLobbyID, i, MemberId);
		if (*MemberId == *UserID) 
		{
			bIsValidMember = true;
			break;
		}
	}
	
	return bIsValidMember;
}

void UWOGEpicOnlineServicesSubsystem::DisconnectFromLobby()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	TSharedPtr<IOnlineLobby> Lobby = Online::GetLobbyInterface(Subsystem);
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);

	/*
	* Handle disconnect from lobby
	*/
	FOnlineSessionSettings* SessionSettings = Session->GetSessionSettings(WOG_SESSION_NAME);
	FString OutLobbyID = FString();
	SessionSettings->Get(FName("LobbyID"), OutLobbyID);
	TSharedPtr<FOnlineLobbyId> DesiredLobbyID = Lobby->ParseSerializedLobbyId(OutLobbyID);
	
	if (!Lobby->DisconnectLobby(
	*LocalUserId,
	*DesiredLobbyID,
	FOnLobbyOperationComplete::CreateLambda([](
		const FOnlineError& Error,
		const FUniqueNetId& UserId)
	{
		if (Error.WasSuccessful())
		{
			// Player has been disconnected from lobby.
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Disconnect from Lobby successful"));
		}
		else
		{
			// Could not disconnect from lobby.
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Disconnect from Lobby failed"));
		}
	})
))
	{
		// Call failed to start.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Disconnect from lobby call failed to start"));
	}
}

void UWOGEpicOnlineServicesSubsystem::OnLobbyMemberDisconnected(const FUniqueNetId& LocalUserId,
	const FOnlineLobbyId& LobbyId, const FUniqueNetId& MemberId, bool bWasKicked)
{
	if(LocalUserId == MemberId)
	{
		//Logout user from the lobby voice chat
		VoiceChatLogout();
		
		if(bWasKicked)
		{
			UGameplayStatics::OpenLevel(this->GetWorld(), FName("StartUp"), true);
		}
	}
}

void UWOGEpicOnlineServicesSubsystem::CreateSession()
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	const IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	//Necessary to get the host name
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);
	
	CreateSessionDelegateHandle =
	Session->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionComplete::FDelegate::CreateUObject(
		this,
		&ThisClass::HandleCreateSessionComplete));

	TSharedRef<FOnlineSessionSettings> SessionSettings = MakeShared<FOnlineSessionSettings>();
	SessionSettings->NumPublicConnections = 5; // The number of players.
	SessionSettings->bShouldAdvertise = true;  // Set to true to make this session discoverable with FindSessions.
	SessionSettings->bUsesPresence = true;    // Set to true if you want this session to be discoverable by presence (Epic Social Overlay).
	SessionSettings->bIsDedicated = false;
	SessionSettings->bIsLANMatch = false;
	SessionSettings->bAllowJoinInProgress = false;
	SessionSettings->bAllowJoinViaPresence = false;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->Set(SEARCH_KEYWORDS, FOnlineSessionSetting(WOG_SESSION_NAME.ToString(), EOnlineDataAdvertisementType::ViaOnlineService));
	SessionSettings->Set(FName("MatchType"),FOnlineSessionSetting( FString("WOG_default"), EOnlineDataAdvertisementType::ViaOnlineService));
	SessionSettings->Set(FName("LobbyID"), FOnlineSessionSetting(LobbyIdString, EOnlineDataAdvertisementType::ViaOnlineService));
	
	FString HostName = Identity->GetUserAccount(*LocalUserId)->GetDisplayName();
	SessionSettings->Set(FName("HostName"),FOnlineSessionSetting( HostName, EOnlineDataAdvertisementType::ViaOnlineService));

	// Create a session and give the local name "MyLocalSessionName". This name is entirely local to the current player and isn't stored in EOS.
	if (!Session->CreateSession(0, WOG_SESSION_NAME, *SessionSettings))
	{
		// Call didn't start, return error.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Create Session call failed to start"));

		//Register existing players
		if(AWOGLobbyGameMode* LobbyGameMode = Cast<AWOGLobbyGameMode>(GetWorld()->GetAuthGameMode()))
		{
			LobbyGameMode->RegisterExistingPlayers();
		}
	}
}

void UWOGEpicOnlineServicesSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		//Session created successfully
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Session created successfully"));

		//Update presence status
		UpdatePresence(FString("Raiding villages"));

		//Register existing players
		if(AWOGLobbyGameMode* LobbyGameMode = Cast<AWOGLobbyGameMode>(GetWorld()->GetAuthGameMode()))
		{
			LobbyGameMode->RegisterExistingPlayers();
		}
	}
	else
	{
		//Session was not created
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to create Session"));
	}
	
	// Deregister the event handler.
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
	CreateSessionDelegateHandle.Reset();
}

void UWOGEpicOnlineServicesSubsystem::FindSession()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	TSharedRef<FOnlineSessionSearch> Search = MakeShared<FOnlineSessionSearch>();
	// Remove the default search parameters that FOnlineSessionSearch sets up.
	Search->QuerySettings.SearchParams.Empty();

	// Add your search settings here. If you're including both listening and non-listening sessions as per the __EOS_bListening example above, then you must include at least one additional filter, or EOS will not return any search results.
	Search->QuerySettings.Set(FName("MatchType"),FString("WOG_default"),EOnlineComparisonOp::Equals);
	Search->QuerySettings.Set(SEARCH_KEYWORDS, WOG_SESSION_NAME.ToString(), EOnlineComparisonOp::Equals);
	Search->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	Search->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	Search->MaxSearchResults = 100;
	Search->bIsLanQuery = false;

	this->FindSessionsDelegateHandle =
	Session->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsComplete::FDelegate::CreateUObject(
		this,
		&ThisClass::HandleFindSessionsComplete,
		Search));

	if (!Session->FindSessions(0, Search))
	{
		// Call didn't start, return error.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Find Session call failed to start"));
	}
}

void UWOGEpicOnlineServicesSubsystem::HandleFindSessionsComplete(bool bWasSuccessful, TSharedRef<FOnlineSessionSearch> Search)
{
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	if (bWasSuccessful)
	{
		if (Search->SearchResults.IsEmpty())
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("No sessions found"));
			return;
		}

		int8 ArrayIndex = -1;

		//Caching the search results for future access
		CachedSessionSearchResults.Empty();
		CachedSessionSearchResults = Search->SearchResults;
		
		for (FOnlineSessionSearchResult Result : Search->SearchResults)
		{
			++ArrayIndex;
			if (!Result.IsValid()) continue;

			FServerItem ServerInfo;
			FString OutHostName;
			Result.Session.SessionSettings.Get(FName("HostName"), OutHostName);
			ServerInfo.HostName = OutHostName;
			ServerInfo.NumMaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			ServerInfo.NumCurrentPlayers = (Result.Session.SessionSettings.NumPublicConnections) - (Result.Session.NumOpenPublicConnections);
			ServerInfo.ServerArrayIndex = ArrayIndex;
			ServerInfo.SetCurrentPlayers();

			ServerFoundDelegate.Broadcast(ServerInfo);
		}
	}

	Session->ClearOnFindSessionsCompleteDelegate_Handle(this->FindSessionsDelegateHandle);
	this->FindSessionsDelegateHandle.Reset();
}

void UWOGEpicOnlineServicesSubsystem::DestroySession()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	DestroySessionDelegateHandle =
	Session->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionComplete::FDelegate::CreateUObject(
			this,
			&ThisClass::HandleDestroySessionComplete));

	// "MyLocalSessionName" is the local name of the session for the client or server. It should be the value you specified when creating or joining the session (depending on which you called).
	if (!Session->DestroySession(WOG_SESSION_NAME))
	{
		// Call didn't start, return error.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Destroy Session call failed to start"));
	}
}

void UWOGEpicOnlineServicesSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Session destroyed successfully"));
		DestroyLobby();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to destroy Session"));
	}

	// Deregister the event handler.
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	Session->ClearOnDestroySessionCompleteDelegate_Handle(this->DestroySessionDelegateHandle);
	this->DestroySessionDelegateHandle.Reset();
}

void UWOGEpicOnlineServicesSubsystem::JoinSession(int32 SessionIndex)
{
	if(CachedSessionSearchResults.IsEmpty() || !CachedSessionSearchResults[SessionIndex].IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("No valid cached search results"));
		return;
	}

	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	
	JoinSessionDelegateHandle =
	Session->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionComplete::FDelegate::CreateUObject(
			this,
			&ThisClass::HandleJoinSessionComplete));

	// "WOG_SESSION_NAME" is the local name of the session for this player. It doesn't have to match the name the server gave their session.
	if (!Session->JoinSession(0, WOG_SESSION_NAME, CachedSessionSearchResults[SessionIndex]))
	{
		// Call didn't start, return error.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Join Session call failed to start"));
	}
}

void UWOGEpicOnlineServicesSubsystem::AttemptReconnectSession()
{
	if(!CachedReconnectSession.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Search result invalid"));
		return;
	}

	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	
	JoinSessionDelegateHandle =
	Session->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionComplete::FDelegate::CreateUObject(
			this,
			&ThisClass::HandleJoinSessionComplete));

	// "WOG_SESSION_NAME" is the local name of the session for this player. It doesn't have to match the name the server gave their session.
	if (!Session->JoinSession(0, WOG_SESSION_NAME, CachedReconnectSession))
	{
		// Call didn't start, return error.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Join Session call failed to start"));
	}
}

void UWOGEpicOnlineServicesSubsystem::HandleJoinSessionComplete(FName SessionName,	EOnJoinSessionCompleteResult::Type JoinResult)
{
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	
	if (JoinResult == EOnJoinSessionCompleteResult::Success || JoinResult == EOnJoinSessionCompleteResult::AlreadyInSession)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Session joined successfully"));

		FOnlineSessionSettings* SessionSettings = Session->GetSessionSettings(SessionName);
		FString OutLobbyID;
		SessionSettings->Get(FName("LobbyID"), OutLobbyID);

		JoinLobby(OutLobbyID);
		
		FString ConnectInfo;
		Session->GetResolvedConnectString(SessionName, ConnectInfo, NAME_GamePort);
		
		FURL NewURL(nullptr, *ConnectInfo, ETravelType::TRAVEL_Absolute);
		FString BrowseError;
		if (GEngine->Browse(GEngine->GetWorldContextFromWorldChecked(GetWorld()), NewURL, BrowseError) ==
			EBrowseReturnVal::Failure)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to start browse: ") + BrowseError);
		}
		
		// Use the connection string that you got from FindSessions in order
		// to connect to the server.
		//
		// Refer to "Connecting to a game server" under the "Networking & Anti-Cheat"
		// section of the documentation for more information on how to do this.
		//
		// NOTE: You can also call GetResolvedConnectString at this point instead
		// of in FindSessions, but it's recommended that you call it in
		// FindSessions, so you know the result is valid.
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to join session"));
	}

	Session->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionDelegateHandle);
	this->JoinSessionDelegateHandle.Reset();
}

bool UWOGEpicOnlineServicesSubsystem::StartSession()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	return Session->StartSession(WOG_SESSION_NAME);
}

bool UWOGEpicOnlineServicesSubsystem::EndSession()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	return Session->EndSession(WOG_SESSION_NAME);
}

void UWOGEpicOnlineServicesSubsystem::SearchReconnectSession()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);

	this->FindFriendSessionDelegateHandle =
	Session->AddOnFindFriendSessionCompleteDelegate_Handle(
		0, 
		FOnFindFriendSessionComplete::FDelegate::CreateUObject(
			this,
			&ThisClass::HandleFindFriendSessionComplete));

	if (!Session->FindFriendSession(0, *LocalUserId))
	{
		// Call didn't start, return error.
		Session->ClearOnFindFriendSessionCompleteDelegate_Handle(0 /* LocalUserNum */, this->FindFriendSessionDelegateHandle);
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to call FindFriendSession"));
	}
}

void UWOGEpicOnlineServicesSubsystem::HandleFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful,
	const TArray<FOnlineSessionSearchResult>& Results)
{
	// NOTE: bWasSuccessful is only true if the call succeeded *AND* a session was found.
	// It's normal for it to be false when the call succeeds if the user doesn't have
	// a session to reconnect to. You should not treat a false value as an indication to
	// retry the request.

	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	if (bWasSuccessful && Results.Num() > 0)
	{
		// The user has a session they can reconnect to. You can handle the (at most one)
		// search result from the Results array the same way that search results
		// are handled in "Finding a session".
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Emerald, FString("Found friend session, caching it..."));
		CachedReconnectSession = Results[0];
		
		FString OutHostName;
		CachedReconnectSession.Session.SessionSettings.Get(FName("HostName"), OutHostName);
		ReconnectSessionFoundDelegate.Broadcast(true, OutHostName);

		Session->ClearOnFindFriendSessionCompleteDelegate_Handle(0, FindFriendSessionDelegateHandle);
		FindFriendSessionDelegateHandle.Reset();
		return;
	}

	// Otherwise, the user does not have a session to reconnect to.
	Session->ClearOnFindFriendSessionCompleteDelegate_Handle(0, FindFriendSessionDelegateHandle);
	FindFriendSessionDelegateHandle.Reset();
	ReconnectSessionFoundDelegate.Broadcast(false, FString());
}

void UWOGEpicOnlineServicesSubsystem::OnVoiceChatLoginComplete(const FString& PlayerName,
	const FVoiceChatResult& Result)
{
	if (Result.IsSuccess())
	{
		// You can now use this->VoiceChatUser to control the user's voice chat.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, PlayerName + FString(" logged in VoiceChat successfully"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, Result.ErrorDesc);
	}
}

void UWOGEpicOnlineServicesSubsystem::UnregisterFromSessionUsingPlayerController(APlayerController* InPlayerController)
{
	check(IsValid(InPlayerController));
	
	if(InPlayerController->GetLocalRole() != ROLE_Authority) return;

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer *LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	// Get the online session interface.
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	/*
	* Handle unregister the player
	* Unregister the player with the "WOG_SESSION_NAME" session; this name should match the name you provided in CreateSession.
	*/
	if (!Session->UnregisterPlayer(WOG_SESSION_NAME, *UniqueNetId))
	{
		// The player could not be unregistered.
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to call Unregister player"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Emerald, FString("Call to unregister player successful"));
	}
}

bool UWOGEpicOnlineServicesSubsystem::UnregisterAllSessionMembers()
{
	if(CachedSessionMemberIds.IsEmpty()) return false;
	
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);

	bool bWasSuccessful = true;
	for (auto MemberID : CachedSessionMemberIds)
	{
		if(*LocalUserId == *MemberID)
		{
			continue;	
		}
		
		if(!Session->UnregisterPlayer(WOG_SESSION_NAME, *MemberID))
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Orange, FString("Unregister failed: ") + *MemberID->ToString());
			bWasSuccessful = false;
		}
	}

	return bWasSuccessful;
}

void UWOGEpicOnlineServicesSubsystem::OnSessionUserInviteAccepted(bool bWasSuccessful, int ControllerId,
                                                                  TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if(bWasSuccessful)
	{
		JoinFriendServer(InviteResult);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString("Failed to join friend's session"));
	}
}

void UWOGEpicOnlineServicesSubsystem::JoinFriendServer(const FOnlineSessionSearchResult& InviteResult)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	
	if (InviteResult.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString("Joining friend"));
		Session.Get()->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::HandleJoinSessionComplete);
		Session.Get()->JoinSession(0, WOG_SESSION_NAME, InviteResult);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Invite result not valid. Not joining friend"));
	}
}

void UWOGEpicOnlineServicesSubsystem::UpdatePresence(const FString& NewPresenceStatus) const
{
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	IOnlinePresencePtr Presence = Subsystem->GetPresenceInterface();

	FOnlineUserPresenceStatus Status;
	Status.State = EOnlinePresenceState::Online;
	Status.StatusStr = NewPresenceStatus;

	Presence->SetPresence(
		*Identity->GetUniquePlayerId(0).Get(),
		Status,
		IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateLambda([](
			const class FUniqueNetId &UserId,
			const bool bWasSuccessful)
			{
				if(bWasSuccessful)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString("Presence Status updated"));
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("Presence Status failed to update"));
				}
			}));
}

void UWOGEpicOnlineServicesSubsystem::VoiceChatLogin()
{
	IVoiceChat* VoiceChat = IVoiceChat::Get();
	VoiceChatUser = VoiceChat->CreateUser();

	IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

	TSharedPtr<const FUniqueNetId> UserId = Identity->GetUniquePlayerId(0);
	FPlatformUserId PlatformUserId = Identity->GetPlatformUserIdFromUniqueNetId(*UserId);

	VoiceChatUser->Login(
		PlatformUserId,
		UserId->ToString(),
		TEXT(""),
		FOnVoiceChatLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceChatLoginComplete));
}

void UWOGEpicOnlineServicesSubsystem::VoiceChatLogout()
{
	IVoiceChat* VoiceChat = IVoiceChat::Get();
	if (VoiceChatUser != nullptr && VoiceChat != nullptr)
	{
		VoiceChat->ReleaseUser(this->VoiceChatUser);
		VoiceChatUser = nullptr;
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Orange, FString("VoiceChatLogout called"));
	}
}
