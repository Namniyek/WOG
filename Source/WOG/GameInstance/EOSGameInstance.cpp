// Fill out your copyright notice in the Description page of Project Settings.


#include "EOSGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

const FName SESSION_NAME = TEXT("Test Session");

UEOSGameInstance::UEOSGameInstance()
{
	bIsLoggedIn = false;

}

void UEOSGameInstance::Init()
{
	Super::Init();

	OnlineSubsystem = IOnlineSubsystem::Get();
	Login();
}

void UEOSGameInstance::Login()
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

void UEOSGameInstance::OnLoginComplete(int ControllerIndex, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorString)
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
}

void UEOSGameInstance::CreateSession()
{
	if (!bIsLoggedIn)
	{
		UE_LOG(LogTemp, Error, TEXT("Not logged in: Cannot create session"));
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
			SessionSettings.bUsesPresence = true;
			SessionSettings.bUseLobbiesIfAvailable = true;
			SessionSettings.Set(SEARCH_KEYWORDS, FString("WOG_dev_lobby"), EOnlineDataAdvertisementType::ViaOnlineService);

			SessionPtr.Get()->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreateSessionComplete);
			SessionPtr.Get()->CreateSession(0, SESSION_NAME, SessionSettings);
		}
	}
}

void UEOSGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Success %d"), bSuccessful);

	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
		if (SessionPtr)
		{
			SessionPtr.Get()->ClearOnCreateSessionCompleteDelegates(this);
			GetWorld()->ServerTravel(FString("/Game/Maps/Lobby?listen"), true);
		}
	}
}

void UEOSGameInstance::DestroySession()
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
			SessionPtr.Get()->OnDestroySessionCompleteDelegates.AddUObject(this, &ThisClass::OnDestroySessionComplete);
			SessionPtr.Get()->DestroySession(SESSION_NAME);
		}
	}
}

void UEOSGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccessful)
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

void UEOSGameInstance::GetAllFriends()
{
	if (!bIsLoggedIn)
	{
		UE_LOG(LogTemp, Error, TEXT("Not logged in: Cannot create session"));
		return;
	}
	if (OnlineSubsystem)
	{
		IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface();
		if (FriendsPtr.IsValid())
		{
			FriendsPtr.Get()->ReadFriendsList(0, FString(), FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnGetAllFriendsComplete));
		}
	}
}

void UEOSGameInstance::OnGetAllFriendsComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (OnlineSubsystem)
	{
		IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface();
		if (FriendsPtr.IsValid())
		{
			TArray<TSharedRef<FOnlineFriend>> FriendsList;
			if (FriendsPtr.Get()->GetFriendsList(0, ListName, FriendsList))
			{
				for (TSharedRef <FOnlineFriend> Friend : FriendsList)
				{
					FString FriendName = Friend.Get().GetRealName();
					UE_LOG(LogTemp, Warning, TEXT("FriendName is: %s"), *FriendName);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to get friends list"));
			}

			if (FriendsList.Num() == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("You are a loser with no friends"));
			}
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("Was successful ate getting friends: %d"), bWasSuccessful);
	//UE_LOG(LogTemp, Warning, TEXT("Was successful ate getting friends: %S"), *ErrorStr);
}

void UEOSGameInstance::ShowInviteUI()
{
	if (!bIsLoggedIn) return;

	if (OnlineSubsystem)
	{
		IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface();
		if (UIPtr)
		{
			UIPtr.Get()->ShowInviteUI(0, SESSION_NAME);
		}
	}
}

void UEOSGameInstance::ShowFriendsUI()
{
	if (!bIsLoggedIn) return;

	if (OnlineSubsystem)
	{
		IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface();
		if (UIPtr)
		{
			UIPtr.Get()->ShowFriendsUI(0);
		}
	}
}

void UEOSGameInstance::FindSessions()
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
			SearchSettings->MaxSearchResults = 10000;

			SessionPtr.Get()->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsComplete);
			SessionPtr.Get()->FindSessions(0, SearchSettings.ToSharedRef());
		}
	}

}

void UEOSGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
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

void UEOSGameInstance::JoinServer(int32 ServerIndex)
{
	if (!OnlineSubsystem || !SearchSettings) return;
	IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
	TArray<FOnlineSessionSearchResult> SearchResults = SearchSettings->SearchResults;

	if (SearchResults.Num())
	{
		GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Cyan, FString("SearchResults not empty"));
		SessionPtr.Get()->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);
		SessionPtr.Get()->JoinSession(0, SESSION_NAME, SearchSettings->SearchResults[ServerIndex]);
		GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Cyan, FString::Printf(TEXT("Joining SearchResults at index: %d"), ServerIndex));
	}
}

void UEOSGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
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
				GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Orange, FString("ClientTravel"));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Orange, FString("Bad player controller"));
			}
		}
	}
}