// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/WOGEpicOnlineServicesSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "RedpointInterfaces/OnlineLobbyInterface.h"

void UWOGEpicOnlineServicesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UWOGEpicOnlineServicesSubsystem::Login()
{
	const IOnlineSubsystem *Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

	if(Identity && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Orange, FString("Already Logged in"));
		return;
	}

	LoginDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(
		0, FOnLoginComplete::FDelegate::CreateUObject(this, &ThisClass::HandleLoginComplete));

	if(!Identity->AutoLogin(0))
	{
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Red, FString("Failed to AutoLogin"));
		Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginDelegateHandle);
		LoginDelegateHandle.Reset();
	}
}

void UWOGEpicOnlineServicesSubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if(bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Green, FString("Login successful"));
		OnLoginCompleteDelegate.Broadcast(bWasSuccessful);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Red, FString("AutoLogin succeeded, but failed to Login"));
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

void UWOGEpicOnlineServicesSubsystem::CreateLobby()
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
	LobbyTransaction->SetMetadata.Add(TEXT("EOSVoiceChat_Enabled"), false);

	// To allow clients connecting to the listen server to join the lobby based on just the ID, we need
	// to set it to public.
	LobbyTransaction->Public = true;

	// Here you can adjust the capacity of the lobby. 
	LobbyTransaction->Capacity = 5;

	// Setting a lobby as locked prevents players from joining it.
	LobbyTransaction->Locked = false;

	//Setting the custom parameter to filter the correct MatchTypes
	LobbyTransaction->SetMetadata.Add(TEXT("MatchType"), FVariantData(TEXT("WOG_default")));

	/*
	 * Create the Lobby
	 */
	if (!LobbyInterface->CreateLobby(*LocalUserId.Get(), *LobbyTransaction,
	FOnLobbyCreateOrConnectComplete::CreateLambda([&](
		const FOnlineError & Error,
		const FUniqueNetId & UserId,
		const TSharedPtr<FOnlineLobby> & CreatedLobby)
	{
		if (Error.WasSuccessful())
		{
			// The lobby was created successfully and is now in CreatedLobby.
			GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Green, FString("Lobby Created"));
			LobbyIdString = CreatedLobby->Id->ToString();
			GetWorld()->ServerTravel(FString("/Game/Maps/Lobby?listen"), true);
			// You'll need to store IdStr somewhere, as that is what needs to be sent to connecting clients.
		}
		else
		{
			// Lobby could not be created.
			GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Red, FString("Failed to Create Lobby"));
		}
	})))
	{
		// Call failed to start.
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Red, FString("Call to Create Lobby failed to start"));
	}
}

void UWOGEpicOnlineServicesSubsystem::GetLobbyMembers()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	TSharedPtr<IOnlineLobby> Lobby = Online::GetLobbyInterface(Subsystem);
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

	TSharedPtr<const FUniqueNetId> LocalUserId = Identity->GetUniquePlayerId(0);
	TSharedPtr<FOnlineLobbyId> CurrentLobbyID = Lobby->ParseSerializedLobbyId(LobbyIdString);
	
	TArray<TSharedRef<const FUniqueNetId>> MemberIds;
	int32 MemberCount = 0;
	Lobby->GetMemberCount(*LocalUserId.Get(), *CurrentLobbyID.Get(), MemberCount);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange, FString("Lobby members count fetched: ") + FString::FromInt(MemberCount));

	for (int32 i = 0; i < MemberCount; i++) 
	{
		TSharedPtr<const FUniqueNetId> MemberId;
		Lobby->GetMemberUserId(*LocalUserId, *CurrentLobbyID, i, MemberId);
		if (MemberId.IsValid()) 
		{
			MemberIds.AddUnique(MemberId.ToSharedRef());
		}
	}

	if(MemberIds.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Red, FString("MembersID Array is empty"));
		return;
	}
	
	for(auto Member : MemberIds)
	{
		auto Account = Identity->GetUserAccount(*Member);
		FString ProductUserID = FString();
		Account->GetUserAttribute(FString("productUserId"), ProductUserID);
		
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Purple, FString("Lobby member fetched: ") + Member->ToString());
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Purple, FString("Lobby member name: ") + Account->GetDisplayName());
		GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Purple, FString("ProductUserID: ") + ProductUserID);
		UE_LOG(LogTemp, Display, TEXT("ProductUserID: %s"), *ProductUserID);
	}
}
