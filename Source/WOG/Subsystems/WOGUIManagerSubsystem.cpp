// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/WOGUIManagerSubsystem.h"
#include "WOG.h"
#include "Data/TODEnum.h"
#include "UI/Vendors/WOGVendorBaseWidget.h"
#include "Interfaces/InventoryInterface.h"
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
#include "UI/WOGAvailableResourceWidget.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "UI/Stash/WOGStashWidget.h"

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
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
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

	IInventoryInterface* Interface = Cast<IInventoryInterface>(VendorWidget);
	if (Interface)
	{
		Interface->Execute_UpdateAvailableResourceWidget(VendorWidget);
	}
}

void UWOGUIManagerSubsystem::UpdateVendorWidgetAfterTransaction()
{
	if (!VendorWidget) return;
	VendorWidget->RefreshVendorItems();
}

void UWOGUIManagerSubsystem::CreateResourceWarningWidget(const FString& Attribute)
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
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
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
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
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->StaminaBarClass)) return;

	TObjectPtr<ABasePlayerCharacter> BaseCharacter = Cast<ABasePlayerCharacter>(OwnerPC->GetPawn());
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
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
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
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->HoldProgressBarWidgetClass)) return;

	HoldProgressBarWidget = Cast<UWOGHoldProgressBar>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->HoldProgressBarWidgetClass));
	if (HoldProgressBarWidget && MatchHUD->HUDWidget->GetHoldBarContainer())
	{
		MatchHUD->HUDWidget->GetHoldBarContainer()->ClearChildren();
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
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->RavenMarkerWidgetClass)) return;

	RavenMarkerWidget = Cast<UWOGRavenMarkerWidget>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->RavenMarkerWidgetClass));
	if (RavenMarkerWidget && MatchHUD->HUDWidget->GetHoldBarContainer())
	{
		MatchHUD->HUDWidget->GetHoldBarContainer()->ClearChildren();
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
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
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
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget) return;
	TObjectPtr<UWOGAbilityContainerWidget> Container = MatchHUD->HUDWidget->GetAbilityContainer();
	if (!Container) return;

	Container->RemoveChildAbility(AbilityID);
	UE_LOG(WOGLogUI, Display, TEXT("Removed ability %d, from Subsystem"), AbilityID);
}

void UWOGUIManagerSubsystem::AddAnnouncementWidget(ETimeOfDay NewTOD)
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->AnnouncementClass) return;

	TOD = NewTOD;
	FString StringMain = FString();
	FString StringSec = FString();
	SetTODString(TOD, StringMain, StringSec);

	MatchHUD->AddAnnouncementWidget(StringMain, StringSec);
}

void UWOGUIManagerSubsystem::AddEndgameWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD) return;

	MatchHUD->AddEndgameWidget();
}

void UWOGUIManagerSubsystem::ResetHUD()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (MatchHUD)
	{
		MatchHUD->ResetHUDAfterRespawn();
	}

	AddBarsWidget();
	AddMinimapWidget();
	RestoreAbilitiesWidget();
	AddAvailableResourcesWidget();
	RestoreObjectiveWidget();
	RestoreTODWidget();
}

void UWOGUIManagerSubsystem::AddBarsWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->BarsWidgetClass)) return;

	BarsWidget = CreateWidget<UUserWidget>(OwnerPC, MatchHUD->BarsWidgetClass);
	if (BarsWidget && MatchHUD->HUDWidget->GetBarsContainer())
	{
		MatchHUD->HUDWidget->GetBarsContainer()->ClearChildren();
		MatchHUD->HUDWidget->GetBarsContainer()->AddChild(BarsWidget);
	}
}

void UWOGUIManagerSubsystem::RemoveBarsWidget()
{
	if (IsValid(BarsWidget))
	{
		BarsWidget->RemoveFromParent();
		UE_LOG(WOGLogUI, Warning, TEXT("BarsWidget Removed"));
	}
	else
	{
		UE_LOG(WOGLogUI, Error, TEXT("BarsWidget invalid"));
	}
}

void UWOGUIManagerSubsystem::AddMinimapWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->MinimapWidgetClass)) return;

	MinimapWidget = CreateWidget<UUserWidget>(OwnerPC, MatchHUD->MinimapWidgetClass);
	if (MinimapWidget && MatchHUD->HUDWidget->GetMinimapContainer())
	{
		//MatchHUD->HUDWidget->GetMinimapContainer()->ClearChildren();
		MatchHUD->HUDWidget->GetMinimapContainer()->AddChild(MinimapWidget);
	}
}

void UWOGUIManagerSubsystem::RemoveMinimapWidget()
{
	if (IsValid(MinimapWidget))
	{
		MinimapWidget->RemoveFromParent();
	}
}

void UWOGUIManagerSubsystem::CollapseAbilitiesWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !MatchHUD->HUDWidget->GetAbilitiesContainer()) return;

	AbilityContainerWidget = AbilityContainerWidget == nullptr ? (TObjectPtr<UUserWidget>) Cast<UUserWidget>(MatchHUD->HUDWidget->GetAbilitiesContainer()->GetChildAt(0)) : AbilityContainerWidget;
	if (IsValid(AbilityContainerWidget) && AbilityContainerWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		AbilityContainerWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UWOGUIManagerSubsystem::RestoreAbilitiesWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !MatchHUD->HUDWidget->GetAbilitiesContainer()) return;

	AbilityContainerWidget = AbilityContainerWidget == nullptr ? (TObjectPtr<UUserWidget>) Cast<UUserWidget>(MatchHUD->HUDWidget->GetAbilitiesContainer()->GetChildAt(0)) : AbilityContainerWidget;
	if (IsValid(AbilityContainerWidget) && AbilityContainerWidget->GetVisibility() == ESlateVisibility::Hidden)
	{
		AbilityContainerWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UWOGUIManagerSubsystem::CollapseTODWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !MatchHUD->HUDWidget->GetTODContainer()) return;

	TODWidget = TODWidget == nullptr ? (TObjectPtr<UUserWidget>) Cast<UUserWidget>(MatchHUD->HUDWidget->GetTODContainer()->GetChildAt(0)) : TODWidget;
	if (TODWidget && TODWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		TODWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UWOGUIManagerSubsystem::RestoreTODWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !MatchHUD->HUDWidget->GetTODContainer()) return;

	TODWidget = TODWidget == nullptr ? (TObjectPtr<UUserWidget>) Cast<UUserWidget>(MatchHUD->HUDWidget->GetTODContainer()->GetChildAt(0)) : TODWidget;
	if (TODWidget && TODWidget->GetVisibility() == ESlateVisibility::Hidden)
	{
		TODWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UWOGUIManagerSubsystem::CollapseObjectiveWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !MatchHUD->HUDWidget->GetObjectiveWidget()) return;

	if (MatchHUD->HUDWidget->GetObjectiveWidget()->GetVisibility() == ESlateVisibility::Visible)
	{
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UWOGUIManagerSubsystem::RestoreObjectiveWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !MatchHUD->HUDWidget->GetObjectiveWidget()) return;

	if (MatchHUD->HUDWidget->GetObjectiveWidget()->GetVisibility() == ESlateVisibility::Hidden)
	{
		MatchHUD->HUDWidget->GetObjectiveWidget()->SetVisibility(ESlateVisibility::Visible);
	}
}

void UWOGUIManagerSubsystem::AddAvailableResourcesWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->AvailableResourceWidgetClass)) return;

	AvailableResourcesWidget = CreateWidget<UUserWidget>(OwnerPC, MatchHUD->AvailableResourceWidgetClass);
	if (IsValid(AvailableResourcesWidget) && MatchHUD->HUDWidget->GetAvailableResourceContainer())
	{
		MatchHUD->HUDWidget->GetAvailableResourceContainer()->ClearChildren();
		MatchHUD->HUDWidget->GetAvailableResourceContainer()->AddChild(AvailableResourcesWidget);
	}
}

void UWOGUIManagerSubsystem::RemoveAvailableResourcesWidget()
{
	if (IsValid(AvailableResourcesWidget))
	{
		AvailableResourcesWidget->RemoveFromParent();
	}
}

void UWOGUIManagerSubsystem::AddVendorWidget(ABasePlayerCharacter* Buyer, AWOGVendor* Vendor)
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->VendorWidgetClass)) return;

	VendorWidget = Cast<UWOGVendorBaseWidget>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->VendorWidgetClass));
	if (VendorWidget)
	{
		VendorWidget->SetPlayerActor(Buyer);
		VendorWidget->SetVendorActor(Vendor);
		VendorWidget->AddToViewport();
	}
}

void UWOGUIManagerSubsystem::RemoveVendorWidget()
{
	if (VendorWidget)
	{
		VendorWidget->RemoveFromParent();
		UE_LOG(WOGLogUI, Display, TEXT("VendorWidget removed from subsystem"));
	}
	else
	{
		UE_LOG(WOGLogUI, Error, TEXT("VendorWidget NOT removed from subsystem"));
	}
}

void UWOGUIManagerSubsystem::AddStashWidget(ABasePlayerCharacter* User, AWOGStashBase* Stash)
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->StashWidgetClass)) return;

	StashWidget = Cast<UWOGStashWidget>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->StashWidgetClass));
	if (StashWidget)
	{
		StashWidget->SetStashActor(Stash);
		StashWidget->SetPlayerActor(User);
		StashWidget->AddToViewport();
	}
}

void UWOGUIManagerSubsystem::RemoveStashWidget()
{
	if (StashWidget)
	{
		StashWidget->RemoveFromParent();
		UE_LOG(WOGLogUI, Display, TEXT("StashWidget removed from subsystem"));
	}
	else
	{
		UE_LOG(WOGLogUI, Error, TEXT("StashWidget NOT removed from subsystem"));
	}
}

void UWOGUIManagerSubsystem::AddSquadOrderWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->SquadOrderWidgetClass)) return;

	SquadOrderWidget = CreateWidget<UUserWidget>(OwnerPC, MatchHUD->SquadOrderWidgetClass);
	if (SquadOrderWidget && MatchHUD->HUDWidget->GetSquadOrderContainer())
	{
		CollapseAbilitiesWidget();
		MatchHUD->HUDWidget->GetSquadOrderContainer()->ClearChildren();
		MatchHUD->HUDWidget->GetSquadOrderContainer()->AddChild(SquadOrderWidget);
	}
}

void UWOGUIManagerSubsystem::RemoveSquadOrderWidget()
{
	if (SquadOrderWidget)
	{
		SquadOrderWidget->RemoveFromParent();
		RestoreAbilitiesWidget();
		UE_LOG(WOGLogUI, Display, TEXT("SquadOrderWidget removed from subsystem"));
		SquadOrderWidget = nullptr;
	}
	else
	{
		UE_LOG(WOGLogUI, Error, TEXT("SquadOrderWidget NOT removed from subsystem"));
	}
}
