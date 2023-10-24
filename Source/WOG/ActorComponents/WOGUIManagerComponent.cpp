// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/WOGUIManagerComponent.h"
#include "WOG.h"
#include "PlayerController/WOGPlayerController.h"
#include "Subsystems/WOGUIManagerSubsystem.h"

// Sets default values for this component's properties
UWOGUIManagerComponent::UWOGUIManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicated(true);
}


// Called when the game starts
void UWOGUIManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPC = Cast<AWOGPlayerController>(GetOwner());
	if (!OwnerPC)
	{
		UE_LOG(WOGLogUI, Error, TEXT("OwnerPC from UIManagerComponent invalid"));
		return;
	}

	UIManager = ULocalPlayer::GetSubsystem<UWOGUIManagerSubsystem>(OwnerPC->GetLocalPlayer());
	if (!UIManager)
	{
		UE_LOG(WOGLogUI, Error, TEXT("UIManagerSubsystem from UIManagerComponent invalid"));
		return;
	}
}

void UWOGUIManagerComponent::Client_CreateWarningWidget_Implementation(const FString& Attribute)
{
	if (UIManager)
	{
		UIManager->CreateWarningWidget(Attribute);
		UE_LOG(WOGLogUI, Display, TEXT("CreateWarningWidget() called from WOGUIManagerComponent class"));
	}
}

