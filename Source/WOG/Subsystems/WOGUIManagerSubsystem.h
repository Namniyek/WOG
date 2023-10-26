// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "WOGUIManagerSubsystem.generated.h"

class UWOGVendorBaseWidget;
class AWOGMatchHUD;
class AWOGPlayerController;
class UWOGHoldProgressBar;
class UWOGRavenMarkerWidget;
class UWOGAbilityWidget;

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

	UPROPERTY()
	ETimeOfDay TOD;

	TObjectPtr<AWOGMatchHUD> MatchHUD;
	TObjectPtr<AWOGPlayerController> OwnerPC;
	
	TObjectPtr<UWOGVendorBaseWidget> VendorWidget;

	UPROPERTY()
	TObjectPtr<UWOGHoldProgressBar> HoldProgressBarWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWOGRavenMarkerWidget> RavenMarkerWidget = nullptr;

	void SetTODString(ETimeOfDay CurrentTOD, FString& StringMain, FString& StringSec);

public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetVendorWidget(UWOGVendorBaseWidget* NewWidget) { VendorWidget = NewWidget; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UWOGVendorBaseWidget* GetVendorWidget() const { return VendorWidget; }
	FORCEINLINE TObjectPtr<UWOGHoldProgressBar> GetHoldProgressBar() const { return HoldProgressBarWidget; }
	FORCEINLINE TObjectPtr<UWOGRavenMarkerWidget> GetRavenMarkerWidget() const { return RavenMarkerWidget; }

	UFUNCTION(BlueprintCallable)
	void UpdateAvailableResourceWidget();

	UFUNCTION(BlueprintCallable)
	void UpdateVendorWidgetAfterTransaction();

	UFUNCTION(BlueprintCallable)
	void CreateResourceWarningWidget(const FString& Attribute);

	UFUNCTION(BlueprintCallable)
	void CreateGenericWarningWidget(const FString& WarningString);

	UFUNCTION()
	void AddStaminaWidget();

	UFUNCTION()
	void AddScreenDamageWidget(const int32& DamageThreshold);

	UFUNCTION()
	void AddHoldProgressBar();
	UFUNCTION()
	void RemoveHoldProgressBar();

	UFUNCTION()
	void AddRavenMarkerWidget(const int32& Amount);
	UFUNCTION()
	void RemoveRavenMarkerWidget();

	UFUNCTION()
	void AddAbilityWidget(const int32& AbilityID, TSubclassOf<UUserWidget> Class, UTexture2D* Icon, const float& Cooldown, const FGameplayTag& Tag);
	UFUNCTION()
	void RemoveAbilityWidget(const int32& AbilityID);

	UFUNCTION()
	void AddAnnouncementWidget(ETimeOfDay NewTOD);

	UFUNCTION()
	void AddEndgameWidget();

	UFUNCTION()
	void ResetHUD();
};
