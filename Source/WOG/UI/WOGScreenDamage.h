// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGScreenDamage.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGScreenDamage : public UUserWidget
{
	GENERATED_BODY()

private:

	float RadiusMin;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetRadius(const float& NewRadius) { RadiusMin = NewRadius; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetRadius() const { return RadiusMin; }

	UFUNCTION(BlueprintCallable)
	void SetRadiusValue(const int32& DamageThreshold);

};
