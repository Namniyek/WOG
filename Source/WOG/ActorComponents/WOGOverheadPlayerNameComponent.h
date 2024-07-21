// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "WOGOverheadPlayerNameComponent.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGOverheadPlayerNameComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void InitOverheadWidget();
	
};
