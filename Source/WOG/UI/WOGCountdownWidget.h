// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGCountdownWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOG_API UWOGCountdownWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CountdownTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText MainText = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SecText = FText();
};
