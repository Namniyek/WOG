// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGAbilityContainerWidget.generated.h"

/**
 * 
 */
class UOverlay;
class UWOGAbilityWidget;

UCLASS()
class WOG_API UWOGAbilityContainerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> AbilityContainer_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> AbilityContainer_2;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> AbilityContainer_3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> AbilityContainer_4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> OverlayInputButton1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> OverlayInputButton2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> OverlayInputButton3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> OverlayInputButton4;

	void AddChildAbility(const int32& AbilityID, UWOGAbilityWidget* WidgetToAdd) const;
	void RemoveChildAbility(const int32& AbilityID) const;
};
