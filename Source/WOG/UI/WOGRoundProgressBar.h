// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WOGRoundProgressBar.generated.h"

/**
 * 
 */
class UImage;
class UMaterialInstance;
class UWOGAttributeSetBase;

UCLASS()
class WOG_API UWOGRoundProgressBar : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
	virtual void NativeOnInitialized() override;

protected:

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ProgressBar;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInstance> ProgressBarMaterial;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor StaminaColor;
	UPROPERTY(EditDefaultsOnly)
	FLinearColor LowStaminaColor;

	TObjectPtr<UMaterialInstanceDynamic> DynamicProgressBarMaterial;

	void SetPercent();
	void SetBarColor(const double& Percent);

private:
	const UWOGAttributeSetBase* AttributeSet = nullptr;
};
