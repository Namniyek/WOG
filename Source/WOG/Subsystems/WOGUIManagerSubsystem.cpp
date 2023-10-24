// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/WOGUIManagerSubsystem.h"
#include "WOG.h"
#include "UI/Vendors/WOGVendorBaseWidget.h"
#include "Interfaces/VendorInterface.h"
#include "PlayerController/WOGPlayerController.h"
#include "UI/WOGMatchHUD.h"
#include "UI/WOG_HUD.h"
#include "Engine/LocalPlayer.h"
#include "UI/WOGWarningWidget.h"
#include "Components/VerticalBox.h"

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

void UWOGUIManagerSubsystem::CreateWarningWidget(const FString& Attribute)
{
	MatchHUD == nullptr ? Cast<AWOGMatchHUD>(OwnerPC->GetHUD()) : MatchHUD;
	if (!MatchHUD || !MatchHUD->HUDWidget || !IsValid(MatchHUD->AttributeWarningClass)) return;

	TObjectPtr<UWOGWarningWidget> WarningWidget = Cast<UWOGWarningWidget>(CreateWidget<UUserWidget>(OwnerPC, MatchHUD->AttributeWarningClass));
	if (WarningWidget)
	{
		WarningWidget->SetWarningText(Attribute);

		if (MatchHUD->HUDWidget->GetWarningBox())
		{
			MatchHUD->HUDWidget->GetWarningBox()->ClearChildren();
			MatchHUD->HUDWidget->GetWarningBox()->AddChild(WarningWidget);
		}
	}
}
