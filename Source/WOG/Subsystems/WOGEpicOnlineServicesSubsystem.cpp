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
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

	LoginDelegateHandle =Identity->AddOnLoginCompleteDelegate_Handle(
		0, FOnLoginComplete::FDelegate::CreateUObject(this, &ThisClass::HandleLoginComplete));

	if(!Identity->AutoLogin(0))
	{
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Red, FString("Failed to AutoLogin"));
		Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginDelegateHandle);
		this->LoginDelegateHandle.Reset();
	}
}

void UWOGEpicOnlineServicesSubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	// TODO Check bWasSuccessful to see if the login was completed.
	if(!bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Red, FString("AutoLogin succeeded, but failed to Login"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(0, 6.f, FColor::Green, FString("Login successful"));
		UGameplayStatics::OpenLevel(this, FName("StartUp"), true);
	}

	// Deregister the event handler.
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginDelegateHandle);
	this->LoginDelegateHandle.Reset();
}
