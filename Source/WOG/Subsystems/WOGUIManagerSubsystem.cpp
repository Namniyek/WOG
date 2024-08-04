// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/WOGUIManagerSubsystem.h"

#include "WOG.h"
#include "ActorComponents/WOGDamageTextComponent.h"
#include "ActorComponents/WOGHealthBarWidgetComponent.h"
#include "Data/TODEnum.h"
#include "UI/Vendors/WOGVendorBaseWidget.h"
#include "Interfaces/InventoryInterface.h"
#include "PlayerController/WOGPlayerController.h"
#include "UI/WOGMatchHUD.h"
#include "UI/WOG_HUD.h"
#include "Engine/LocalPlayer.h"
#include "UI/WOGWarningWidget.h"
#include "Components/VerticalBox.h"
#include "UI/WOGCharacterWidgetContainer.h"
#include "Components/SizeBox.h"
#include "UI/WOGRoundProgressBar.h"
#include "UI/WOGScreenDamage.h"
#include "UI/WOGHoldProgressBar.h"
#include "UI/WOGRavenMarkerWidget.h"
#include "UI/WOGAbilityWidget.h"
#include "UI/WOGAbilityContainerWidget.h"
#include "UI/WOGObjectiveWidget.h"
#include "UI/WOGAvailableResourceWidget.h"
#include "PlayerCharacter/BasePlayerCharacter.h"
#include "UI/WOGCountdownWidget.h"
#include "UI/Stash/WOGStashWidget.h"
#include "UI/WOGHuntProgressBar.h"

void UWOGUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UWOGUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UWOGUIManagerSubsystem::InitVariables()
{
	VendorWidget = nullptr;
	StashWidget = nullptr;
	BarsWidget = nullptr;
	TODWidget = nullptr;
	MinimapWidget = nullptr;
	AbilityContainerWidget = nullptr;
	AvailableResourcesWidget = nullptr;
	SquadOrderWidget = nullptr;
	CrosshairWidget = nullptr;
	HuntWidget = nullptr;
	HoldProgressBarWidget = nullptr;
	RavenMarkerWidget = nullptr;

	
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

void UWOGUIManagerSubsystem::SetTODString(const ETimeOfDay CurrentTOD, FString& StringMain, FString& StringSec) const
{
	MatchHUD == nullptr ? static_cast<TObjectPtr<AWOGMatchHUD>>(Cast<AWOGMatchHUD>(OwnerPC->GetHUD())) : MatchHUD;
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

void UWOGUIManagerSubsystem::AddStaminaWidget() const
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

void UWOGUIManagerSubsystem::RemoveStaminaWidget()
{
	TObjectPtr<ABasePlayerCharacter> BaseCharacter = Cast<ABasePlayerCharacter>(OwnerPC->GetPawn());
	if (!BaseCharacter || !BaseCharacter->GetStaminaWidgetContainer() || !BaseCharacter->GetStaminaWidgetContainer()->GetContainer()) return;
	if (!BaseCharacter->GetStaminaWidgetContainer()->GetContainer()->HasAnyChildren()) return;

	UWidget* StaminaBar = BaseCharacter->GetStaminaWidgetContainer()->GetContainer()->GetChildAt(0);
	if (StaminaBar)
	{
		StaminaBar->RemoveFromParent();
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
	MatchHUD == nullptr ? static_cast<TObjectPtr<AWOGMatchHUD>>(Cast<AWOGMatchHUD>(OwnerPC->GetHUD())) : MatchHUD;
	if (!MatchHUD || !MatchHUD->AnnouncementClass) return;

	TOD = NewTOD;
	FString StringMain = FString();
	FString StringSec = FString();
	SetTODString(TOD, StringMain, StringSec);

	MatchHUD->AddAnnouncementWidget(StringMain, StringSec);
}

void UWOGUIManagerSubsystem::AddGenericAnnouncementWidget(const FText& MainText, const FText& SecText)
{
	MatchHUD == nullptr ? static_cast<TObjectPtr<AWOGMatchHUD>>(Cast<AWOGMatchHUD>(OwnerPC->GetHUD())) : MatchHUD;
	if (!MatchHUD || !MatchHUD->AnnouncementClass) return;

	MatchHUD->AddAnnouncementWidget(MainText.ToString(), SecText.ToString());
}

void UWOGUIManagerSubsystem::AddEndgameWidget() const
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD) return;

	MatchHUD->AddEndgameWidget();
}

void UWOGUIManagerSubsystem::ResetHUD()
{
	if(!OwnerPC || !OwnerPC->GetHUD()) return;
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

void UWOGUIManagerSubsystem::ClearHUD()
{
	if(!OwnerPC || !OwnerPC->GetHUD()) return;
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	
	RemoveStaminaWidget();
	RemoveBarsWidget();
	RemoveMinimapWidget();
	CollapseAbilitiesWidget();
	CollapseObjectiveWidget();
	RemoveAvailableResourcesWidget();
	CollapseTODWidget();
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
		MatchHUD->HUDWidget->GetMinimapContainer()->ClearChildren();
		MatchHUD->HUDWidget->GetMinimapContainer()->AddChild(MinimapWidget);
	}
}

void UWOGUIManagerSubsystem::RemoveMinimapWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !MatchHUD->HUDWidget->GetMinimapContainer()) return;

	MatchHUD->HUDWidget->GetMinimapContainer()->ClearChildren();
	MinimapWidget = nullptr;
}

void UWOGUIManagerSubsystem::CollapseAbilitiesWidget()
{
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !MatchHUD->HUDWidget->GetAbilitiesContainer()) return;

	AbilityContainerWidget = AbilityContainerWidget == nullptr ? (TObjectPtr<UUserWidget>) Cast<UUserWidget>(MatchHUD->HUDWidget->GetAbilitiesContainer()->GetChildAt(0)) : AbilityContainerWidget;
	if (AbilityContainerWidget != nullptr && AbilityContainerWidget->GetVisibility() == ESlateVisibility::Visible)
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
	MatchHUD == nullptr ? static_cast<TObjectPtr<AWOGMatchHUD>>(Cast<AWOGMatchHUD>(OwnerPC->GetHUD())) : MatchHUD;
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

void UWOGUIManagerSubsystem::AddCrosshairWidget()
{
	if (!OwnerPC) return;
	MatchHUD == nullptr ? (TObjectPtr<AWOGMatchHUD>) Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->CrosshairWidgetClass)) return;
	if (CrosshairWidget) return;

	CrosshairWidget = CreateWidget<UUserWidget>(OwnerPC, MatchHUD->CrosshairWidgetClass);
	if (CrosshairWidget)
	{
		CrosshairWidget->AddToViewport();
	}
}

void UWOGUIManagerSubsystem::RemoveCrosshairWidget()
{
	if (CrosshairWidget)
	{
		CrosshairWidget->RemoveFromParent();
	}

	CrosshairWidget = nullptr;
}

void UWOGUIManagerSubsystem::AddHuntWidget(AWOGHuntEnemy* HuntEnemy)
{
	if (!OwnerPC) return;
	MatchHUD == nullptr ? static_cast<TObjectPtr<AWOGMatchHUD>>(Cast<AWOGMatchHUD>(OwnerPC->GetHUD())) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->HuntWidgetClass)) return;
	if (HuntWidget) return;

	HuntWidget = CreateWidget<UWOGHuntProgressBar>(OwnerPC, MatchHUD->HuntWidgetClass);
	if (HuntWidget && MatchHUD->HUDWidget->GetHuntProgressBarContainer())
	{
		HuntWidget->HuntCharacter = HuntEnemy;
		MatchHUD->HUDWidget->GetHuntProgressBarContainer()->ClearChildren();
		MatchHUD->HUDWidget->GetHuntProgressBarContainer()->AddChild(HuntWidget);
	}
}

void UWOGUIManagerSubsystem::RemoveHuntWidget()
{
	if (HuntWidget)
	{
		HuntWidget->RemoveFromParent();
	}

	HuntWidget = nullptr;
}

void UWOGUIManagerSubsystem::AddCountdownWidget(const FText& MainText, const FText& SecText, float CountdownDuration)
{
	if (!OwnerPC) return;
	MatchHUD == nullptr ? static_cast<TObjectPtr<AWOGMatchHUD>>(Cast<AWOGMatchHUD>(OwnerPC->GetHUD())) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->CountdownWidgetClass)) return;

	UWOGCountdownWidget* CountdownWidget = CreateWidget<UWOGCountdownWidget>(OwnerPC, MatchHUD->CountdownWidgetClass);
	if (CountdownWidget && MatchHUD->HUDWidget->GetCountdownContainer())
	{
		CountdownWidget->MainText = MainText;
		CountdownWidget->SecText = SecText;
		CountdownWidget->CountdownTime = CountdownDuration;
		
		MatchHUD->HUDWidget->GetCountdownContainer()->ClearChildren();
		MatchHUD->HUDWidget->GetCountdownContainer()->AddChild(CountdownWidget);
	}
}

void UWOGUIManagerSubsystem::AddFloatingDamageTextWidget(float DamageAmount, AActor* TargetActor)
{
	if (!OwnerPC) return;
	MatchHUD == nullptr ? static_cast<TObjectPtr<AWOGMatchHUD>>(Cast<AWOGMatchHUD>(OwnerPC->GetHUD())) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->DamageTextComponentClass)) return;
	if(!IsValid(TargetActor)) return;

	UWOGDamageTextComponent* DamageTextComponent = NewObject<UWOGDamageTextComponent>(TargetActor, MatchHUD->DamageTextComponentClass);
	DamageTextComponent->RegisterComponent();
	TArray<UActorComponent*> Array = TargetActor->GetComponentsByTag(USceneComponent::StaticClass(), WOG_Overhead_Widget_Location);
	if(Array.IsEmpty()) return;

	USceneComponent* WidgetLocation = CastChecked<USceneComponent>(Array[0]);
	DamageTextComponent->AttachToComponent(WidgetLocation, FAttachmentTransformRules::KeepRelativeTransform);
	DamageTextComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	DamageTextComponent->SetDamageText(DamageAmount);
	
}

void UWOGUIManagerSubsystem::AddCharacterHealthBarWidget(float NewValue, float MaxValue, AActor* TargetActor)
{
	
	if (!OwnerPC) return;
	MatchHUD == nullptr ? static_cast<TObjectPtr<AWOGMatchHUD>>(Cast<AWOGMatchHUD>(OwnerPC->GetHUD())) : MatchHUD;
	if (!MatchHUD || !IsValid(MatchHUD->CharacterHealthBarComponentClass)) return;
	if(!IsValid(TargetActor)) return;

	float HealthBarPercent = NewValue/MaxValue;

	if(IsValid(CharacterHealthBarWidgetComponent))
	{
		CharacterHealthBarWidgetComponent->SetHealthBarPercent(HealthBarPercent);
		GetWorld()->GetTimerManager().ClearTimer(CharacterHealthBarTimer);
		GetWorld()->GetTimerManager().SetTimer(CharacterHealthBarTimer, this, &ThisClass::RemoveCharacterHealthBar, 3.f);
		return;
	}
	
	CharacterHealthBarWidgetComponent = NewObject<UWOGHealthBarWidgetComponent>(TargetActor, MatchHUD->CharacterHealthBarComponentClass);
	if(IsValid(CharacterHealthBarWidgetComponent))
	{
		CharacterHealthBarWidgetComponent->RegisterComponent();
		TArray<UActorComponent*> Array = TargetActor->GetComponentsByTag(USceneComponent::StaticClass(), WOG_Overhead_Widget_Location);
		if(Array.IsEmpty()) return;

		USceneComponent* WidgetLocation = CastChecked<USceneComponent>(Array[0]);
		CharacterHealthBarWidgetComponent->AttachToComponent(WidgetLocation, FAttachmentTransformRules::KeepRelativeTransform);
		CharacterHealthBarWidgetComponent->SetHealthBarPercent(HealthBarPercent);
		GetWorld()->GetTimerManager().ClearTimer(CharacterHealthBarTimer);
		GetWorld()->GetTimerManager().SetTimer(CharacterHealthBarTimer, this, &ThisClass::RemoveCharacterHealthBar, 3.f);
	}
}

void UWOGUIManagerSubsystem::RemoveCharacterHealthBar()
{
	if(IsValid(CharacterHealthBarWidgetComponent))
	{
		CharacterHealthBarWidgetComponent->DestroyComponent();
		CharacterHealthBarWidgetComponent = nullptr;
	}
}

void UWOGUIManagerSubsystem::SetLocalOutlineEnabled(bool bEnableOutline, int32 NewStencilIndex)
{
	OwnerPC = Cast<AWOGPlayerController>(GetLocalPlayer()->GetPlayerController(GetLocalPlayer()->GetWorld()));
	if(!OwnerPC) return;

	int32 NewStencilValue = bEnableOutline ? NewStencilIndex : 0;
	
	const AWOGBaseCharacter* PlayerCharacter = Cast<AWOGBaseCharacter>(OwnerPC->GetPawn());
	if(!PlayerCharacter) return;

	PlayerCharacter->GetMesh()->SetCustomDepthStencilValue(NewStencilValue);
}

