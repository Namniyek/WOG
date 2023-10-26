// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/WOGUIManagerComponent.h"
#include "WOG.h"
#include "PlayerController/WOGPlayerController.h"
#include "Subsystems/WOGUIManagerSubsystem.h"

UWOGUIManagerComponent::UWOGUIManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicated(true);
}

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

void UWOGUIManagerComponent::Client_ResetHUD_Implementation()
{
	if (UIManager)
	{
		UIManager->ResetHUD();
	}
}

void UWOGUIManagerComponent::Client_AddEndgameWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->AddEndgameWidget();
	}
}

void UWOGUIManagerComponent::Client_AddAnnouncementWidget_Implementation(ETimeOfDay NewTOD)
{
	if (UIManager)
	{
		UIManager->AddAnnouncementWidget(NewTOD);
	}
}

void UWOGUIManagerComponent::Client_AddAbilityWidget_Implementation(const int32& AbilityID, TSubclassOf<UUserWidget> Class, UTexture2D* Icon, const float& Cooldown, const FGameplayTag& Tag)
{
	if (UIManager)
	{
		UIManager->AddAbilityWidget(AbilityID, Class, Icon, Cooldown, Tag);
	}
}

void UWOGUIManagerComponent::Client_RemoveAbilityWidget_Implementation(const int32& AbilityID)
{
	if (UIManager)
	{
		UIManager->RemoveAbilityWidget(AbilityID);
	}
}

