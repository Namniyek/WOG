// Fill out your copyright notice in the Description page of Project Settings.


#include "WOGGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "GameFramework/PlayerController.h"
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

	if (!OnlineSubsystem) return;

	for (int32 i = 0; i < 5; i++)
	{
		PlayersMap.Add(i, FString("empty"));
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
