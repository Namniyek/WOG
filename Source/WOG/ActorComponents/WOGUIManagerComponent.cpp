// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/WOGUIManagerComponent.h"
#include "WOG.h"
#include "PlayerController/WOGPlayerController.h"
#include "Subsystems/WOGUIManagerSubsystem.h"

UWOGUIManagerComponent::UWOGUIManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
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
		UE_LOG(WOGLogUI, Display, TEXT("Removed ability %d"), AbilityID);
	}
}

void UWOGUIManagerComponent::Client_AddBarsWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->AddBarsWidget();
	}
}

void UWOGUIManagerComponent::Client_RemoveBarsWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->RemoveBarsWidget();
	}
}

void UWOGUIManagerComponent::Client_AddMinimapWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->AddMinimapWidget();
	}
}

void UWOGUIManagerComponent::Client_RemoveMinimapWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->RemoveMinimapWidget();
	}
}

void UWOGUIManagerComponent::Client_CollapseAbilitiesWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->CollapseAbilitiesWidget();
	}
}

void UWOGUIManagerComponent::Client_RestoreAbilitiesWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->RestoreAbilitiesWidget();
	}
}

void UWOGUIManagerComponent::Client_CollapseTODWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->CollapseTODWidget();
	}
}

void UWOGUIManagerComponent::Client_RestoreTODWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->RestoreTODWidget();
	}
}

void UWOGUIManagerComponent::Client_CollapseObjectiveWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->CollapseObjectiveWidget();
	}
}

void UWOGUIManagerComponent::Client_RestoreObjectiveWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->RestoreObjectiveWidget();
	}
}

void UWOGUIManagerComponent::Client_AddAvailableResourceWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->AddAvailableResourcesWidget();
	}
}

void UWOGUIManagerComponent::Client_RemoveAvailableResourceWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->RemoveAvailableResourcesWidget();
	}
}

void UWOGUIManagerComponent::Client_UpdateVendorWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->UpdateVendorWidgetAfterTransaction();
	}
}

void UWOGUIManagerComponent::Client_CreateGenericWarningWidget_Implementation(const FString& WarningString)
{
	if (UIManager)
	{
		UIManager->CreateGenericWarningWidget(WarningString);
	}
}

void UWOGUIManagerComponent::Client_AddHuntWidget_Implementation(AWOGHuntEnemy* HuntEnemy)
{
	if (UIManager)
	{
		UIManager->AddHuntWidget(HuntEnemy);
	}
}

void UWOGUIManagerComponent::Client_RemoveHuntWidget_Implementation()
{
	if (UIManager)
	{
		UIManager->RemoveHuntWidget();
	}
}
