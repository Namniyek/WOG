// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/SpawnInterface.h"
#include "WOGRavenMarkerWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGRavenMarkerWidget : public UUserWidget, public ISpawnInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	int32 AmountAvailableMarkers;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetAmountAvailableMarkers(const int32& Amount);
	
};
