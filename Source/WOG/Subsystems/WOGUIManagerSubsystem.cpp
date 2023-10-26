// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/WOGUIManagerSubsystem.h"
#include "WOG.h"
#include "Data/TODEnum.h"
#include "UI/Vendors/WOGVendorBaseWidget.h"
#include "Interfaces/VendorInterface.h"
#include "PlayerController/WOGPlayerController.h"
#include "UI/WOGMatchHUD.h"
#include "UI/WOG_HUD.h"
#include "Engine/LocalPlayer.h"
#include "UI/WOGWarningWidget.h"
#include "Components/VerticalBox.h"
#include "Characters/WOGBaseCharacter.h"
#include "UI/WOGCharacterWidgetContainer.h"
#include "Components/SizeBox.h"
#include "UI/WOGRoundProgressBar.h"
#include "UI/WOGScreenDamage.h"
#include "UI/WOGHoldProgressBar.h"
#include "UI/WOGRavenMarkerWidget.h"
#include "UI/WOGAbilityWidget.h"
#include "UI/WOGAbilityContainerWidget.h"
#include "UI/WOGMatchHUD.h"
#include "UI/WOGObjectiveWidget.h"
#include "UI/MainAnnouncementWidget.h"
#include "UI/EndgameWidget.h"

void UWOGUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(WOGLogUI, Display, TEXT("UI Manager Initialized"));
}

void UWOGUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(WOGLogUI, Display, TEXT("UI Manager Deinitialized"));
}

void UWOGUIManagerSubsystem::InitVariables()
{
	OwnerPC = Cast<AWOGPlayerController>(GetLocalPlayer()->GetPlayerController(GetLocalPlayer()->GetWorld()));
	if (OwnerPC)
	{
		UE_LOG(WOGLogUI, Display, TEXT("OwnerPC in UIManagerSubsystem: %s"), *GetNameSafe(OwnerPC));

		MatchHUD = Cast<AWOGMatchHUD>(OwnerPC->GetHUD());
		if (MatchHUD)
		{
			UE_LOG(WOGLogUI, Display, TEXT("MatchHUD in UIManagerSubsystem: %s"), *GetNameSafe(MatchHUD));
		}
		else
		{
			UE_LOG(WOGLogUI, Error, TEXT("invalid MatchHUD in UIManagerSubsystem"));
		}
	}
	else
	{
		UE_LOG(WOGLogUI, Error, TEXT("invalid OwnerPC in UIManagerSubsystem"));
	}
}

void UWOGUIManagerSubsystem::SetTODString(ETimeOfDay CurrentTOD, FString& StringMain, FString& StringSec)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget) return;

	switch (CurrentTOD)
	{
	case ETimeOfDay::TOD_Dusk1:
		StringMain = FString("Dusk of the first day");
		StringSec = OwnerPC->GetIsAttacker() ? FString("Destroy the Village") : FString("Defend the Village!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(OwnerPC->GetIsAttacker() ? EObjectiveText::EOT_DestroyTheVillage : EObjectiveText::EOT_DefendTheVillage);
		break;

	case ETimeOfDay::TOD_Dawn2:
		StringMain = FString("Dawn of the second day");
		StringSec = FString("Rest and prepare for the night!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(EObjectiveText::EOT_PrepareForTheNight);
		break;

	case ETimeOfDay::TOD_Dusk2:
		StringMain = FString("Dusk of the second day");
		StringSec = OwnerPC->GetIsAttacker() ? FString("Destroy the Village") : FString("Defend the Village!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(OwnerPC->GetIsAttacker() ? EObjectiveText::EOT_DestroyTheVillage : EObjectiveText::EOT_DefendTheVillage);
		break;

	case ETimeOfDay::TOD_Dawn3:
		StringMain = FString("Dawn of the final day");
		StringSec = FString("Rest and prepare for the night!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(EObjectiveText::EOT_PrepareForTheNight);
		break;

	case ETimeOfDay::TOD_Dusk3:
		StringMain = FString("Dusk of the final day");
		StringSec = OwnerPC->GetIsAttacker() ? FString("Destroy the Village") : FString("Defend the Village!");
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetObjectiveText(OwnerPC->GetIsAttacker() ? EObjectiveText::EOT_DestroyTheVillage : EObjectiveText::EOT_DefendTheVillage);
		break;

	case ETimeOfDay::TOD_Dawn4:
		StringMain = FString("Game Over!");
		StringSec = FString("");
		MatchHUD->HUDWidget->GetObjectiveWidget()->RemoveFromParent();
		break;

	default:
		return;
	}
}

void UWOGUIManagerSubsystem::UpdateAvailableResourceWidget()
{
	if (!VendorWidget) return;

	TObjectPtr<IVendorInterface> VendorInterface = Cast<IVendorInterface>(VendorWidget);
	if (VendorInterface)
	{
		VendorInterface->Execute_UpdateAvailableResourceWidget(VendorWidget);
	}
}

void UWOGUIManagerSubsystem::UpdateVendorWidgetAfterTransaction()
{
	if (!VendorWidget) return;

	TObjectPtr<IVendorInterface> VendorInterface = Cast<IVendorInterface>(VendorWidget);
	if (VendorInterface)
	{
		VendorInterface->Execute_UpdateVendorInventoryWidget(VendorWidget);
	}
}

void UWOGUIManagerSubsystem::CreateResourceWarningWidget(const FString& Attribute)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->AttributeWarningClass)) return;

	TObjectPtr<UWOGWarningWidget> WarningWidget = Cast<UWOGWarningWidget>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->AttributeWarningClass));
	if (WarningWidget)
	{
		WarningWidget->SetWarningText(Attribute);

		if (MatchHUD->HUDWidget->GetWarningContainer())
		{
			MatchHUD->HUDWidget->GetWarningContainer()->ClearChildren();
			MatchHUD->HUDWidget->GetWarningContainer()->AddChild(WarningWidget);
		}
	}
}

void UWOGUIManagerSubsystem::CreateGenericWarningWidget(const FString& WarningString)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->GenericWarningClass)) return;

	TObjectPtr<UWOGWarningWidget> WarningWidget = Cast<UWOGWarningWidget>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->GenericWarningClass));
	if (WarningWidget)
	{
		WarningWidget->SetWarningText(WarningString);

		if (MatchHUD->HUDWidget->GetWarningContainer())
		{
			MatchHUD->HUDWidget->GetWarningContainer()->ClearChildren();
			MatchHUD->HUDWidget->GetWarningContainer()->AddChild(WarningWidget);
		}
	}
}

void UWOGUIManagerSubsystem::AddStaminaWidget()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->StaminaBarClass)) return;

	TObjectPtr<AWOGBaseCharacter> BaseCharacter = Cast<AWOGBaseCharacter>(OwnerPC->GetPawn());
	if (!BaseCharacter || !BaseCharacter->GetStaminaWidgetContainer() || !BaseCharacter->GetStaminaWidgetContainer()->GetContainer()) return;
	if (BaseCharacter->GetStaminaWidgetContainer()->GetContainer()->HasAnyChildren()) return;

	TObjectPtr<UWOGRoundProgressBar> StaminaBar = Cast<UWOGRoundProgressBar>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->StaminaBarClass));
	if (StaminaBar)
	{
		BaseCharacter->GetStaminaWidgetContainer()->GetContainer()->AddChild(StaminaBar);
	}
}

void UWOGUIManagerSubsystem::AddScreenDamageWidget(const int32& DamageThreshold)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->ScreenDamageWidgetClass)) return;

	TObjectPtr<UWOGScreenDamage> ScreenDamageWidget = Cast<UWOGScreenDamage>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->ScreenDamageWidgetClass));
	if (ScreenDamageWidget)
	{
		ScreenDamageWidget->SetRadiusValue(DamageThreshold);
		ScreenDamageWidget->AddToViewport();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid ScreenDamageWidget"));
	}
}

void UWOGUIManagerSubsystem::AddHoldProgressBar()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->HoldProgressBarWidgetClass)) return;

	HoldProgressBarWidget = Cast<UWOGHoldProgressBar>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->HoldProgressBarWidgetClass));
	if (HoldProgressBarWidget && MatchHUD->HUDWidget->GetHoldBarContainer())
	{
		MatchHUD->HUDWidget->GetHoldBarContainer()->AddChild(HoldProgressBarWidget);
	}
	else
	{
		UE_LOG(WOGLogUI, Error, TEXT("Invalid HoldProgressBarWidget"));
	}
}

void UWOGUIManagerSubsystem::RemoveHoldProgressBar()
{
	if (HoldProgressBarWidget)
	{
		HoldProgressBarWidget->RemoveFromParent();
	}
}

void UWOGUIManagerSubsystem::AddRavenMarkerWidget(const int32& Amount)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->RavenMarkerWidgetClass)) return;

	RavenMarkerWidget = Cast<UWOGRavenMarkerWidget>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->RavenMarkerWidgetClass));
	if (RavenMarkerWidget && MatchHUD->HUDWidget->GetHoldBarContainer())
	{
		MatchHUD->HUDWidget->GetHoldBarContainer()->AddChild(RavenMarkerWidget);
		RavenMarkerWidget->SetAmountAvailableMarkers(Amount);
	}
	else
	{
		UE_LOG(WOGLogUI, Error, TEXT("Invalid RavenMarkerWidget"));
	}
}

void UWOGUIManagerSubsystem::RemoveRavenMarkerWidget()
{
	if (RavenMarkerWidget)
	{
		RavenMarkerWidget->RemoveFromParent();
	}
}

void UWOGUIManagerSubsystem::AddAbilityWidget(const int32& AbilityID, TSubclassOf<UUserWidget> Class, UTexture2D* Icon, const float& Cooldown, const FGameplayTag& Tag)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget) return;

	TObjectPtr<UWOGAbilityWidget> AbilityWidget = Cast<UWOGAbilityWidget>(CreateWidget<UUserWidget>(OwnerPC, Class));
	if (!AbilityWidget) return;

	AbilityWidget->SetIconTexture(Icon);
	AbilityWidget->SetCooldownTag(Tag);
	AbilityWidget->SetCooldownTime(Cooldown);
	AbilityWidget->InitializeWidget();

	TObjectPtr<UWOGAbilityContainerWidget> Container = MatchHUD->HUDWidget->GetAbilityContainer();
	if (!Container) return;

	Container->AddChildAbility(AbilityID, AbilityWidget);
}

void UWOGUIManagerSubsystem::RemoveAbilityWidget(const int32& AbilityID)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget) return;
	TObjectPtr<UWOGAbilityContainerWidget> Container = MatchHUD->HUDWidget->GetAbilityContainer();
	if (!Container) return;

	Container->RemoveChildAbility(AbilityID);
}

void UWOGUIManagerSubsystem::AddAnnouncementWidget(ETimeOfDay NewTOD)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->AnnouncementClass) return;

	TOD = NewTOD;
	FString StringMain = FString();
	FString StringSec = FString();
	SetTODString(TOD, StringMain, StringSec);

	MatchHUD->AddAnnouncementWidget(StringMain, StringSec);
}

void UWOGUIManagerSubsystem::AddEndgameWidget()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD) return;

	MatchHUD->AddEndgameWidget();
}

void UWOGUIManagerSubsystem::ResetHUD()
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (MatchHUD)
	{
		MatchHUD->ResetHUDAfterRespawn();
	}
}