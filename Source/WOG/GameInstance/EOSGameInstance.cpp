// Fill out your copyright notice in the Description page of Project Settings.


#include "EOSGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"

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
			Credentials.Id = FString("127.0.0.1:8081");
			Credentials.Token = FString("WOG_dev");
			Credentials.Type = FString("developer");


			Identity.Get()->OnLoginCompleteDelegates->AddUObject(this, &ThisClass::OnLoginComplete);
			Identity.Get()->Login(0, Credentials);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No identity"));
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
			SessionPtr.Get()->CreateSession(0, FName("Test Session"), SessionSettings);
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
