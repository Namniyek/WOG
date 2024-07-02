// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/WOGEpicOnlineServicesSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"

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
