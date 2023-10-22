// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGCharacterWidgetContainer.generated.h"

/**
 * 
 */
class USizeBox;

UCLASS()
class WOG_API UWOGCharacterWidgetContainer : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> Container;
	
public:
	FORCEINLINE USizeBox* GetContainer() const { return Container; }
	
};
