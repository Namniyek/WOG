// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "WOGHealthBarWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGHealthBarWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetHealthBarPercent(float HealthBarPercent);
};
