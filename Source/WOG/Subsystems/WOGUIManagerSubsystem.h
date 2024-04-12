// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "WOGUIManagerSubsystem.generated.h"

class ABasePlayerCharacter;
struct FGameplayTag;
class UUserWidget;
class UWOGVendorBaseWidget;
class UWOGStashWidget;
class AWOGMatchHUD;
class AWOGPlayerController;
class UWOGHoldProgressBar;
class UWOGRavenMarkerWidget;
class UWOGAbilityWidget;
class AWOGVendor;
class AWOGStashBase;
class UWOGHuntProgressBar;

/**
 * 
 */
UCLASS()
class WOG_API UWOGUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void InitVariables();
	virtual void Deinitialize() override;
	
private:

	UPROPERTY()
	ETimeOfDay TOD;

	TObjectPtr<AWOGMatchHUD> MatchHUD;
	TObjectPtr<AWOGPlayerController> OwnerPC;
	
	TObjectPtr<UWOGVendorBaseWidget> VendorWidget;
	TObjectPtr<UWOGStashWidget> StashWidget;
	TObjectPtr<UUserWidget> BarsWidget;
	TObjectPtr<UUserWidget> TODWidget;
	TObjectPtr<UUserWidget> MinimapWidget;
	TObjectPtr<UUserWidget> AbilityContainerWidget;
	TObjectPtr<UUserWidget> AvailableResourcesWidget;
	TObjectPtr<UUserWidget> SquadOrderWidget;
	TObjectPtr<UUserWidget> CrosshairWidget;
	TObjectPtr<UWOGHuntProgressBar> HuntWidget;

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
	void RemoveStaminaWidget();

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

	UFUNCTION(BlueprintCallable)
	void ResetHUD();

	UFUNCTION(BlueprintCallable)
	void AddBarsWidget();
	UFUNCTION(BlueprintCallable)
	void RemoveBarsWidget();

	UFUNCTION(BlueprintCallable)
	void AddMinimapWidget();
	UFUNCTION(BlueprintCallable)
	void RemoveMinimapWidget();

	UFUNCTION(BlueprintCallable)
	void CollapseAbilitiesWidget();
	UFUNCTION(BlueprintCallable)
	void RestoreAbilitiesWidget();

	UFUNCTION(BlueprintCallable)
	void CollapseTODWidget();
	UFUNCTION(BlueprintCallable)
	void RestoreTODWidget();

	UFUNCTION(BlueprintCallable)
	void CollapseObjectiveWidget();
	UFUNCTION(BlueprintCallable)
	void RestoreObjectiveWidget();

	UFUNCTION(BlueprintCallable)
	void AddAvailableResourcesWidget();
	UFUNCTION(BlueprintCallable)
	void RemoveAvailableResourcesWidget();

	UFUNCTION(BlueprintCallable)
	void AddVendorWidget(ABasePlayerCharacter* Buyer, AWOGVendor* Vendor);
	UFUNCTION(BlueprintCallable)
	void RemoveVendorWidget();

	UFUNCTION(BlueprintCallable)
	void AddStashWidget(ABasePlayerCharacter* User, AWOGStashBase* Stash);
	UFUNCTION(BlueprintCallable)
	void RemoveStashWidget();

	UFUNCTION(BlueprintCallable)
	void AddSquadOrderWidget();
	UFUNCTION(BlueprintCallable)
	void RemoveSquadOrderWidget();

	UFUNCTION()
	void AddCrosshairWidget();
	UFUNCTION()
	void RemoveCrosshairWidget();

	UFUNCTION()
	void AddHuntWidget(AWOGHuntEnemy* HuntEnemy);
	UFUNCTION()
	void RemoveHuntWidget();
};
