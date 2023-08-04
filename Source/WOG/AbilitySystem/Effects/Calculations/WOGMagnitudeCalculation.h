// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "WOGMagnitudeCalculation.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGMagnitudeCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Calculation")
	float CalculateMagicCost(const FGameplayEffectSpec& Spec) const;

	UFUNCTION(BlueprintCallable, Category = "Calculation")
	float CalculateMagicCooldown(const FGameplayEffectSpec& Spec) const;

};
