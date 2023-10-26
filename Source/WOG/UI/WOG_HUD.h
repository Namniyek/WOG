// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOG_HUD.generated.h"

/**
 * 
 */
class UWOGAbilityContainerWidget;
class UVerticalBox;
class UWOGObjectiveWidget;
class USizeBox;

UCLASS()
class WOG_API UWOG_HUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	class ABasePlayerCharacter* OwnerPlayerCharacter;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	class AWOGPlayerController* OwnerPlayerController;

	virtual void NativeConstruct() override;

protected:
	virtual void NativeOnInitialized() override;
	UFUNCTION(BlueprintCallable)
	void ResetHUDAfterRespawn();

private:

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* WarningContainer;

	UPROPERTY(meta = (BindWidget))
	UWOGObjectiveWidget* ObjectiveWidget;

	UPROPERTY(meta = (BindWidget))
	USizeBox* HoldBarContainer;

	UPROPERTY(meta = (BindWidget))
	USizeBox* TODContainer;

	UPROPERTY(meta = (BindWidget))
	USizeBox* MinimapContainer;

	UPROPERTY(meta = (BindWidget))
	USizeBox* AvailableResourceContainer;

	UPROPERTY(meta = (BindWidget))
	USizeBox* AbilitiesContainer;

	UPROPERTY(meta = (BindWidget))
	USizeBox* BarsContainer;

public:
	UFUNCTION(BlueprintPure)
	float GetHealthPercent();
	UFUNCTION(BlueprintPure)
	float GetManaPercent();
	UFUNCTION(BlueprintPure)
	float GetAdrenalinePercent();

	UFUNCTION(BlueprintPure)
	float GetStaminaPercent();

	//Can return nullptr
	UFUNCTION()
	UWOGAbilityContainerWidget* GetAbilityContainer();

	FORCEINLINE UVerticalBox* GetWarningContainer() const { return WarningContainer; }
	FORCEINLINE UWOGObjectiveWidget* GetObjectiveWidget() const { return ObjectiveWidget; }
	FORCEINLINE USizeBox* GetHoldBarContainer() const { return HoldBarContainer; }
	FORCEINLINE USizeBox* GetBarsContainer() const { return BarsContainer; }
	FORCEINLINE USizeBox* GetAbilitiesContainer() const { return AbilitiesContainer; }
	FORCEINLINE USizeBox* GetAvailableResourceContainer() const { return AvailableResourceContainer; }
	FORCEINLINE USizeBox* GetMinimapContainer() const { return MinimapContainer; }
	FORCEINLINE USizeBox* GetTODContainer() const { return TODContainer; }

};
