// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGHoldProgressBar.generated.h"

/**
 * 
 */
class UProgressBar;

UCLASS()
class WOG_API UWOGHoldProgressBar : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HoldProgressBar;

	float MaxHoldTime = 0.75f;

	UFUNCTION(BlueprintPure)
	float GetHoldProgress();

public:
	UPROPERTY(BlueprintReadOnly);
	float TimeHeld;

	
};
