// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGWarningWidget.generated.h"

/**
 * 
 */
class UTextBlock;

UCLASS()
class WOG_API UWOGWarningWidget : public UUserWidget
{
	GENERATED_BODY()


private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarningText;


public:

	void SetWarningText(const FString& Attribute);

	
};
