// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/WOGUIManagerSubsystem.h"
#include "WOG.h"
#include "UI/Vendors/WOGVendorBaseWidget.h"
#include "Interfaces/VendorInterface.h"

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
