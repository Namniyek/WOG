// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "WOGDamageTextComponent.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGDamageTextComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(const float& DamageText);	
	
};
