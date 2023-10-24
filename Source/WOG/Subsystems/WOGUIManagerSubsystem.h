// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "WOGUIManagerSubsystem.generated.h"

class UWOGVendorBaseWidget;
class AWOGMatchHUD;
class AWOGPlayerController;

/**
 * 
 */
UCLASS()
class WOG_API UWOGUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UFUNCTION(BlueprintCallable)
	void InitVariables();
	virtual void Deinitialize() override;
	

private:
	TObjectPtr<AWOGMatchHUD> MatchHUD;
	TObjectPtr<AWOGPlayerController> OwnerPC;
	
	TObjectPtr<UWOGVendorBaseWidget> VendorWidget;




public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetVendorWidget(UWOGVendorBaseWidget* NewWidget) { VendorWidget = NewWidget; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UWOGVendorBaseWidget* GetVendorWidget() const { return VendorWidget; }

	UFUNCTION(BlueprintCallable)
	void UpdateAvailableResourceWidget();

	UFUNCTION(BlueprintCallable)
	void UpdateVendorWidgetAfterTransaction();

	UFUNCTION(BlueprintCallable)
	void CreateWarningWidget(const FString& Attribute);
};
