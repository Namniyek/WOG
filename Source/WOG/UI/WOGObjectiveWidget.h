// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGObjectiveWidget.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EObjectiveText : uint8
{
	EOT_DefendTheVillage UMETA(DisplayName = "Defend the Village"),
	EOT_DestroyTheVillage UMETA(DisplayName = "Destroy the Village"),
	EOT_PrepareForTheNight UMETA(DisplayName = "Rest and prepare for the night"),

	EOT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class WOG_API UWOGObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	void SetObjectiveText(const EObjectiveText& ObjectiveText);
	
};
