// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/LocalPlayer.h"
#include "Online/OnlineSessionNames.h"

const FName SESSION_NAME = TEXT("WOG Session");

UWOGGameInstance::UWOGGameInstance()
{
	bIsLoggedIn = false;
}

void UWOGGameInstance::Init()
{
	Super::Init();

	OnlineSubsystem = IOnlineSubsystem::Get();
	//Login();

	if (!OnlineSubsystem) return;

	IOnlineSessionPtr SessionsPtr = OnlineSubsystem->GetSessionInterface();
	SessionsPtr->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &ThisClass::OnSessionInviteAccepted);

	for (int32 i = 0; i < 5; i++)
	{
		PlayersMap.Add(i, FString("empty"));
	}
}

void UWOGGameInstance::Login()
{
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface();
		if (Identity)
		{
			UE_LOG(LogTemp, Error, TEXT("Yes Identity"));

			FOnlineAccountCredentials Credentials;
			Credentials.Id = nullptr;
			Credentials.Token = nullptr;
			Credentials.Type = FString("persistentauth");


			Identity.Get()->OnLoginCompleteDelegates->AddUObject(this, &ThisClass::OnLoginComplete);
			Identity.Get()->Login(0, Credentials);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No identity"));
		}
	}
}

void UWOGGameInstance::OnLoginComplete(int ControllerIndex, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorString)
{
	UE_LOG(LogTemp, Warning, TEXT("Logged in: %d"), bWasSuccessful);

	bIsLoggedIn = bWasSuccessful;

	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface();
		if (Identity)
		{
			Identity->ClearOnLoginCompleteDelegates(0, this);
		}
	}
	DestroySession();
}

void UWOGGameInstance::CreateSession()
{
	/*IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface();
	if (Identity && Identity->GetLoginStatus(0)==ELoginStatus::NotLoggedIn)
	{
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Red, FString("Not logged in: Cannot create session"));
		return;
	}
	
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
		if (SessionPtr)
		{
			FOnlineSessionSettings SessionSettings;
			SessionSettings.bIsDedicated = false;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bIsLANMatch = false;
			SessionSettings.NumPublicConnections = 5;
			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bAllowJoinViaPresence = true;
			SessionSettings.bAllowInvites = true;
			SessionSettings.bUsesPresence = true;
			SessionSettings.bUseLobbiesIfAvailable = true;
			SessionSettings.Set(SEARCH_KEYWORDS, FString("WOG_dev_lobby"), EOnlineDataAdvertisementType::ViaOnlineService);

			SessionPtr.Get()->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreateSessionComplete);
			SessionPtr.Get()->CreateSession(0, SESSION_NAME, SessionSettings);
			UE_LOG(LogTemp, Display, TEXT("Attempt to create Session"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid SessionPtr when creating session"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Online Subsystem"));
	}
	*/
	
}

void UWOGGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Success %d"), bSuccessful);

	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
		if (SessionPtr)
		{
			SessionPtr.Get()->ClearOnCreateSessionCompleteDelegates(this);
			GetWorld()->ServerTravel(FString("/Game/Maps/Lobby?listen"), true);
			UE_LOG(LogTemp, Display, TEXT("Session Created, ServerTravel() called"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid SessionPtr on OnCreateSessionComplete delegate"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Online Subsystem"));
	}
}

void UWOGGameInstance::DestroySession()
{
	if (!bIsLoggedIn) return;
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
		if (SessionPtr)
		{
			SessionPtr.Get()->OnDestroySessionCompleteDelegates.AddUObject(this, &ThisClass::OnDestroySessionComplete);
			bool DestroyedSession = SessionPtr.Get()->DestroySession(SESSION_NAME);
		}
	}
}

void UWOGGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccessful)
{
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
		if (SessionPtr)
		{
			SessionPtr->ClearOnDestroySessionCompleteDelegates(this);
		}
	}
}

void UWOGGameInstance::GetFriends()
{
	if (!OnlineSubsystem)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString("OnlineSubsystem not valid"));
		return;
	}
	IOnlineExternalUIPtr ExternalUIPtr = OnlineSubsystem->GetExternalUIInterface();
	if (ExternalUIPtr)
	{
		ExternalUIPtr->ShowInviteUI(0, SESSION_NAME);
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, FString("Friend UI coming up"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString("Friend UI NOT coming up"));
	}
	/*if (!bIsLoggedIn)
	{
		UE_LOG(LogTemp, Error, TEXT("Not logged in: Cannot create session"));
		return;
	}
	if (OnlineSubsystem)
	{
		IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface();
		if (FriendsPtr.IsValid())
		{
			FriendsPtr.Get()->ReadFriendsList(0, FString(), FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnGetFriendsComplete));
		}
	}*/
}

void UWOGGameInstance::OnGetFriendsComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (!OnlineSubsystem) return;

	IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface();

	if (bWasSuccessful && FriendsPtr.IsValid())
	{
		int8 ArrayIndex = -1;
		TArray<TSharedRef<FOnlineFriend>> FriendsList;

		if (FriendsPtr.Get()->GetFriendsList(0, ListName, FriendsList))
		{
			for (TSharedRef <FOnlineFriend> Friend : FriendsList)
			{
				++ArrayIndex;
				FFriendItem FriendInfo;
				FriendInfo.FriendName =  Friend->GetDisplayName();
				FriendInfo.ServerArrayIndex = ArrayIndex;
				FriendInfo.bIsOnline = Friend->GetPresence().bIsOnline;
				FriendInfo.bIsPlayingThisGame = Friend->GetPresence().bIsPlayingThisGame;
				FriendInfo.UniqueID = Friend->GetUserId()->ToString();
				
				FriendFoundDelegate.Broadcast(FriendInfo);
			}
		}
	}
}

bool UWOGGameInstance::SendInvite(FString UniqueID)
{
	if (!OnlineSubsystem) return false;

	IOnlineSessionPtr SessionsPtr = OnlineSubsystem->GetSessionInterface();
	IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface();

	if (!FriendsPtr || !SessionsPtr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, FString(TEXT("Something went wrong")));
		return false;
	}
	TArray< TSharedRef<FOnlineFriend> > OutFriends;

	FriendsPtr->GetFriendsList(0,FString(), OutFriends);
	const FUniqueNetId& PlayerNetID = *(GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId().Get());

	for (auto Friend : OutFriends)
	{
		if (Friend->GetUserId()->ToString() == UniqueID)
		{
			bool Success = SessionsPtr->SendSessionInviteToFriend(PlayerNetID, SESSION_NAME, Friend->GetUserId().Get());
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Orange, FString::Printf(TEXT("Invitation Sent to: %s"), *Friend->GetUserId().Get().ToString()));
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Orange, FString::Printf(TEXT("Invitation Sent successfully: %d"), Success));

			return true;
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, FString(TEXT("Something went wrong")));
	return false;
}

void UWOGGameInstance::OnSessionInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	JoinFriendServer(InviteResult);
}

void UWOGGameInstance::FindSessions()
{
	if (!bIsLoggedIn)
	{
		return;
	}
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
		if (SessionPtr)
		{
			SearchSettings = MakeShareable(new FOnlineSessionSearch());

			SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("WOG_dev_lobby"), EOnlineComparisonOp::Equals);
			SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
			SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
			SearchSettings->MaxSearchResults = 10000;
			SearchSettings->bIsLanQuery = false;

			SessionPtr.Get()->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsComplete);
			SessionPtr.Get()->FindSessions(0, SearchSettings.ToSharedRef());
		}
	}

}

void UWOGGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!OnlineSubsystem) return;

	IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();

	UE_LOG(LogTemp, Warning, TEXT("Success: %d"), bWasSuccessful);

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Amount sessions found: %d"), SearchSettings->SearchResults.Num());
		TArray<FOnlineSessionSearchResult> SearchResults = SearchSettings->SearchResults;
		int8 ArrayIndex = -1;

		if (SessionPtr)
		{
			if (SearchResults.Num())
			{
				for (FOnlineSessionSearchResult Result : SearchResults)
				{
					++ArrayIndex;
					if (!Result.IsValid()) continue;

					FServerItem ServerInfo;
					ServerInfo.HostName = Result.Session.OwningUserName;
					ServerInfo.NumMaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
					ServerInfo.NumCurrentPlayers = (Result.Session.SessionSettings.NumPublicConnections) - (Result.Session.NumOpenPublicConnections);
					ServerInfo.ServerArrayIndex = ArrayIndex;
					ServerInfo.SetCurrentPlayers();

					ServerFoundDelegate.Broadcast(ServerInfo);
				}
			}
		}
	}
	if (SessionPtr)
	{
		SessionPtr.Get()->ClearOnFindSessionsCompleteDelegates(this);
	}
}

void UWOGGameInstance::JoinServer(int32 ServerIndex)
{
	if (!OnlineSubsystem || !SearchSettings) return;
	IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
	if (!SessionPtr.IsValid()) return;
	TArray<FOnlineSessionSearchResult> SearchResults = SearchSettings->SearchResults;

	if (SearchResults.Num())
	{
		GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Cyan, FString("SearchResults not empty"));
		SessionPtr.Get()->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);
		SessionPtr.Get()->JoinSession(0, SESSION_NAME, SearchSettings->SearchResults[ServerIndex]);
		GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Cyan, FString::Printf(TEXT("Joining SearchResults at index: %d"), ServerIndex));
	}
}

void UWOGGameInstance::JoinFriendServer(const FOnlineSessionSearchResult& InviteResult)
{
	if (!OnlineSubsystem) return;
	IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
	if (InviteResult.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Cyan, FString("Joining friend"));
		SessionPtr.Get()->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);
		SessionPtr.Get()->JoinSession(0, SESSION_NAME, InviteResult);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Red, FString("Invite result not valid"));
	}
}

void UWOGGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSubsystem && Result != EOnJoinSessionCompleteResult::Success) return;
	IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
	APlayerController* PlayerController = GetFirstLocalPlayerController();

	if (SessionPtr)
	{
		FString ConnectionInfo = FString();
		SessionPtr.Get()->GetResolvedConnectString(SessionName, ConnectionInfo);
		GEngine->AddOnScreenDebugMessage(2, 2.f, FColor::Orange, FString::Printf(TEXT("Connect string: %s"), *ConnectionInfo));
		if (ConnectionInfo != FString())
		{
			if (PlayerController)
			{
				PlayerController->ClientTravel(ConnectionInfo, ETravelType::TRAVEL_Absolute);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Red, FString("Bad player controller"));
			}
		}
	}
}

void UWOGGameInstance::BPServerTravel(FString Address)
{
	GetWorld()->ServerTravel(Address, true);
}
