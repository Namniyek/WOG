// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOG_HUD.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOG_HUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	class ABasePlayerCharacter* OwnerPlayerCharacter;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	class AWOGPlayerController* OwnerPlayerController;

protected:
	virtual void NativeOnInitialized() override;
	UFUNCTION(BlueprintCallable)
	void ResetHUDAfterRespawn();

private:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

public:
	UFUNCTION(BlueprintPure)
	float GetHealthPercent();
	UFUNCTION(BlueprintPure)
	float GetManaPercent();
	UFUNCTION(BlueprintPure)
	float GetAdrenalinePercent();

	UFUNCTION(BlueprintPure)
	float GetStaminaPercent();

};
